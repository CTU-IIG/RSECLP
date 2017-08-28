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

#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <iostream>
#include <gurobi_c++.h>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <sched.h>
#include "ExperimentPrescription.h"
#include "../rseclp/instance/Instance.h"
#include "../rseclp/instance/InstanceJsonReader.h"
#include "../rseclp/solvers/MultiStageSolver.h"
#include "../rseclp/solvers/heuristics/GreedyHeuristics.h"
#include "../rseclp/solvers/exact/LazyConstraints.h"
#include "../rseclp/solvers/exact/BranchAndBoundOnOrder.h"
#include "../rseclp/objectives/TotalTardiness.h"
#include "../rseclp/feasibility-checkers/FeasibilityChecker.h"
#include "../rseclp/solvers/SolverResultJsonWriter.h"
#include "../rseclp/solvers/SolverResultJsonReader.h"
#include "../rseclp/solvers/heuristics/TabuSearch.h"

using namespace std;
using namespace boost::filesystem;
using namespace rseclp;

struct Work {
    const int mRealisationIndex;
    const int mRepetition;

    Work(int realisationIndex, int repetition) : mRealisationIndex(realisationIndex), mRepetition(repetition) { }
};

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "Error: experiment name not provided";
        exit(1);
    }

    path experimentDataDir = current_path() / "experiment-data";

    string datasetName = string(argv[1]);
    path experimentPrescriptionFilename = path(argv[2]);
    int numThreads = stoi(argv[3]);

    path experimentPrescriptionName = experimentPrescriptionFilename.stem();

    path experimentPrescriptionDir = experimentDataDir / "experiments" / datasetName;
    path experimentPrescriptionPath = experimentPrescriptionDir / experimentPrescriptionFilename;

    path resultsPath = experimentPrescriptionDir / "results";
    path datasetsDir = experimentDataDir / "datasets";
    path datasetDir = datasetsDir / datasetName;

    cout << "Starting experiment " << experimentPrescriptionFilename << " for dataset " << datasetName << "." << endl;

    auto experimentPrescription = ExperimentPrescription::readFromJson(experimentPrescriptionPath);

    if (!exists(resultsPath)) {
        create_directories(resultsPath);
    }

    // Generate work.
    vector<Work*> works;
    for (directory_iterator realisationPathIter(datasetDir); realisationPathIter != directory_iterator(); realisationPathIter++) {
        path realisationPath = realisationPathIter->path();
        path instancesDir = realisationPath / "instances";
        int realisationIndex = stoi(realisationPath.stem().string());

        for (directory_iterator instancesPathIter(instancesDir); instancesPathIter != directory_iterator(); instancesPathIter++) {
            path instancePath = instancesPathIter->path();
            int repetition = stoi(instancePath.stem().string());
            works.push_back(new Work(realisationIndex, repetition));
        }
    }

    auto &solverStage = experimentPrescription.getSolverStage();

    // One global mutex for everything.
    // Since all shared objects are accessed outside of time-consuming solver.solve(), it is not such a problem.
    mutex mtx;
    auto threadCallback = [&]() {
        unique_ptr<TotalTardiness> obj(new TotalTardiness());
        while (true) {
            // This is needed due to license check.
            mtx.lock();
            GRBEnv env;
            mtx.unlock();

            unique_ptr<Work> work = nullptr;
            unique_ptr<Instance> ins = nullptr;
            unique_ptr<Solver> solver = nullptr;
            unique_ptr<Solver::Config> cfg = nullptr;

            {
                // Acquire lock on mtx.
                lock_guard<mutex> lock(mtx);

                if (works.size() == 0) {
                    return;
                }

                work.reset(works.back());
                works.pop_back();

                path instancePath = datasetDir / to_string(work->mRealisationIndex) / "instances" / (to_string(work->mRepetition) + ".json");
                ins.reset(InstanceJsonReader::read(instancePath));

                experimentPrescription.setGurobiEnvParams(env);

                if (solverStage.getName() == GreedyHeuristics::KEY_SOLVER) {
                    solver.reset(new GreedyHeuristics(*ins));
                } else if (solverStage.getName() == TabuSearch::KEY_SOLVER) {
                    solver.reset(new TabuSearch(*ins));
                } else if (solverStage.getName() == LazyConstraints::KEY_SOLVER) {
                    solver.reset(new LazyConstraints(*ins, env));
                } else if (solverStage.getName() == BranchAndBoundOnOrder::KEY_SOLVER) {
                    solver.reset(new BranchAndBoundOnOrder(*ins)); } else {
                    cout << "Unkown solver " << solverStage.getName() << endl;
                    exit(1);
                };

                Solver::SpecialisedConfig specialisedConfig;
                solverStage.setSpecialisedConfig(specialisedConfig);

                bool useInitStartTimes = false;
                StartTimes initStartTimes;
                if (experimentPrescription.getPreviousStage() != "") {
                    path previousStageFilename = path(experimentPrescription.getPreviousStage());
                    path previousStageName = previousStageFilename.stem();
                    auto previousStageResult = SolverResultJsonReader::read(resultsPath / previousStageName / to_string(work->mRealisationIndex) / (to_string(work->mRepetition) + ".json"));

                    if (previousStageResult.getStatus() == Solver::Result::Status::OPTIMAL ||
                            previousStageResult.getStatus() == Solver::Result::Status::FEASIBLE) {
                        useInitStartTimes = true;
                        initStartTimes = previousStageResult.getStartTimes();
                    }
                }

                cfg.reset(new Solver::Config(experimentPrescription.getTimeLimit(),
                                             obj.get(),
                                             useInitStartTimes,
                                             initStartTimes,
                                             move(specialisedConfig)));

                cout << "Starting instance " << instancePath << endl;
            }

            auto result = solver->solve(*cfg);

            {
                // Acquire lock on mtx.
                lock_guard<mutex> lock(mtx);

                if (result.getStatus() == Solver::Result::Status::OPTIMAL ||
                        result.getStatus() == Solver::Result::Status::FEASIBLE) {
                    if (!FeasibilityChecker(*ins).areFeasible(result.getStartTimes())) {
                        cout << "error" << endl;
                        exit(1);
                    }
                }

                cout << " Solver finished with status \"" << result.getStatus() << "\"." << endl;
                cout << endl;

                path resultPath = resultsPath / experimentPrescriptionName / to_string(work->mRealisationIndex) / (to_string(work->mRepetition) + ".json");

                // Write result.
                if (!exists(resultPath.parent_path())) {
                    create_directories(resultPath.parent_path());
                }

                SolverResultJsonWriter::write(result, resultPath);
            }
        }
    };

    vector<thread> workers;
    {
        // Acquire lock on mtx.
        lock_guard<mutex> lock(mtx);

        for (int workerId = 0; workerId < numThreads; workerId++) {
            workers.emplace_back(threadCallback);
        }

        // TODO: requires root - problem with gurobi.
//        struct sched_param schedParam;
//        schedParam.sched_priority = sched_get_priority_max(SCHED_FIFO);
//        for (thread &worker : workers) {
//           auto error = pthread_setschedparam(worker.native_handle(), SCHED_FIFO, &schedParam);
//           cout << error << endl;
//        }
    }

    for (auto &worker : workers) {
        worker.join();
    }

    cout << "Done." << endl;

    return 0;
}
