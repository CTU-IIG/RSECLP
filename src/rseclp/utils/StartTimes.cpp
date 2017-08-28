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

#include <algorithm>
#include "GeneralUtils.h"
#include "StartTimes.h"

namespace rseclp {

    StartTimes::StartTimes() {}

    StartTimes::StartTimes(const int numOperations) : mStartTimes(numOperations, 0) {}

    StartTimes::StartTimes(const vector<int> startTimes) : mStartTimes(startTimes) {}

    int StartTimes::getNumOperations() const {
        return (int)mStartTimes.size();
    }

    vector<const Operation*> StartTimes::getOperationsOrdered(const Instance &ins) const {
        vector<const Operation*> ordered(ins.getOperations());

        sort(ordered.begin(), ordered.end(), [this](const Operation *lhs, const Operation *rhs) {
            return mStartTimes[lhs->getIndex()] < mStartTimes[rhs->getIndex()];
        });

        return ordered;
    }

    bool StartTimes::areEnergyConsumptionLimitsViolated(const Instance &ins) const {
        return getViolatedMeteringInterval(ins) != nullptr;
    }

    const MeteringInterval *StartTimes::getViolatedMeteringInterval(const Instance &ins) const {
        vector<double> energyConsumptionPerInterval(ins.getNumMeteringIntervals(), 0.0);
        for (auto *pOperation : ins.getOperations()) {
            const Operation &operation = *pOperation;

            int startTime = mStartTimes[operation.getIndex()];
            int completionTime = mStartTimes[operation.getIndex()] + operation.getProcessingTime();
            auto &firstMeteringInterval = *GeneralUtils::firstNonZeroIntersectionMeteringInterval(ins, startTime);
            auto &lastMeteringInterval = *GeneralUtils::lastNonZeroIntersectionMeteringInterval(ins, completionTime);

            if (firstMeteringInterval.getIndex() == lastMeteringInterval.getIndex()) {
                energyConsumptionPerInterval[firstMeteringInterval.getIndex()] +=
                        operation.getProcessingTime() * operation.getPowerConsumption();
            }
            else {
                int intersection = 0;

                intersection = firstMeteringInterval.getEnd() - startTime;
                energyConsumptionPerInterval[firstMeteringInterval.getIndex()] +=
                        intersection * operation.getPowerConsumption();

                intersection = completionTime - lastMeteringInterval.getStart();
                energyConsumptionPerInterval[lastMeteringInterval.getIndex()] +=
                        intersection * operation.getPowerConsumption();
            }

            for (int meteringIntervalIndex = firstMeteringInterval.getIndex() + 1; meteringIntervalIndex < lastMeteringInterval.getIndex(); meteringIntervalIndex++) {
                int intersection = ins.getLengthMeteringInterval();
                energyConsumptionPerInterval[meteringIntervalIndex] +=
                        intersection * operation.getPowerConsumption();
            }
        }

        for (auto pMeteringInterval : ins.getMeteringIntervals()) {
            const MeteringInterval &meteringInterval = *pMeteringInterval;
            if (GeneralUtils::isGreater(energyConsumptionPerInterval[meteringInterval.getIndex()],
                                        meteringInterval.getMaxEnergyConsumption(), 0.1)) {
                return pMeteringInterval;
            }
        }

        return nullptr;
    }

    int &StartTimes::operator[](const Operation &operation) {
        return mStartTimes[operation.getIndex()];
    }

    const int &StartTimes::operator[](const Operation &operation) const {
        return mStartTimes[operation.getIndex()];
    }

    int &StartTimes::operator[](const int operationIndex) {
        return mStartTimes[operationIndex];
    }

    const int &StartTimes::operator[](const int operationIndex) const {
        return mStartTimes[operationIndex];
    }

    ostream& operator<<(ostream &stream, const StartTimes &startTimes) {
        stream << "StartTimes(";
        for (int operationIndex = 0; operationIndex < startTimes.getNumOperations(); operationIndex++) {
            if ((operationIndex + 1) < startTimes.getNumOperations()) {
                stream << startTimes[operationIndex] << ", ";
            }
            else {
                stream << startTimes[operationIndex];
            }
        }
        stream << ")";
        return stream;
    }



    void StartTimes::computeLatestStartTimes(const Instance &ins,
                                             const vector<const Operation*> &ordered,
                                             StartTimes &latestStartTimes) const {
        for (int position = 0; position < ins.getNumOperations(); position++) {
            StartTimes::computeLatestStartTime(ins, ordered, position, latestStartTimes);
        }
    }

    void StartTimes::computeLatestStartTime(const Instance &ins,
                                            const vector<const Operation*> &ordered,
                                            int forPosition,
                                            StartTimes &latestStartTimes) const {
        const Operation &operation = *ordered[forPosition];
        if (forPosition == 0) {
            latestStartTimes[operation] = mStartTimes[operation.getIndex()] + ins.getMaxDeviation();
        }
        else {
            const Operation &prevOperation = *ordered[forPosition - 1];
            latestStartTimes[operation] = max(mStartTimes[operation.getIndex()],
                                              latestStartTimes[prevOperation] + prevOperation.getProcessingTime()) + ins.getMaxDeviation();
        }
    }

    void StartTimes::computeRealisedStartTimes(const Instance &ins,
                                               const vector<const Operation*> &ordered,
                                               const vector<int> &uncertaintyScenario,
                                               StartTimes &realisedStartTimes) const {
        for (int position = 0; position < ins.getNumOperations(); position++) {
            StartTimes::computeRealisedStartTime(ins, ordered, position, uncertaintyScenario, realisedStartTimes);
        }
    }

    void StartTimes::computeRealisedStartTime(const Instance &/*ins*/,
                                              const vector<const Operation*> &ordered,
                                              int forPosition,
                                              const vector<int> &uncertaintyScenario,
                                              StartTimes &realisedStartTimes) const {
        const Operation &operation = *ordered[forPosition];
        if (forPosition == 0) {
            realisedStartTimes[operation] = mStartTimes[operation.getIndex()] + uncertaintyScenario[operation.getIndex()];
        }
        else {
            const Operation &prevOperation = *ordered[forPosition - 1];
            realisedStartTimes[operation] = max(mStartTimes[operation.getIndex()],
                                                realisedStartTimes[prevOperation] + prevOperation.getProcessingTime()) + uncertaintyScenario[operation.getIndex()];
        }
    }

    const vector<int> &StartTimes::getBackingArray() const {
        return mStartTimes;
    }
}
