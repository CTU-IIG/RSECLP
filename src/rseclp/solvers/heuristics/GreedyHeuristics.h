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

#ifndef ROBUSTENERGYAWARESCHEDULING_GREEDYHEURISTICS_H
#define ROBUSTENERGYAWARESCHEDULING_GREEDYHEURISTICS_H

#include <algorithm>
#include <numeric>
#include "../../instance/Instance.h"
#include "../Solver.h"

namespace rseclp {
    using namespace std;

    class GreedyHeuristics : public Solver {
    private:
        const Instance &mIns;

    public:
        static const string KEY_SOLVER;

        class Config {
        public:
            static const string RULE_DUE_DATES;
            static const string RULE_RELEASE_TIMES;
            static const string RULE_PROCESSING_TIMES;
            static const string RULE_POWER_CONSUMPTIONS;
            static const string RULE_TARDINESS;
            static const string RULE_RANDOM;

        public:
            static const string KEY_RULE;
            static const string KEY_ASCENDING;

            const string mRule;
            const bool mAscending;

            Config(const string rule, const bool ascending);

            static Config createFrom(const Solver::Config &cfg);
        };

        GreedyHeuristics(const Instance &ins);

        vector<const Operation*> ruleDueDates(bool ascending);

        vector<const Operation*> ruleReleaseTimes(bool ascending);

        vector<const Operation*> ruleProcessingTimes(bool ascending);

        vector<const Operation*> rulePowerConsumptions(bool ascending);

        vector<const Operation*> ruleTardiness();

        vector<const Operation*> ruleRandom();

        virtual Solver::Result solve(const Solver::Config &cfg);
    };
}

#endif //ROBUSTENERGYAWARESCHEDULING_GREEDYHEURISTICS_H
