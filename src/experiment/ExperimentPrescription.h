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

#ifndef ROBUSTENERGYAWARESCHEDULING_EXPERIMENTPRESCRIPTION_H
#define ROBUSTENERGYAWARESCHEDULING_EXPERIMENTPRESCRIPTION_H

#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include <gurobi_c++.h>
#include "../rseclp/solvers/Solver.h"

namespace rseclp {
    using namespace std;
    using namespace boost::filesystem;
    using namespace rapidjson;

    class ExperimentPrescription {

    public:

        class SolverStage {
        private:
            const string mName;
            const map<string, string> mSpecialisedConfig;

        public:
            SolverStage(string name, map<string, string> specialisedConfig);

            const string &getName() const;

            void setSpecialisedConfig(Solver::SpecialisedConfig &specialisedConfig) const;

            static SolverStage read(const Value &value);
        };

    private:
        const string mPreviousStage;
        const chrono::milliseconds mTimeLimit;
        const map<string, string> mGurobiEnvParams;
        const SolverStage mSolverStage;

    public:

        ExperimentPrescription(string previousStage,
                               chrono::milliseconds timeLimit,
                               map<string, string> gurobiEnvParams,
                               SolverStage solverStage);

        void setGurobiEnvParams(GRBEnv &env) const;

        const string &getPreviousStage() const;

        const chrono::milliseconds &getTimeLimit() const;

        const SolverStage &getSolverStage() const;

        static ExperimentPrescription readFromJson(const path &jsonPath);
    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_EXPERIMENTPRESCRIPTION_H
