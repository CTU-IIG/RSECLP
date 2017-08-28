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
#include "RobustScheduleFixedOrderOptimised.h"
#include "../../utils/GeneralUtils.h"

namespace rseclp {

    RobustScheduleFixedOrderOptimised::RobustScheduleFixedOrderOptimised(const Instance &ins)
            : RobustScheduleFixedOrder(ins),
              mIns(ins),
              mStartTimes(ins.getNumOperations()),
              mLatestStartTimes(ins.getNumOperations()),
              mRightShiftStartTimes(ins.getNumOperations()) {
    }

    FeasibilityResult RobustScheduleFixedOrderOptimised::appendPosition(const vector<const Operation*> &ordered, const int position) {
        const Operation &operation = *ordered[position];

        if (position == 0) {
            mStartTimes[operation] = operation.getReleaseTime();
        }
        else {
            int prevPosition = position - 1;
            const Operation &prevOperation = *ordered[prevPosition];

            bool stop = false;

            // Init start time.
            mStartTimes[operation] = max(operation.getReleaseTime(),
                                         mStartTimes[prevOperation] + prevOperation.getProcessingTime());

            auto itMeteringInterval = mIns.getMeteringIntervals().cbegin() + ((mLatestStartTimes[prevOperation] + prevOperation.getProcessingTime()) / mIns.getLengthMeteringInterval());
            if ((mLatestStartTimes[prevOperation] + prevOperation.getProcessingTime()) == (*itMeteringInterval)->getStart()) {
                // So that prevOperation intersects at least a bit the metering interval.
                if (itMeteringInterval == mIns.getMeteringIntervals().cbegin()) {
                    stop = true;
                }
                else {
                    itMeteringInterval--;
                }
            }

            while (!stop) {
                // Loop over metering intervals.
                const MeteringInterval &meteringInterval = **itMeteringInterval;

                int t = min(meteringInterval.getEnd() - prevOperation.getProcessingTime() - 1,
                            mLatestStartTimes[prevOperation]);
                if (t < mStartTimes[prevOperation]) {
                    stop = true;
                    continue;
                }

                computeRightShiftStartTimes(ordered, prevPosition, prevPosition, t, meteringInterval);
                int firstIntersectingPosition = findFirstIntersectingPositionInRightShiftStartTimes(ordered,
                                                                                                    prevPosition,
                                                                                                    meteringInterval);
                assert(firstIntersectingPosition >= 0);

                computeRightShiftStartTimes(ordered,
                                            firstIntersectingPosition,
                                            prevPosition,
                                            computeLeftShiftStartTimeFromRightShiftStartTimes(ordered,
                                                                                              firstIntersectingPosition,
                                                                                              meteringInterval),
                                            meteringInterval);

                bool continueWithPreviousMeteringInterval = false;
                while (!stop && !continueWithPreviousMeteringInterval) {
                    const Operation &firstIntersectingOperation = *ordered[firstIntersectingPosition];
                    int earliestStartTime = computeEarliestStartTimeDuePreceeding(ordered,
                                                                                  firstIntersectingPosition,
                                                                                  position,
                                                                                  meteringInterval);

                    if (firstIntersectingPosition == position) {
                        if (meteringInterval.getStart() < earliestStartTime) {
                            mStartTimes[operation] = earliestStartTime;
                            stop = true;
                        }
                        else {
                            continueWithPreviousMeteringInterval = true;
                        }
                    }
                    else if ((mRightShiftStartTimes[prevOperation] + prevOperation.getProcessingTime()) < earliestStartTime) {
                        mStartTimes[operation] = earliestStartTime;
                        stop = true;
                    }
                    else if (mStartTimes[firstIntersectingOperation] == mRightShiftStartTimes[firstIntersectingOperation]) {
                        // We cannot move further into the left.
                        stop = true;
                    }
                    else if ((earliestStartTime + operation.getProcessingTime()) <= meteringInterval.getEnd()) {
                        // Operation is fully contained in metering interval.
                        continueWithPreviousMeteringInterval = true;
                    }
                    else if (firstIntersectingOperation.getPowerConsumption() >= operation.getPowerConsumption()) {
                        // firstIntersectingOperation can be completely shifted out of interval without violating
                        // energy limit.
                        firstIntersectingPosition += 1;
                    }
                    else {
                        // Step to the left by one.
                        computeRightShiftStartTimes(ordered,
                                                    firstIntersectingPosition,
                                                    prevPosition,
                                                    mRightShiftStartTimes[firstIntersectingOperation] - 1,  // This is feasible due to previous case.
                                                    meteringInterval);
                        if ((mRightShiftStartTimes[firstIntersectingOperation] + firstIntersectingOperation.getProcessingTime()) <= meteringInterval.getStart()) {
                            firstIntersectingPosition += 1;
                        }
                    }
                }

                if (continueWithPreviousMeteringInterval) {
                    if (itMeteringInterval == mIns.getMeteringIntervals().cbegin()) {
                        stop = true;
                    }
                    else {
                        itMeteringInterval--;
                    }
                }
            }
        }

        // Computation of start time due to only ordered[position].
        mStartTimes.computeLatestStartTime(mIns, ordered, position, mLatestStartTimes);
        int initMeteringIntervalIndex = mStartTimes[operation] / mIns.getLengthMeteringInterval();
        for (int meteringIntervalIndex = initMeteringIntervalIndex; meteringIntervalIndex != mIns.getNumMeteringIntervals(); meteringIntervalIndex++) {
            const MeteringInterval &meteringInterval = *mIns.getMeteringInterval(meteringIntervalIndex);

            int maxIntersection = (int)(meteringInterval.getMaxEnergyConsumption() / operation.getPowerConsumption());
            int intersection = min(operation.getProcessingTime(),
                                   GeneralUtils::computeIntervalsIntersectionLength(meteringInterval.getStart(),
                                                                                    meteringInterval.getEnd(),
                                                                                    mStartTimes[operation],
                                                                                    mLatestStartTimes[operation] + operation.getProcessingTime()));

            // Invariant: intersection is bounded above by the length of metering interval.
            assert(intersection <= mIns.getLengthMeteringInterval());

            if (intersection == 0) {
                break;
            }

            if (maxIntersection < intersection) {
                mStartTimes[operation] = meteringInterval.getEnd() - maxIntersection;
                mStartTimes.computeLatestStartTime(mIns, ordered, position, mLatestStartTimes);
            }
        }

        if (mStartTimes[operation] > mIns.getMaximumStartTime()) {
            return FeasibilityResult::INFEASIBLE;
        }

        return FeasibilityResult::FEASIBLE;
    }

    void RobustScheduleFixedOrderOptimised::computeRightShiftStartTimes(const vector<const Operation*> &ordered,
                                                               const int forPosition,
                                                               const int upToPosition,
                                                               const int t,
                                                               const MeteringInterval &meteringInterval) {
        // forPosition
        mRightShiftStartTimes[*ordered[forPosition]] = t;

        // position < forPosition
        for (int position = forPosition - 1; position >= 0; position--) {
            const Operation &operation = *ordered[position];
            const Operation &nextOperation = *ordered[position + 1];

            mRightShiftStartTimes[operation] = min(mLatestStartTimes[operation],
                                                   mRightShiftStartTimes[nextOperation] - operation.getProcessingTime());

            if (mRightShiftStartTimes[operation] <= meteringInterval.getStart()) {
                break;
            }
        }

        // forPosition < position <= upToPosition
        for (int position = forPosition + 1; position <= upToPosition; position++) {
            const Operation &operation = *ordered[position];
            const Operation &prevOperation = *ordered[position - 1];

            mRightShiftStartTimes[operation] = max(mStartTimes[operation],
                                                   mRightShiftStartTimes[prevOperation] + prevOperation.getProcessingTime());

            if (meteringInterval.getEnd() <= (mRightShiftStartTimes[operation] + operation.getProcessingTime())) {
                break;
            }
        }
    }

    int RobustScheduleFixedOrderOptimised::findFirstIntersectingPositionInRightShiftStartTimes(const vector<const Operation*> &ordered,
                                                                                      const int upToPosition,
                                                                                      const MeteringInterval &meteringInterval) {
        int lastNonZeroIntersectingPosition = -1;
        for (int position = upToPosition; position >= 0; position--) {
            const Operation &operation = *ordered[position];

            int startTime = mRightShiftStartTimes[operation];
            int completionTime = startTime + operation.getProcessingTime();

            int intersection = GeneralUtils::computeIntervalsIntersectionLength(meteringInterval.getStart(),
                                                                                meteringInterval.getEnd(),
                                                                                startTime,
                                                                                completionTime);

            if (intersection == 0) {
                if (lastNonZeroIntersectingPosition >= 0) {
                    return lastNonZeroIntersectingPosition;
                }
            } else {
                lastNonZeroIntersectingPosition = position;
            }

            if (startTime <= meteringInterval.getStart()) {
                break;
            }
        }

        return lastNonZeroIntersectingPosition;
    }

    int RobustScheduleFixedOrderOptimised::computeLeftShiftStartTimeFromRightShiftStartTimes(const vector<const Operation*> &ordered,
                                                                                    int firstIntersectingPosition,
                                                                                    const MeteringInterval &meteringInterval) {
        const Operation &firstIntersectingOperation = *ordered[firstIntersectingPosition];
        return max(mStartTimes[firstIntersectingOperation],
                   min(meteringInterval.getStart(), mRightShiftStartTimes[firstIntersectingOperation]));
    }

    int RobustScheduleFixedOrderOptimised::computeEarliestStartTimeDuePreceeding(const vector<const Operation*> &ordered,
                                                                        int firstIntersectingPosition,
                                                                        int forPosition,
                                                                        const MeteringInterval &meteringInterval) {
        assert(forPosition > 0);

        const Operation &operation = *ordered[forPosition];
        double energyConsumption = computeEnergyConsumption(ordered,
                                                            firstIntersectingPosition,
                                                            forPosition - 1,
                                                            meteringInterval);
        double remainingEnergyConsumption = max(0.0, meteringInterval.getMaxEnergyConsumption() - energyConsumption);
        int maxIntersection = (int)(remainingEnergyConsumption / operation.getPowerConsumption());

        int earliestStartTime = mStartTimes[operation];
        if (maxIntersection < operation.getProcessingTime()) {
            earliestStartTime = max(earliestStartTime, meteringInterval.getEnd() - maxIntersection);
        }

        return earliestStartTime;
    }

    double RobustScheduleFixedOrderOptimised::computeEnergyConsumption(const vector<const Operation*> &ordered,
                                                              int firstIntersectingPosition,
                                                              int upToPosition,
                                                              const MeteringInterval &meteringInterval) {
        double energyConsumption = 0.0;
        for (int position = upToPosition; position >= firstIntersectingPosition; position--) {
            const Operation &operation = *ordered[position];

            int startTime = mRightShiftStartTimes[operation];
            int completionTime = startTime + operation.getProcessingTime();

            int intersection = GeneralUtils::computeIntervalsIntersectionLength(meteringInterval.getStart(),
                                                                                meteringInterval.getEnd(),
                                                                                startTime,
                                                                                completionTime);
            energyConsumption += operation.getPowerConsumption() * intersection;
        }

        return energyConsumption;
    }

    const StartTimes &RobustScheduleFixedOrderOptimised::getStartTimes() const {
        return mStartTimes;
    }
}
