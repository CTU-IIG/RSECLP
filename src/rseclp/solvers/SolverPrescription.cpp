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

#include "SolverPrescription.h"
#include "../utils/JsonUtils.h"
#include "../objectives/TotalTardiness.h"
#include "heuristics/GreedyHeuristics.h"
#include "heuristics/TabuSearch.h"
#include "exact/LazyConstraints.h"
#include "exact/BranchAndBoundOnOrder.h"

namespace rseclp {
    SolverPrescription::SolverPrescription(Solver::Config *config,
                                           const map<string, string> &gurobiEnvParams,
                                           const string &solverStage,
                                           bool hasPreviousStage,
                                           const string &previousStage)
            : mConfig(config),
              mSolverStage(solverStage),
              mHasPreviousStage(hasPreviousStage),
              mPreviousStage(previousStage) {
        fillGurobiEnvParams(gurobiEnvParams);
    }

    SolverPrescription *SolverPrescription::read(const boost::filesystem::path &prescriptionPath) {
        Document doc;
        doc.SetObject();
        JsonUtils::readJsonDocument(prescriptionPath, doc, true);

        map<string, string> gurobiEnvParams;
        if (doc.FindMember("gurobiEnvParams") != doc.MemberEnd()) {
            for (auto it = doc["gurobiEnvParams"].MemberBegin(); it != doc["gurobiEnvParams"].MemberEnd(); it++) {
                gurobiEnvParams[string(it->name.GetString())] = move(string(it->value.GetString()));
            }
        }

        chrono::milliseconds timeLimit(chrono::milliseconds::max());
        if (doc.FindMember("timeLimitInMilliseconds") != doc.MemberEnd()) {
            timeLimit = chrono::milliseconds(doc["timeLimitInMilliseconds"].GetInt64());
        }

        string previousStage = "";
        bool hasPreviousStage = false;
        if (doc.HasMember("previousStage")) {
            previousStage = move(string(doc["previousStage"].GetString()));
            if (previousStage.length() > 0) {
                hasPreviousStage = true;
            }
        }

        StartTimes startTimes;
        bool useInitStartTimes = false;
        if (doc.HasMember("initStartTimes")) {
            useInitStartTimes = true;
            if (doc.HasMember("useInitStartTimes")) {
                useInitStartTimes = doc["useInitStartTimes"].GetBool();
            }
            startTimes = StartTimes(JsonUtils::getVector<int>(doc, "initStartTimes"));
        }

        // Solver stage.
        auto const &solverStage = doc["solverStage"];
        const string &solverName = solverStage.FindMember("name")->value.GetString();
        Solver::SpecialisedConfig specialisedConfig;
        for (auto itCfg = solverStage.FindMember("cfg")->value.MemberBegin(); itCfg != solverStage.FindMember("cfg")->value.MemberEnd(); itCfg++) {
            specialisedConfig.addValue(solverName, string(itCfg->name.GetString()), move(string(itCfg->value.GetString())));
        }

        return new SolverPrescription(
                new Solver::Config(timeLimit, new TotalTardiness(), useInitStartTimes, startTimes, specialisedConfig),
                gurobiEnvParams,
                solverName,
                hasPreviousStage,
                previousStage);
    }

    const Solver::Config& SolverPrescription::getConfig() const {
        return *mConfig;
    }

    std::string SolverPrescription::getPreviousStage() const {
        return mPreviousStage;
    }

    bool SolverPrescription::hasPreviousStage() const {
        return mHasPreviousStage;
    }

    Solver* SolverPrescription::createSolver(const Instance &ins) {
        if (mSolverStage == GreedyHeuristics::KEY_SOLVER) {
            return new GreedyHeuristics(ins);
        } else if (mSolverStage == TabuSearch::KEY_SOLVER) {
            return new TabuSearch(ins);
        } else if (mSolverStage == LazyConstraints::KEY_SOLVER) {
            return new LazyConstraints(ins, mEnv);
        } else if (mSolverStage == BranchAndBoundOnOrder::KEY_SOLVER) {
            return new BranchAndBoundOnOrder(ins);
        } else {
            cout << "Unkown solver " << mSolverStage << endl;
            exit(1);
        };
    }

    void SolverPrescription::fillGurobiEnvParams(const map<string, string> &gurobiEnvParams) {
        for (auto &gurobiEnvParam: gurobiEnvParams) {
            mEnv.set(gurobiEnvParam.first, gurobiEnvParam.second);
        }
    }
}
