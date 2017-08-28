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

#ifndef ROBUSTENERGYAWARESCHEDULING_INSTANCE_H
#define ROBUSTENERGYAWARESCHEDULING_INSTANCE_H

#include <cstdlib>
#include <vector>
#include <map>
#include "MeteringInterval.h"
#include "Operation.h"

namespace rseclp {

    using namespace std;

    class Instance {

    private:
        const vector<const Operation*> mOperations;
        const vector<const MeteringInterval*> mMeteringIntervals;
        const int mLengthMeteringInterval;
        const int mMaxDeviation;
        const int mHorizon;
        int mMaximumStartTime;
        map<string, string> mMetadata;

        Instance(const vector<const Operation*> &operations,
                 const vector<const MeteringInterval*> &meteringIntervals,
                 const int lengthMeteringInterval,
                 const int maxDeviation,
                 map<string, string> metadata);

    public:
        ~Instance();

        const vector<const MeteringInterval*> &getMeteringIntervals() const;

        const MeteringInterval *getMeteringInterval(const int index) const;

        const vector<const Operation*> &getOperations() const;

        vector<const Operation*> getOperationsByOrder(const vector<int> &orderedOperationIndices) const;

        const Operation *getOperation(const int index) const;

        int getLengthMeteringInterval() const;

        int getMaxDeviation() const;

        int getNumOperations() const;

        int getNumMeteringIntervals() const;

        int getMaximumStartTime() const;

        int getHorizon() const;

        vector<int> collectReleaseTimes() const;

        vector<int> collectDueDates() const;

        vector<int> collectProcessingTimes() const;

        vector<double> collectPowerConsumptions() const;

        vector<double> collectMaxEnergyConsumptions() const;

        const map<string, string> &getMetadata() const;

        void setMetadata(const string &key, const string &value);

        static Instance *create(const int numOperations,
                                const vector<int> &releaseTimes,
                                const vector<int> &dueDates,
                                const vector<int> &processingTimes,
                                const vector<double> &powerConsumptions,
                                const int maxDeviation,
                                const int numMeteringIntervals,
                                const int lengthMeteringInterval,
                                const vector<double> maxEnergyConsumptions,
                                const map<string, string> &metadata);

    };

}

#endif //ROBUSTENERGYAWARESCHEDULING_INSTANCE_H
