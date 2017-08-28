/*
    
    Copyright (C) 2017 Czech Technical University in Prague
    The MIT License (MIT)
    
    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files
    (the "Software"), to deal in the Software without restriction,
    including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software,
    and to permit persons to whom the Software is furnished to do so,
    subject to the following conditions:
    
    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "DatasetGenerator2016B_2.h"
#include "../../rseclp/utils/JsonUtils.h"
#include "../../rseclp/utils/GeneralUtils.h"
#include "../../rseclp/instance/Instance.h"
#include "../../rseclp/instance/InstanceJsonWriter.h"
#include "../../rseclp/solvers/exact/LazyConstraints.h"
#include "../../rseclp/solvers/heuristics/GreedyHeuristics.h"
#include "../../rseclp/objectives/TotalTardiness.h"

namespace rseclp {

    DatasetGenerator2016B_2::Prescription::Prescription(int numInstances,
                                                  vector<int> numOperationsVector,
                                                  int numMeteringIntervalsMul,
                                                  vector<double> alpha1Vector,
                                                  vector<double> alpha2Vector,
                                                  vector<double> alpha3Vector,
                                                  vector<int> maxDeviationVector)
            : mNumInstances(numInstances),
              mNumOperationsVector(move(numOperationsVector)),
              mNumMeteringIntervalsMul(numMeteringIntervalsMul),
              mAlpha1Vector(move(alpha1Vector)),
              mAlpha2Vector(move(alpha2Vector)),
              mAlpha3Vector(move(alpha3Vector)),
              mMaxDeviationVector(move(maxDeviationVector)) {}

    DatasetGenerator2016B_2::Prescription DatasetGenerator2016B_2::Prescription::load(const Document &doc) {
        return Prescription(doc["numInstances"].GetInt(),
                            move(JsonUtils::getVector<int>(doc, "numOperations")),
                            doc["numMeteringIntervalsMul"].GetInt(),
                            move(JsonUtils::getVector<double>(doc, "alpha1")),
                            move(JsonUtils::getVector<double>(doc, "alpha2")),
                            move(JsonUtils::getVector<double>(doc, "alpha3")),
                            move(JsonUtils::getVector<int>(doc, "maxDeviation")));
    }

    Instance *DatasetGenerator2016B_2::Prescription::sample(default_random_engine &randomEngine, int numOperations) const {
        int lengthMeteringInterval = 15;
        double maxEnergyConsumption = 100;

        double alpha1 = GeneralUtils::randomElement(mAlpha1Vector, randomEngine);
        double alpha2 = GeneralUtils::randomElement(mAlpha2Vector, randomEngine);
        double alpha3 = GeneralUtils::randomElement(mAlpha3Vector, randomEngine);
        int maxDeviation = GeneralUtils::randomElement(mMaxDeviationVector, randomEngine);

        // TODO (refactor): same as in DatasetGenerator2016B_1
        int numMeteringIntervals = mNumMeteringIntervalsMul * numOperations;
        vector<double> maxEnergyConsumptions(numMeteringIntervals, maxEnergyConsumption);

        // Processing times.
        uniform_int_distribution<int> processingTimesDist(1, lengthMeteringInterval);
        vector<int> processingTimes;
        for (int j = 0; j < numOperations; j++) {
            int processingTime = processingTimesDist(randomEngine);
            processingTimes.push_back(processingTime);
        }
        int sumProcessingTimes = GeneralUtils::sum(processingTimes);
        double averageProcessingTime = GeneralUtils::average(processingTimes);

        // Release times.
        // Based on sampling exponential interarrival time.
        // The mean interarrival time is alpha1 * averageProcessingTime
        exponential_distribution<double> interarrivalTimeDist(1.0 / (alpha1 * averageProcessingTime));
        vector<int> releaseTimes;
        {
            int currentTime = 0;
            for (int j = 0; j < numOperations; j++) {
                currentTime += (int) interarrivalTimeDist(randomEngine);
                int releaseTime = currentTime;
                releaseTimes.push_back(releaseTime);
            }
        }

        // Due dates.
        // Generator taken from "A Branch-and-Bound Procedure to Minimize Total Tardiness on One Machine with Arbitrary Release Dates"
        vector<int> dueDates;
        uniform_int_distribution<int> dueDateDiffDist(0, (int)ceil(alpha2 * sumProcessingTimes));
        for (int j = 0; j < numOperations; j++) {
            int dueDateDiff = dueDateDiffDist(randomEngine);
            dueDates.push_back(releaseTimes[j] + processingTimes[j] + dueDateDiff);
        }

        // Power consumptions.
        // Inspired by example instance in "An hybrid CP/MILP method for scheduling with energy costs"
        vector<double> powerConsumptions;
        uniform_real_distribution<double> energyConsumptionsDist(alpha3 * maxEnergyConsumption, maxEnergyConsumption);
        for (int j = 0; j < numOperations; j++) {
            double energyConsumption = energyConsumptionsDist(randomEngine);
            powerConsumptions.push_back(energyConsumption / processingTimes[j]);
        }

        Instance *ins = Instance::create(numOperations,
                                         releaseTimes,
                                         dueDates,
                                         processingTimes,
                                         powerConsumptions,
                                         maxDeviation,
                                         numMeteringIntervals,
                                         lengthMeteringInterval,
                                         maxEnergyConsumptions,
                                         map<string, string>());

        // TODO (refactor): check whether there is some feasible solution.
        {
            Solver::SpecialisedConfig scfg;
            scfg.addValue(GreedyHeuristics::KEY_SOLVER,
                          GreedyHeuristics::Config::KEY_RULE,
                          GreedyHeuristics::Config::RULE_TARDINESS);

            auto obj = unique_ptr<TotalTardiness>(new TotalTardiness());
            Solver::Config cfg(chrono::seconds(3600), obj.get(), false, StartTimes(), scfg);

            GreedyHeuristics alg(*ins);
            if (alg.solve(cfg).getStatus() != Solver::Result::Status::FEASIBLE) {
                cout << "error: infeasible instance" << endl;
                exit(1);
            }
        }

        return ins;
    }

    void DatasetGenerator2016B_2::generate(const DatasetGenerator2016B_2::Prescription &p,
                                           const path &prescriptionOutputDir) const {
        default_random_engine randomEngine;

        for (int numOperations: p.mNumOperationsVector) {
            cout << "Generating instances for n=" << numOperations << endl;
            path instancesOutputDir = prescriptionOutputDir / to_string(numOperations);
            create_directory(instancesOutputDir);

            for (int instanceIndex = 0; instanceIndex < p.mNumInstances; instanceIndex++) {
                cout << "Generating instance " << instanceIndex << endl;
                auto instancePath = instancesOutputDir / (to_string(instanceIndex) + ".json");
                auto ins = unique_ptr<Instance>(p.sample(randomEngine, numOperations));
                InstanceJsonWriter::write(*ins, instancePath.string());
            }
        }
    }

    void DatasetGenerator2016B_2::fromPrescription(Document &prescriptionDocument,
                                             const string &datasetName,
                                             const path &outputDir) {
        auto p = Prescription::load(prescriptionDocument);

        if (!exists(outputDir)) {
            create_directories(outputDir);
        }

        auto prescriptionOutputDir = outputDir / datasetName;
        if (exists(prescriptionOutputDir)) {
            remove_all(prescriptionOutputDir);
        }
        create_directory(prescriptionOutputDir);

        generate(p, prescriptionOutputDir);
    }
}
