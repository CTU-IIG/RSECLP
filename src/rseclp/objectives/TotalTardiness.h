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

#ifndef ROBUSTENERGYAWARESCHEDULING_TOTALTARDINESS_H
#define ROBUSTENERGYAWARESCHEDULING_TOTALTARDINESS_H

#include <set>
#include "Objective.h"
#include "../solvers/fixed-order/RobustScheduleFixedOrder.h"

namespace rseclp {
    using namespace std;

    class TotalTardiness : public Objective {
    public:
        TotalTardiness();

        virtual double worstValue() const;

        virtual bool isBetter(const double objVal1, const double objVal2) const;

        int computeForOperation(const StartTimes &startTimes, const Operation &operation) const;

        virtual double compute(const Instance &ins,
                                     const StartTimes &startTimes,
                                     const vector<const Operation*> ordered,
                                     const int upToPosition) const;

        virtual double compute(const Instance &ins, const StartTimes &startTimes) const;

        double computeLowerBoundChu(const Instance &ins,
                                    vector<const Operation*> &ordered,
                                    const StartTimes &startTimes,
                                    const set<int> remainingOperationIndices) const;
    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_TOTALTARDINESS_H
