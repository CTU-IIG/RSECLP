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
#include <iostream>
#include "RobustScheduleFixedOrder2016B.h"
#include "../../utils/GeneralUtils.h"

namespace rseclp {

    RobustScheduleFixedOrder2016B::RobustScheduleFixedOrder2016B(const Instance &ins)
            : RobustScheduleFixedOrder(ins),
              mIns(ins),
              mStartTimes(ins.getNumOperations()),
              mLatestStartTimes(ins.getNumOperations()),
              mRightShiftStartTimes(ins.getNumOperations()) {
    }

    FeasibilityResult RobustScheduleFixedOrder2016B::appendPosition(const vector<const Operation*> &ordered, const int position) {
        const Operation &operation = *ordered[position];
        mStartTimes[operation] = operation.getReleaseTime();

        // Earliest robust start time due to preceding operations.
        if (position > 0) {
            int prevPosition = position - 1;
            const Operation &prevOperation = *ordered[prevPosition];
            mStartTimes[operation] = max(operation.getReleaseTime(),
                                         mStartTimes[prevOperation] + prevOperation.getProcessingTime());

            int t = mLatestStartTimes[prevOperation];
            int tMin = min(mLatestStartTimes[prevOperation],
                           max(mStartTimes[prevOperation], operation.getReleaseTime() - prevOperation.getProcessingTime()));
            while (t >= tMin) {
                auto meteringInterval = *GeneralUtils::lastNonZeroIntersectionMeteringInterval(mIns,
                                                                                               t + prevOperation.getProcessingTime());
                GeneralUtils::computeRightShiftStartTimes(ordered,
                                                          mLatestStartTimes,
                                                          mRightShiftStartTimes,
                                                          prevPosition,
                                                          t,
                                                          &meteringInterval);
                int maxPossibleIntersection = (int) ((meteringInterval.getMaxEnergyConsumption() - GeneralUtils::computeEnergyConsumptionInMeteringInterval(ordered, mRightShiftStartTimes, prevPosition, meteringInterval)) / operation.getPowerConsumption());
                if (operation.getProcessingTime() <= maxPossibleIntersection) {
                    t = meteringInterval.getStart() - prevOperation.getProcessingTime() - 1;
                }
                else if (maxPossibleIntersection >= (meteringInterval.getEnd() - (mRightShiftStartTimes[prevOperation] + prevOperation.getProcessingTime()))) {
                    t -= 1;
                }
                else {
                    mStartTimes[operation] = max(operation.getReleaseTime(), meteringInterval.getEnd() - maxPossibleIntersection);
                    break;
                }
            }
        }

        // Earliest robust start time due to itself.
        GeneralUtils::computeLatestStartTime(mIns, ordered, mStartTimes, mLatestStartTimes, position);
        auto pMeteringInterval = GeneralUtils::firstNonZeroIntersectionMeteringInterval(mIns, mStartTimes[operation]);
        if (pMeteringInterval != nullptr) {
            for (auto itMeteringInterval = next(mIns.getMeteringIntervals().cbegin(), pMeteringInterval->getIndex()); itMeteringInterval != mIns.getMeteringIntervals().cend(); itMeteringInterval++) {
                pMeteringInterval = *itMeteringInterval;

                int maxNonviolatingIntersection = (int) (pMeteringInterval->getMaxEnergyConsumption() / operation.getPowerConsumption());
                int maxIntersection = min(operation.getProcessingTime(),
                                          GeneralUtils::computeIntervalsIntersectionLength(pMeteringInterval->getStart(),
                                                                                           pMeteringInterval->getEnd(),
                                                                                           mStartTimes[operation],
                                                                                           mLatestStartTimes[operation] + operation.getProcessingTime()));
                if (maxIntersection == 0) {
                    break;
                }
                else if (maxNonviolatingIntersection < maxIntersection) {
                    mStartTimes[operation] = pMeteringInterval->getEnd() - maxNonviolatingIntersection;
                    GeneralUtils::computeLatestStartTime(mIns, ordered, mStartTimes, mLatestStartTimes, position);
                }
            }
        }


        if (mStartTimes[operation] > mIns.getMaximumStartTime()) {
            return FeasibilityResult::INFEASIBLE;
        }

        return FeasibilityResult::FEASIBLE;
    }

    const StartTimes &RobustScheduleFixedOrder2016B::getStartTimes() const {
        return mStartTimes;
    }
}
