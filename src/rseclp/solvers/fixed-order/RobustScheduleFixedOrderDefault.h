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

#ifndef ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDERDEFAULT_H
#define ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDERDEFAULT_H


#include "../../instance/Instance.h"
#include "../../utils/StartTimes.h"
#include "../../solvers/Solver.h"

#ifdef _2016B
#include "../../solvers/fixed-order/RobustScheduleFixedOrder2016B.h"
#else
#include "../../solvers/fixed-order/RobustScheduleFixedOrderOptimised.h"
#endif

namespace rseclp {
    using namespace std;

#ifdef _2016B
    class RobustScheduleFixedOrderDefault : public RobustScheduleFixedOrder2016B {
#else
    class RobustScheduleFixedOrderDefault : public RobustScheduleFixedOrderOptimised {
#endif

    public:
        RobustScheduleFixedOrderDefault(const Instance &ins);
    };

}


#endif //ROBUSTENERGYAWARESCHEDULING_ROBUSTSCHEDULEFIXEDORDERDEFAULT_H
