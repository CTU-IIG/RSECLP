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
#include "Instance.h"

namespace rseclp {

    Instance::Instance(const vector<const Operation*> &operations,
                       const vector<const MeteringInterval*> &meteringIntervals,
                       const int lengthMeteringInterval,
                       const int maxDeviation,
                       map<string, string> metadata)
            : mOperations(operations),
              mMeteringIntervals(meteringIntervals),
              mLengthMeteringInterval(lengthMeteringInterval),
              mMaxDeviation(maxDeviation),
              mHorizon((int)meteringIntervals.size() * lengthMeteringInterval),
              mMetadata(move(metadata)) {
        int maxProcessingTime = 0;
        for (const Operation *operation : operations) {
            maxProcessingTime = max(maxProcessingTime, operation->getProcessingTime());
        }

        mMaximumStartTime = (int)meteringIntervals.size() * lengthMeteringInterval - (maxProcessingTime + (int) operations.size() * maxDeviation);
    }

    Instance::~Instance() {
        for (auto it = mOperations.begin(); it != mOperations.end(); it++) {
            delete *it;
        }

        for (auto it = mMeteringIntervals.begin(); it != mMeteringIntervals.end(); it++) {
            delete *it;
        }
    }

    int Instance::getNumOperations() const {
        return (int)mOperations.size();
    }

    int Instance::getNumMeteringIntervals() const {
        return (int)mMeteringIntervals.size();
    }

    int Instance::getMaximumStartTime() const {
        return mMaximumStartTime;
    }

    const MeteringInterval *Instance::getMeteringInterval(const int index) const {
        return mMeteringIntervals[index];
    }

    const vector<const MeteringInterval*> &Instance::getMeteringIntervals() const {
        return mMeteringIntervals;
    }

    vector<const Operation*> Instance::getOperationsByOrder(const vector<int> &orderedOperationIndices) const {
        assert(orderedOperationIndices.size() == mOperations.size());

        vector<const Operation*> ordered(getNumOperations(), nullptr);
        for (int position = 0; position < getNumOperations(); position++) {
            ordered[position] = getOperation(orderedOperationIndices[position]);
        }
        return ordered;
    }

    const vector<const Operation*> &Instance::getOperations() const {
        return mOperations;
    }

    const Operation *Instance::getOperation(const int index) const {
        return mOperations[index];
    }

    const map<string, string> &Instance::getMetadata() const {
        return mMetadata;
    }

    void Instance::setMetadata(const string &key, const string &value) {
        mMetadata[key] = value;
    }

    Instance* Instance::create(const int numOperations,
                               const vector<int> &releaseTimes,
                               const vector<int> &dueDates,
                               const vector<int> &processingTimes,
                               const vector<double> &powerConsumptions,
                               const int maxDeviation,
                               const int numMeteringIntervals,
                               const int lengthMeteringInterval,
                               const vector<double> maxEnergyConsumptions,
                               const map<string, string> &metadata) {
        assert(numOperations >= 0);
        assert((int) releaseTimes.size() == numOperations);
        assert((int) dueDates.size() == numOperations);
        assert((int) processingTimes.size() == numOperations);
        assert((int) powerConsumptions.size() == numOperations);
        assert(maxDeviation >= 0);
        assert(numMeteringIntervals >= 0);
        assert(lengthMeteringInterval >= 0);
        assert((int) maxEnergyConsumptions.size() == numMeteringIntervals);

        vector<const Operation*> operations;
        for (int operationIndex = 0; operationIndex < numOperations; operationIndex++) {
            operations.push_back(new Operation(operationIndex,
                                               releaseTimes[operationIndex],
                                               dueDates[operationIndex],
                                               processingTimes[operationIndex],
                                               powerConsumptions[operationIndex]));
        }

        vector<const MeteringInterval*> meteringIntervals;
        for (int meteringIntervalIndex = 0; meteringIntervalIndex < numMeteringIntervals; meteringIntervalIndex++) {
            meteringIntervals.push_back(new MeteringInterval(meteringIntervalIndex,
                                                             lengthMeteringInterval,
                                                             maxEnergyConsumptions[meteringIntervalIndex]));
        }

        return new Instance(operations, meteringIntervals, lengthMeteringInterval, maxDeviation, metadata);
    }

    int Instance::getLengthMeteringInterval() const {
        return mLengthMeteringInterval;
    }

    int Instance::getMaxDeviation() const {
        return mMaxDeviation;
    }

    int Instance::getHorizon() const {
        return mHorizon;
    }

    vector<int> Instance::collectDueDates() const {
        vector<int> dueDates;
        for (auto *pOperation : mOperations) {
            dueDates.push_back(pOperation->getDueDate());
        }

        return dueDates;
    }

    vector<int> Instance::collectReleaseTimes() const {
        vector<int> releaseTimes;
        for (auto *pOperation : mOperations) {
            releaseTimes.push_back(pOperation->getReleaseTime());
        }

        return releaseTimes;
    }

    vector<int> Instance::collectProcessingTimes() const {
        vector<int> processingTimes;
        for (auto *pOperation : mOperations) {
            processingTimes.push_back(pOperation->getProcessingTime());
        }

        return processingTimes;
    }

    vector<double> Instance::collectPowerConsumptions() const {
        vector<double> powerConsumptions;
        for (auto *pOperation : mOperations) {
            powerConsumptions.push_back(pOperation->getPowerConsumption());
        }

        return powerConsumptions;
    }

    vector<double> Instance::collectMaxEnergyConsumptions() const {
        vector<double> maxEnergyConsumptions;
        for (auto *pMeteringInterval : mMeteringIntervals) {
            maxEnergyConsumptions.push_back(pMeteringInterval->getMaxEnergyConsumption());
        }

        return maxEnergyConsumptions;
    }
}
