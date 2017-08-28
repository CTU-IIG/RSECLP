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

#ifndef ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDER2016B_H
#define ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDER2016B_H


#include "../../instance/Instance.h"
#include "../../utils/StartTimes.h"
#include "../../solvers/Solver.h"
#include "../../solvers/fixed-order/RobustScheduleFixedOrder.h"

namespace rseclp {
    using namespace std;

    class RobustScheduleFixedOrder2016B : public RobustScheduleFixedOrder {

    private:
        const Instance &mIns;

        StartTimes mStartTimes;
        StartTimes mLatestStartTimes;
        StartTimes mRightShiftStartTimes;

    public:
        RobustScheduleFixedOrder2016B(const Instance &ins);

        virtual FeasibilityResult appendPosition(const vector<const Operation*> &ordered, const int position);

        virtual const StartTimes &getStartTimes() const;
    };

}


#endif //ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDER2016B_H

