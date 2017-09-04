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

#ifndef ROBUSTENERGYAWARESCHEDULING_SOLVERPRESCRIPTION_H
#define ROBUSTENERGYAWARESCHEDULING_SOLVERPRESCRIPTION_H

#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include <gurobi_c++.h>
#include "Solver.h"

namespace rseclp {

    class SolverPrescription {
    private:
        std::unique_ptr<Solver::Config> mConfig;
        GRBEnv mEnv;
        const std::string mSolverStage;
        const bool mHasPreviousStage;
        const std::string mPreviousStage;

        void fillGurobiEnvParams(const map<string, string> &gurobiEnvParams);

    public:
        SolverPrescription(Solver::Config *config,
                           const map<string, string> &gurobiEnvParams,
                           const string &solverStage,
                           bool hasPreviousStage,
                           const string &previousStage);


        const Solver::Config &getConfig() const;
        Solver *createSolver(const Instance &ins);
        bool hasPreviousStage() const;
        std::string getPreviousStage() const;
        static SolverPrescription *read(const boost::filesystem::path &prescriptionPath);
    };

}



#endif //ROBUSTENERGYAWARESCHEDULING_SOLVERPRESCRIPTION_H
