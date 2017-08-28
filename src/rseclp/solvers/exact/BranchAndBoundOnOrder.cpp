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

#include <cassert>
#include <algorithm>
#include <iostream>
#include "BranchAndBoundOnOrder.h"
#include "../../objectives/TotalTardiness.h"
#include "../../feasibility-checkers/FeasibilityChecker.h"

namespace rseclp {

    BranchAndBoundOnOrder::GlobalState::GlobalState(const Instance &ins, Result result)
            : mAlg(ins),
              mResult(move(result)),
              mCurrentOrdered(ins.getNumOperations(), nullptr) {
        mTimeLimitReached = false;
        for (int operationIndex = 0; operationIndex < ins.getNumOperations(); operationIndex++) {
            mRemainingOperationIndices.insert(operationIndex);
        }
    }

    BranchAndBoundOnOrder::BranchAndBoundOnOrder(const Instance &ins) : mIns(ins) { }

    const string BranchAndBoundOnOrder::KEY_SOLVER = "BranchAndBoundOnOrder";

    Solver::Result BranchAndBoundOnOrder::solve(const Solver::Config &cfg) {
        // TODO (refactoring): Make explicit that this B&B is for total tardiness (in name).
        assert(cfg.getObjective()->getType() == Objective::Type::TOTAL_TARDINESS);

        GlobalState globalState(mIns, Solver::Result(mIns.getNumOperations(), cfg.getObjective()->worstValue()));

        setInitialSolution(cfg, globalState);

        globalState.mStopwatch.start();
        inBranchDfs(cfg, globalState);
        if (!globalState.mTimeLimitReached) {
            if (globalState.mResult.getStatus() == Solver::Result::FEASIBLE) {
                globalState.mResult.setStatus(Solver::Result::OPTIMAL);
            }
            else {
                globalState.mResult.setStatus(Solver::Result::INFEASIBLE);
            }
        }
        globalState.mStopwatch.stop();

        globalState.mResult.setSolverRuntime(globalState.mStopwatch.duration());

        return globalState.mResult;
    }

    void BranchAndBoundOnOrder::setInitialSolution(const Solver::Config &cfg, GlobalState &globalState) {
        if (cfg.getUseInitStartTimes()) {
            auto &initStartTimes = cfg.getInitStartTimes();
            if (FeasibilityChecker(mIns).areFeasible(initStartTimes)) {
                globalState.mResult.setSolution(Solver::Result::FEASIBLE,
                                                initStartTimes,
                                                cfg.getObjective()->compute(mIns, initStartTimes));
            }
        }
    }

    void BranchAndBoundOnOrder::inBranchDfs(const Solver::Config &cfg, GlobalState &globalState) {
        // TODO (optimisation): it would be better to store some partial information regarding lower bound.
        if (globalState.mTimeLimitReached) {
            return;
        }


        if (globalState.mStopwatch.timeLimitReached(cfg.getTimeLimit())) {
            globalState.mTimeLimitReached = true;
            return;
        }

        const TotalTardiness &obj = (const TotalTardiness&) *cfg.getObjective();
        if (globalState.mRemainingOperationIndices.size() == 0) {
            // Leaf node.
            double objVal = obj.compute(mIns, globalState.mAlg.getStartTimes());
            if (obj.isBetter(objVal, globalState.mResult.getObjectiveValue())) {
                globalState.mResult.setSolution(Solver::Result::Status::FEASIBLE,
                                                globalState.mAlg.getStartTimes(),
                                                objVal);
            }
            return;
        }

        double lowerBound = 0.0;

        double lowerBoundChu = obj.computeLowerBoundChu(mIns,
                                                        globalState.mCurrentOrdered,
                                                        globalState.mAlg.getStartTimes(),
                                                        globalState.mRemainingOperationIndices);
        lowerBound = max(lowerBound, lowerBoundChu);

        if (!obj.isBetter(lowerBound, globalState.mResult.getObjectiveValue())) {
            return;
        }

        // Branch over all remaining jobs.
        auto branchOnOrder = vector<int>(globalState.mRemainingOperationIndices.begin(),
                                         globalState.mRemainingOperationIndices.end());


        // Branching priority.
        sort(branchOnOrder.begin(), branchOnOrder.end(), [&](const int lhs, const int rhs) {
            // TODO (refactoring): selectable by specialised config.
            return mIns.getOperation(lhs)->getDueDate() < mIns.getOperation(rhs)->getDueDate();
        });

        int forPosition = mIns.getNumOperations() - (int)globalState.mRemainingOperationIndices.size();
        for (int operationIndex : branchOnOrder) {
            const Operation *pOperation = mIns.getOperation(operationIndex);

            globalState.mCurrentOrdered[forPosition] = pOperation;
            globalState.mRemainingOperationIndices.erase(operationIndex);

            if (globalState.mAlg.appendPosition(globalState.mCurrentOrdered, forPosition) == FeasibilityResult::FEASIBLE) {
                // Go deeper.
                inBranchDfs(cfg, globalState);
            } else {
                // Infeasible.
                // TODO (optimisation): question is, could I stop searching in this subtree?
            }

            globalState.mRemainingOperationIndices.insert(operationIndex);

            if (globalState.mTimeLimitReached) {
                return;
            }
        }
    }
}
