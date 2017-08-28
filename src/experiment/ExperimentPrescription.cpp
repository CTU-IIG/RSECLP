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

#include "ExperimentPrescription.h"
#include "../rseclp/utils/JsonUtils.h"

namespace rseclp {

    ExperimentPrescription::SolverStage::SolverStage(string name,
                                                                 map<string, string> specialisedConfig)
            : mName(move(name)), mSpecialisedConfig(move(specialisedConfig)) {}

    const string &ExperimentPrescription::SolverStage::getName() const {
        return mName;
    }

    void ExperimentPrescription::SolverStage::setSpecialisedConfig(Solver::SpecialisedConfig &specialisedConfig) const {
        for (auto &param : mSpecialisedConfig) {
            specialisedConfig.addValue(mName, param.first, param.second);
        }
    }

    ExperimentPrescription::SolverStage ExperimentPrescription::SolverStage::read(const Value &value) {
        const string &name = value.FindMember("name")->value.GetString();
        map<string, string> specialisedConfig;
        for (auto itCfg = value.FindMember("cfg")->value.MemberBegin(); itCfg != value.FindMember("cfg")->value.MemberEnd(); itCfg++) {
            specialisedConfig[string(itCfg->name.GetString())] = move(string(itCfg->value.GetString()));
        }

        return SolverStage(name, move(specialisedConfig));
    }

    ExperimentPrescription::ExperimentPrescription(string previousStage,
                                                   chrono::milliseconds timeLimit,
                                                   map<string, string> gurobiEnvParams,
                                                   SolverStage solverStage)
            : mPreviousStage(move(previousStage)),
              mTimeLimit(move(timeLimit)),
              mGurobiEnvParams(move(gurobiEnvParams)),
              mSolverStage(move(solverStage)) {}

    ExperimentPrescription ExperimentPrescription::readFromJson(const path &jsonPath) {
        Document doc;
        doc.SetObject();
        JsonUtils::readJsonDocument(jsonPath, doc, true);

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
        if (doc.FindMember("previousStage") != doc.MemberEnd()) {
            previousStage = move(string(doc["previousStage"].GetString()));
        }

        auto solverStage = SolverStage::read(doc["solverStage"]);

        return ExperimentPrescription(move(previousStage),
                                      move(timeLimit),
                                      move(gurobiEnvParams),
                                      move(solverStage));
    }

    void ExperimentPrescription::setGurobiEnvParams(GRBEnv &env) const {
        for (auto &gurobiEnvParam: mGurobiEnvParams) {
            env.set(gurobiEnvParam.first, gurobiEnvParam.second);
        }
    }

    const string &ExperimentPrescription::getPreviousStage() const {
        return mPreviousStage;
    }

    const chrono::milliseconds &ExperimentPrescription::getTimeLimit() const {
        return mTimeLimit;
    }

    const ExperimentPrescription::SolverStage &ExperimentPrescription::getSolverStage() const {
        return mSolverStage;
    }

}
