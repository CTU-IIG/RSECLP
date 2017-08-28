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
#include <cmath>
#include "GeneralUtils.h"

namespace rseclp {

    int GeneralUtils::computeIntervalsIntersectionLength(const int start1,
                                                         const int end1,
                                                         const int start2,
                                                         const int end2) {
        assert(start1 <= end1);
        assert(start2 <= end2);

        return max(0, min(end1, end2) - max(start1, start2));
    }

    bool GeneralUtils::isGreater(const double x, const double y, const double tolerance) {
        return x > y && (abs(x - y) > tolerance);
    }

    bool GeneralUtils::areClose(const double x, const double y, const double tolerance) {
        return abs(x - y) <= tolerance;
    }

    void GeneralUtils::computeRightShiftStartTimes(const vector<const Operation *> &ordered,
                                                   const StartTimes &latestStartTimes,
                                                   StartTimes &rightShiftStartTimes,
                                                   int forPosition,
                                                   int t,
                                                   const MeteringInterval *pMeteringInterval) {
        rightShiftStartTimes[*ordered[forPosition]] = t;
        for (int position = forPosition - 1; position >= 0; position--) {
            const Operation &operation = *ordered[position];
            const Operation &nextOperation = *ordered[position + 1];

            rightShiftStartTimes[operation] = min(latestStartTimes[operation],
                                                  rightShiftStartTimes[nextOperation] - operation.getProcessingTime());

            if (pMeteringInterval != nullptr) {
                if (rightShiftStartTimes[operation] + operation.getProcessingTime() <= pMeteringInterval->getStart()) {
                    break;
                }
            }
        }
    }

    void GeneralUtils::computeLatestStartTimes(const Instance &ins,
                                               const vector<const Operation*> &ordered,
                                               const StartTimes &baselineStartTimes,
                                               StartTimes &latestStartTimes,
                                               int upToPosition) {
        for (int position = 0; position <= upToPosition; position++) {
            GeneralUtils::computeLatestStartTime(ins, ordered, baselineStartTimes, latestStartTimes, position);
        }
    }

    void GeneralUtils::computeLatestStartTime(const Instance &ins,
                                              const vector<const Operation*> &ordered,
                                              const StartTimes &baselineStartTimes,
                                              StartTimes &latestStartTimes,
                                              int forPosition) {
        const Operation &operation = *ordered[forPosition];
        if (forPosition == 0) {
            latestStartTimes[operation] = baselineStartTimes[operation.getIndex()] + ins.getMaxDeviation();
        }
        else {
            const Operation &prevOperation = *ordered[forPosition - 1];
            latestStartTimes[operation] = max(baselineStartTimes[operation.getIndex()],
                                              latestStartTimes[prevOperation] + prevOperation.getProcessingTime()) + ins.getMaxDeviation();
        }
    }

    const MeteringInterval *GeneralUtils::firstNonZeroIntersectionMeteringInterval(const Instance &ins,
                                                                                   const int startTime) {
        int meteringIntervalIndex = startTime / ins.getLengthMeteringInterval();
        if (meteringIntervalIndex >= ins.getNumMeteringIntervals()) {
            return nullptr;
        }
        else {
            return ins.getMeteringInterval(meteringIntervalIndex);
        }
    }

    const MeteringInterval *GeneralUtils::lastNonZeroIntersectionMeteringInterval(const Instance &ins,
                                                                                  const int completionTime) {
        int meteringIntervalIndex = (completionTime - 1) / ins.getLengthMeteringInterval();
        if (meteringIntervalIndex >= ins.getNumMeteringIntervals()) {
            return nullptr;
        }
        else {
            return ins.getMeteringInterval(meteringIntervalIndex);
        }
    }

    double GeneralUtils::computeEnergyConsumptionInMeteringInterval(const vector<const Operation *> &ordered,
                                                                    const StartTimes &startTimes,
                                                                    int upToPosition,
                                                                    const MeteringInterval &meteringInterval) {
        double energyConsumption = 0.0;
        for (int position = upToPosition; position >= 0; position--) {
            const Operation &operation = *ordered[position];
            int startTime = startTimes[operation.getIndex()];
            int completionTime = startTime + operation.getProcessingTime();

            if (completionTime <= meteringInterval.getStart()) {
                break;
            }

            int intersectionLength = GeneralUtils::computeIntervalsIntersectionLength(meteringInterval.getStart(),
                                                                                      meteringInterval.getEnd(),
                                                                                      startTime,
                                                                                      completionTime);
            energyConsumption += intersectionLength * operation.getPowerConsumption();
        }

        return energyConsumption;
    }

}
