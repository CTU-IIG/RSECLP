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

#include <sstream>
#include "IlpUtils.h"

namespace rseclp {

    string IlpUtils::varName(const string name, initializer_list<int> indices) {
        stringstream ss;

        ss << name;
        ss << "_{";

        for (int index : indices) {
            ss << index << ",";
        }

        ss << "}";

        return ss.str();
    }

    MultiArray<int> IlpUtils::binaryVariablesToValues(const MultiArray<GRBVar> &vars) {
        MultiArray<int> result(vars.shape(), 0);
        auto &varsBackingArray = vars.backingArray();
        for (int i = 0; i < (int)varsBackingArray.size(); i++) {
            auto var = varsBackingArray[i];
            if (var.sameAs(GRBVar())) {
                result.directSet(i, -1);
            }
            else {
                result.directSet(i, var.get(GRB_DoubleAttr_X) < 0.5 ? 0 : 1);
            }
        }

        return result;
    }

}
