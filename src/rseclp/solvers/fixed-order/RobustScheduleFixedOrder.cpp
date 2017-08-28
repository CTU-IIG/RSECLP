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
#include "RobustScheduleFixedOrder.h"
#include "../../utils/GeneralUtils.h"

namespace rseclp {

    RobustScheduleFixedOrder::RobustScheduleFixedOrder(const Instance &ins)
            : mIns(ins) { }

    FeasibilityResult RobustScheduleFixedOrder::create(const vector<const Operation*> &ordered) {
        return create(ordered, mIns.getNumOperations() - 1);
    }

    FeasibilityResult RobustScheduleFixedOrder::create(const vector<const Operation*> &ordered, const int upToPosition) {
        for (int position = 0; position <= upToPosition; position++) {
            auto feasibilityResult = appendPosition(ordered, position);
            if (feasibilityResult == FeasibilityResult::INFEASIBLE) {
                return FeasibilityResult::INFEASIBLE;
            }
        }

        return FeasibilityResult::FEASIBLE;
    }

}
