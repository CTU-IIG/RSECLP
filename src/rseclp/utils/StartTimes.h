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

#ifndef ROBUSTENERGYAWARESCHEDULING_STARTTIMES_H_H
#define ROBUSTENERGYAWARESCHEDULING_STARTTIMES_H_H

#include "../instance/Instance.h"

namespace rseclp {
    class StartTimes {
    private:
        vector<int> mStartTimes;

    public:
        StartTimes();

        StartTimes(const int numOperations);

        StartTimes(const vector<int> startTimes);

        int getNumOperations() const;

        vector<const Operation*> getOperationsOrdered(const Instance &ins) const;

        bool areEnergyConsumptionLimitsViolated(const Instance &ins) const;

        const MeteringInterval *getViolatedMeteringInterval(const Instance &ins) const;

        const vector<int> &getBackingArray() const;

        int &operator[](const Operation &operation);

        const int &operator[](const Operation &operation) const;

        int &operator[](const int operationIndex);

        const int &operator[](const int operationIndex) const;

        void computeLatestStartTimes(const Instance &ins,
                                     const vector<const Operation*> &ordered,
                                     StartTimes &latestStartTimes) const;

        void computeLatestStartTime(const Instance &ins,
                                    const vector<const Operation*> &ordered,
                                    int forPosition,
                                    StartTimes &latestStartTimes) const;

        void computeRealisedStartTimes(const Instance &ins,
                                       const vector<const Operation*> &ordered,
                                       const vector<int> &uncertaintyScenario,
                                       StartTimes &realisedStartTimes) const;

        void computeRealisedStartTime(const Instance &/*ins*/,
                                      const vector<const Operation*> &ordered,
                                      int forPosition,
                                      const vector<int> &uncertaintyScenario,
                                      StartTimes &realisedStartTimes) const;


    };

    ostream& operator<<(ostream &stream, const StartTimes &startTimes);

}

#endif //ROBUSTENERGYAWARESCHEDULING_STARTTIMES_H_H
