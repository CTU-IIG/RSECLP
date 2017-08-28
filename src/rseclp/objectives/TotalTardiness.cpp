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

#include <limits>
#include <algorithm>
#include <queue>
#include <functional>
#include <iostream>
#include "TotalTardiness.h"

namespace rseclp {

    TotalTardiness::TotalTardiness() : Objective(Objective::Type::TOTAL_TARDINESS) {}

    double TotalTardiness::worstValue() const {
        return numeric_limits<double>::max();
    }

    bool TotalTardiness::isBetter(const double objVal1, const double objVal2) const {
        return objVal1 < objVal2;
    }

    int TotalTardiness::computeForOperation(const StartTimes &startTimes, const Operation &operation) const {
        return max(0, startTimes[operation] + operation.getProcessingTime() - operation.getDueDate());
    }

    double TotalTardiness::compute(const Instance &/*ins*/,
                                   const StartTimes &startTimes,
                                   const vector<const Operation*> ordered,
                                   const int upToPosition) const {
        int objVal = 0;
        for (int position = 0; position <= upToPosition; position++) {
            const Operation &operation = *ordered[position];
            objVal += computeForOperation(startTimes, operation);
        }

        return (double)objVal;
    }

    double TotalTardiness::compute(const Instance &ins, const StartTimes &startTimes) const {
        int objVal = 0;
        for (const Operation *pOperation : ins.getOperations()) {
            auto &operation = *pOperation;
            objVal += computeForOperation(startTimes, operation);
        }

        return (double)objVal;
    }

    double TotalTardiness::computeLowerBoundChu(const Instance &ins,
                                                vector<const Operation*> &ordered,
                                                const StartTimes &startTimes,
                                                const set<int> remainingOperationIndices) const {
        // Philippe Baptiste et al. A Branch-and-Bound Procedure to Minimize Total Tardiness on One Machine with Arbitrary Release Dates
        // sect. 3.1 (not primal source)
        int forPosition = ins.getNumOperations() - (int)remainingOperationIndices.size();

        // operationsSortedByReleaseTime
        // readyOperationsQueue - by shortestRemainingProcessingTime
        vector<int> remainingProcessingTimes(ins.getNumOperations(), 0);    // Use operationIndex to access.
        auto remainingProcessingTimeCmp = [&](const Operation *lhs, const Operation *rhs) {
            // return true if lhs is strictly before rhs
            return remainingProcessingTimes[lhs->getIndex()] > remainingProcessingTimes[rhs->getIndex()];
        };

        auto releaseTimeCmp = [&](const Operation *lhs, const Operation *rhs) {
            // return true if lhs is strictly before rhs
            return lhs->getReleaseTime() > rhs->getReleaseTime();
        };

        priority_queue<const Operation*, vector<const Operation*>, function<bool(const Operation*, const Operation*)>> readyOperationsHeap(remainingProcessingTimeCmp);
        priority_queue<const Operation*, vector<const Operation*>, function<bool(const Operation*, const Operation*)>> notReadyOperationsHeap(releaseTimeCmp);
        priority_queue<int, vector<int>, greater<int>> remainingDueDatesHeap;
        for (int remainingOperationIndex : remainingOperationIndices) {
            const Operation *pOperation = ins.getOperation(remainingOperationIndex);
            remainingProcessingTimes[pOperation->getIndex()] = pOperation->getProcessingTime();
            notReadyOperationsHeap.push(pOperation);
            remainingDueDatesHeap.push(pOperation->getDueDate());
        }

        int t = forPosition > 0 ? startTimes[*ordered[forPosition - 1]] + ordered[forPosition - 1]->getProcessingTime() : 0;
        int objVal = 0;
        while ((readyOperationsHeap.size() + notReadyOperationsHeap.size()) > 0) {
            if (readyOperationsHeap.size() == 0) {
                const Operation *pNewReadyOperation = notReadyOperationsHeap.top();
                notReadyOperationsHeap.pop();

                t = pNewReadyOperation->getReleaseTime();
                readyOperationsHeap.push(pNewReadyOperation);
            }

            while (notReadyOperationsHeap.size() > 0 && notReadyOperationsHeap.top()->getReleaseTime() <= t) {
                const Operation *pNewReadyOperation = notReadyOperationsHeap.top();
                notReadyOperationsHeap.pop();
                readyOperationsHeap.push(pNewReadyOperation);
            }

            const Operation *pOperationToSchedule = readyOperationsHeap.top();
            readyOperationsHeap.pop();
            int tBound = t + remainingProcessingTimes[pOperationToSchedule->getIndex()];
            if (notReadyOperationsHeap.size() > 0) {
                tBound = min(tBound, notReadyOperationsHeap.top()->getReleaseTime());
            }

            remainingProcessingTimes[pOperationToSchedule->getIndex()] -= (tBound - t);
            if (remainingProcessingTimes[pOperationToSchedule->getIndex()] > 0) {
                readyOperationsHeap.push(pOperationToSchedule);
            }
            else {
                // Operation completed at tBound.
                objVal += max(0, tBound - remainingDueDatesHeap.top());
                remainingDueDatesHeap.pop();
            }

            t = tBound;
        }

        return compute(ins, startTimes, ordered, forPosition - 1) + (double)objVal;
    }
}
