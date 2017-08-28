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

#include "FeasibilityChecker.h"

namespace rseclp {

    FeasibilityChecker::FeasibilityChecker(const Instance &ins)
            : mIns(ins),
              mLatestStartTimes(ins.getNumOperations()),
              mUncertaintyScenario(ins.getNumOperations(), 0) {}

    bool FeasibilityChecker::areFeasible(const StartTimes &startTimes) {
        StartTimes realisedStartTimes(startTimes);
        auto ordered = startTimes.getOperationsOrdered(mIns);
        startTimes.computeLatestStartTimes(mIns, ordered, mLatestStartTimes);
        fill(mUncertaintyScenario.begin(), mUncertaintyScenario.end(), 0);
        mViolatedMeteringInterval = nullptr;

        int positionArb = 0;
        while (positionArb < mIns.getNumOperations()) {
            mViolatedMeteringInterval = realisedStartTimes.getViolatedMeteringInterval(mIns);
            if (mViolatedMeteringInterval != nullptr) {
                // Reconstruct the uncertainty scenario.
                for (int position = 0; position < positionArb; position++) {
                    const Operation &operation = *ordered[position];
                    mUncertaintyScenario[operation.getIndex()] = mIns.getMaxDeviation();
                }
                const Operation &operationArb = *ordered[positionArb];
                mUncertaintyScenario[operationArb.getIndex()] = mIns.getMaxDeviation() - (mLatestStartTimes[operationArb] - realisedStartTimes[operationArb]);

                return false;
            }

            const Operation &operationArb = *ordered[positionArb];
            if ((realisedStartTimes[operationArb] + 1) > mLatestStartTimes[operationArb]) {
                positionArb += 1;
            }
            else {
                realisedStartTimes[operationArb] += 1;
                int position = positionArb + 1;
                while (position < mIns.getNumOperations()) {
                    const Operation &operation = *ordered[position];
                    const Operation &prevOperation = *ordered[position - 1];
                    if ((realisedStartTimes[prevOperation] + prevOperation.getProcessingTime()) <= realisedStartTimes[operation]) {
                        break;
                    }
                    else {
                        realisedStartTimes[operation] += 1;
                        position += 1;
                    }
                }
            }
        }

        return true;
    }
}
