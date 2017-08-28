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

#ifndef ROBUSTENERGYAWARESCHEDULING_SOLVER_H
#define ROBUSTENERGYAWARESCHEDULING_SOLVER_H

#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include "../utils/StartTimes.h"
#include "../objectives/Objective.h"

namespace rseclp {
    using namespace std;

    enum FeasibilityResult {
        FEASIBLE = 0,
        INFEASIBLE = 1
    };

    class Solver {
    public:
        class Config;

        class Status;

        class SpecialisedConfig {
        private:
            map<string, map<string, string>> mSolverToParams;

        public:
            void addValue(const string &keySolver, const string &key, const string &value);

            void addValue(const string &keySolver, const string &key, const bool value);

            void addValue(const string &keySolver, const string &key, const int value);

            bool testAndGetValue(const string &keySolver, const string &key, string &valueOut) const;

            bool testAndGetValue(const string &keySolver, const string &key, bool &valueOut) const;

            int testAndGetValue(const string &keySolver, const string &key, int &valueOut) const;

            template<typename T>
            T getValue(const string &keySolver, const string &key, T defaultValue) const {
                T value;
                return testAndGetValue(keySolver, key, value) ? value : defaultValue;
            }

        };

        class Config {
        private:
            const chrono::milliseconds mTimeLimit;
            const Objective *mObjective;
            const bool mUseInitStartTimes;
            const StartTimes mInitStartTimes;
            const SpecialisedConfig mSpecialisedConfig;

        public:
            Config(chrono::milliseconds timeLimit,
                   const Objective *objective,
                   bool useInitStartTimes,
                   StartTimes initStartTimes,
                   SpecialisedConfig specialisedConfig);

            const chrono::milliseconds &getTimeLimit() const;

            const Objective *getObjective() const;

            bool getUseInitStartTimes() const;

            const StartTimes &getInitStartTimes() const;

            const SpecialisedConfig &getSpecialisedConfig() const;
        };

        class Result {
        public:
            enum Status {
                NO_SOLUTION = 0,
                OPTIMAL = 1,
                INFEASIBLE = 2,
                FEASIBLE = 3,
                // TODO: put here OPTIMAL_NO_DEVIATIONS
            };

            Result(const int numOperations, const double objectiveValue);

            Result(Status status,
                   StartTimes startTimes,
                   double objectiveValue,
                   chrono::milliseconds solverRuntime,
                   map<string, string> optional);

            void setStatus(const Status status);

            void setSolution(const Status status, const StartTimes &startTimes, const double objectiveValue);

            void setSolverRuntime(const chrono::milliseconds &solverRuntime);

            void setOptional(const string &key, const string &value);

            Status getStatus() const;

            const StartTimes &getStartTimes() const;

            double getObjectiveValue() const;

            const chrono::milliseconds &getSolverRuntime() const;

            const map<string, string> &getOptional() const;

        private:
            Status mStatus;
            StartTimes mStartTimes;
            double mObjectiveValue;
            chrono::milliseconds mSolverRuntime;
            map<string, string> mOptional;
        };

    public:
        virtual ~Solver();

        virtual Result solve(const Config &cfg) = 0;
    };
}

#endif //ROBUSTENERGYAWARESCHEDULING_SOLVER_H
