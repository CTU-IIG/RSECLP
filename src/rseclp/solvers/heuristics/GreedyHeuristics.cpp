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
#include <set>
#include <assert.h>
#include "GreedyHeuristics.h"
#include "../../utils/Stopwatch.h"
#include "../../utils/GeneralUtils.h"
#include "../fixed-order/RobustScheduleFixedOrderDefault.h"
#include "../../objectives/TotalTardiness.h"

namespace rseclp {
    const string GreedyHeuristics::Config::KEY_RULE = "rule";
    const string GreedyHeuristics::Config::KEY_ASCENDING = "ascending";

    const string GreedyHeuristics::Config::RULE_DUE_DATES = "due_dates";
    const string GreedyHeuristics::Config::RULE_RELEASE_TIMES = "release_times";
    const string GreedyHeuristics::Config::RULE_PROCESSING_TIMES = "processing_times";
    const string GreedyHeuristics::Config::RULE_POWER_CONSUMPTIONS = "power_consumptions";
    const string GreedyHeuristics::Config::RULE_TARDINESS = "tardiness";
    const string GreedyHeuristics::Config::RULE_RANDOM = "random";

    GreedyHeuristics::Config::Config(const string rule, const bool ascending) : mRule(rule), mAscending(ascending) { }

    GreedyHeuristics::Config GreedyHeuristics::Config::createFrom(const Solver::Config &cfg) {
        auto &scfg = cfg.getSpecialisedConfig();

        return GreedyHeuristics::Config(scfg.getValue<string>(KEY_SOLVER, KEY_RULE, RULE_TARDINESS),
                                        scfg.getValue<bool>(KEY_SOLVER, KEY_ASCENDING, true));
    }

    const string GreedyHeuristics::KEY_SOLVER = "GreedyHeuristics";

    GreedyHeuristics::GreedyHeuristics(const Instance &ins) : mIns(ins) { }


    vector<const Operation*> GreedyHeuristics::ruleDueDates(bool ascending) {
        vector<const Operation*> ordered(mIns.getOperations());

        sort(ordered.begin(), ordered.end(), [&](const Operation *lhs, const Operation *rhs) {
            return ascending ?
                   lhs->getDueDate() < rhs->getDueDate() :
                   lhs->getDueDate() > rhs->getDueDate();
        });

        return ordered;
    }

    vector<const Operation*> GreedyHeuristics::ruleReleaseTimes(bool ascending) {
        vector<const Operation*> ordered(mIns.getOperations());

        sort(ordered.begin(), ordered.end(), [&](const Operation *lhs, const Operation *rhs) {
            return ascending ?
                   lhs->getReleaseTime() < rhs->getReleaseTime() :
                   lhs->getReleaseTime() > rhs->getReleaseTime();
        });

        return ordered;
    }

    vector<const Operation*> GreedyHeuristics::ruleProcessingTimes(bool ascending) {
        vector<const Operation*> ordered(mIns.getOperations());

        sort(ordered.begin(), ordered.end(), [&](const Operation *lhs, const Operation *rhs) {
            return ascending ?
                   lhs->getProcessingTime() < rhs->getProcessingTime() :
                   lhs->getProcessingTime() > rhs->getProcessingTime();
        });

        return ordered;
    }

    vector<const Operation*> GreedyHeuristics::rulePowerConsumptions(bool ascending) {
        vector<const Operation*> ordered(mIns.getOperations());

        sort(ordered.begin(), ordered.end(), [&](const Operation *lhs, const Operation *rhs) {
            return ascending ?
                   lhs->getPowerConsumption() < rhs->getPowerConsumption() :
                   lhs->getPowerConsumption() > rhs->getPowerConsumption();
        });

        return ordered;
    }

    vector<const Operation*> GreedyHeuristics::ruleTardiness() {
        set<int> remainingOperationIndices;
        for (auto pOperation : mIns.getOperations()) {
            remainingOperationIndices.insert(pOperation->getIndex());
        }

        vector<const Operation*> ordered(mIns.getNumOperations(), nullptr);
        RobustScheduleFixedOrderDefault alg(mIns);
        TotalTardiness obj;

        for (int forPosition = 0; forPosition < mIns.getNumOperations(); forPosition++) {
            double bestObjVal = obj.worstValue();
            int bestOperationIndex = -1;
            int bestCompletionTime = numeric_limits<int>::max();

            for (int operationIndex : remainingOperationIndices) {
                const Operation *pOperation = mIns.getOperation(operationIndex);
                const Operation &operation = *pOperation;
                ordered[forPosition] = pOperation;

                if (alg.appendPosition(ordered, forPosition) == FeasibilityResult::INFEASIBLE) {
                    continue;
                }

                int completionTime = alg.getStartTimes()[operation] + operation.getProcessingTime();
                double objVal = max(0, completionTime - operation.getDueDate());
                for (int operationIndexPrime : remainingOperationIndices) {
                    if (operationIndex != operationIndexPrime) {
                        const Operation &operationPrime = *mIns.getOperation(operationIndexPrime);

                        objVal += max(0,
                                      max(completionTime, operationPrime.getReleaseTime()) + operationPrime.getProcessingTime() - operationPrime.getDueDate());
                    }
                }

                if (objVal < bestObjVal || (GeneralUtils::areClose(objVal, bestObjVal) && completionTime < bestCompletionTime)) {
                    bestObjVal = objVal;
                    bestOperationIndex = operationIndex;
                    bestCompletionTime = completionTime;
                }
            }

            if (bestOperationIndex == -1) {
                return vector<const Operation*>();
            }

            remainingOperationIndices.erase(bestOperationIndex);
            ordered[forPosition] = mIns.getOperation(bestOperationIndex);
            alg.appendPosition(ordered, forPosition);
        }

        return ordered;
    }

    vector<const Operation*> GreedyHeuristics::ruleRandom() {
        vector<const Operation*> ordered(mIns.getOperations());
        random_shuffle(ordered.begin(), ordered.end());
        return ordered;
    }

    Solver::Result GreedyHeuristics::solve(const Solver::Config &cfg) {
        Stopwatch stopwatch;
        stopwatch.start();

        auto scfg = GreedyHeuristics::Config::createFrom(cfg);

        Solver::Result result(mIns.getNumOperations(), cfg.getObjective()->worstValue());

        vector<const Operation *> ordered;
        if (scfg.mRule == Config::RULE_DUE_DATES) {
            ordered = ruleDueDates(scfg.mAscending);
        }
        else if (scfg.mRule == Config::RULE_RELEASE_TIMES) {
            ordered = ruleReleaseTimes(scfg.mAscending);
        }
        else if (scfg.mRule == Config::RULE_PROCESSING_TIMES) {
            ordered = ruleProcessingTimes(scfg.mAscending);
        }
        else if (scfg.mRule == Config::RULE_POWER_CONSUMPTIONS) {
            ordered = rulePowerConsumptions(scfg.mAscending);
        }
        else if (scfg.mRule == Config::RULE_RANDOM) {
            ordered = ruleRandom();
        }
        else if (scfg.mRule == Config::RULE_TARDINESS) {
            assert(cfg.getObjective()->getType() == Objective::Type::TOTAL_TARDINESS);
            ordered = ruleTardiness();
        }
        else {
            cout << "Invalid rule " << scfg.mRule << endl;
            exit(1);
        }

        if (ordered.size() != 0) {
            RobustScheduleFixedOrderDefault alg(mIns);
            if (alg.create(ordered) == FeasibilityResult::FEASIBLE) {
                result.setSolution(Solver::Result::Status::FEASIBLE,
                                   alg.getStartTimes(),
                                   cfg.getObjective()->compute(mIns, alg.getStartTimes()));
            }
        }

        stopwatch.stop();
        result.setSolverRuntime(stopwatch.duration());
        return result;
    }
}
