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

#include <iostream>
#include <algorithm>
#include <numeric>
#include "BruteForceSearch.h"
#include "../fixed-order/RobustScheduleFixedOrderDefault.h"
#include "../../utils/Stopwatch.h"

namespace rseclp {
    BruteForceSearch::BruteForceSearch(const Instance &ins) : mIns(ins) {}

    Solver::Result BruteForceSearch::solve(const Solver::Config &cfg) {
        Stopwatch stopwatch;
        stopwatch.start();

        const Objective &obj = *cfg.getObjective();
        RobustScheduleFixedOrderDefault alg(mIns);

        vector<int> orderedOperationIndices(mIns.getNumOperations(), 0);
        iota(orderedOperationIndices.begin(), orderedOperationIndices.end(), 0);

        Result result(mIns.getNumOperations(),
                      cfg.getUseInitStartTimes() ? obj.compute(mIns, cfg.getInitStartTimes()) : obj.worstValue());
        bool timeLimitReached = false;
        do {
            if (stopwatch.timeLimitReached(cfg.getTimeLimit())) {
                timeLimitReached = true;
                break;
            }

            if (alg.create(mIns.getOperationsByOrder(orderedOperationIndices)) == FeasibilityResult::FEASIBLE) {
                double objVal = obj.compute(mIns, alg.getStartTimes());
                if (obj.isBetter(objVal, result.getObjectiveValue())) {
                    result.setSolution(Solver::Result::Status::FEASIBLE, alg.getStartTimes(), objVal);
                }
            }
        } while (next_permutation(orderedOperationIndices.begin(), orderedOperationIndices.end()));

        if (!timeLimitReached) {
            if (result.getStatus() ==Solver::Result::Status::FEASIBLE) {
                result.setStatus(Solver::Result::Status::OPTIMAL);
            }
            else {
                result.setStatus(Solver::Result::Status::INFEASIBLE);
            }
        }

        stopwatch.stop();
        result.setSolverRuntime(stopwatch.duration());

        return result;
    }
}
