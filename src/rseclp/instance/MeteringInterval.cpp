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

#include <assert.h>
#include "MeteringInterval.h"

namespace rseclp {

    MeteringInterval::MeteringInterval(const int index,
                                       const int lengthMeteringInterval,
                                       const double maxEnergyConsumption)
            : Interval(index * lengthMeteringInterval, (index + 1) * lengthMeteringInterval),
              mIndex(index),
              mMaxEnergyConsumption(maxEnergyConsumption) {
        assert(index >= 0);
        assert(lengthMeteringInterval >= 0);
        assert(maxEnergyConsumption >= 0.0);
    }

    int MeteringInterval::getIndex() const {
        return mIndex;
    }

    double MeteringInterval::getMaxEnergyConsumption() const {
        return mMaxEnergyConsumption;
    }

    ostream& operator<<(ostream &stream, const MeteringInterval &meteringInterval) {
        stream
                << "MetInt("
                << "idx=" << meteringInterval.getIndex() << ", "
                << "s=" << meteringInterval.getStart() << ", "
                << "e=" << meteringInterval.getEnd() << ", "
                << "eMax=" << meteringInterval.getMaxEnergyConsumption()
                << ")";

        return stream;
    }

}
