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

#ifndef ROBUSTENERGYAWARESCHEDULING_OBJECTIVE_H
#define ROBUSTENERGYAWARESCHEDULING_OBJECTIVE_H

#include "../utils/StartTimes.h"
#include "../instance/Instance.h"

namespace rseclp {
    class Objective {
    public:
        enum Type {
            TOTAL_TARDINESS
        };

        Type getType() const;

        virtual double worstValue() const = 0;

        virtual bool isBetter(const double objVal1, const double objVal2) const = 0;

        virtual double compute(const Instance &ins,
                               const StartTimes &startTimes,
                               const vector<const Operation*> ordered,
                               const int upToPosition) const = 0;

        virtual double compute(const Instance &ins, const StartTimes &startTimes) const = 0;

    protected:
        const Type mType;

        Objective(const Type type);

        virtual ~Objective();
    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_OBJECTIVE_H
