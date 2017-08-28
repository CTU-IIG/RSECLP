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

#ifndef ROBUSTENERGYAWARESCHEDULING_STOPWATCH_H
#define ROBUSTENERGYAWARESCHEDULING_STOPWATCH_H

#include <chrono>

namespace rseclp {

    using namespace std;

    class Stopwatch {
    private:
        chrono::time_point<chrono::steady_clock> mStart;
        chrono::time_point<chrono::steady_clock> mEnd;
        bool mRunning;

    public:
        Stopwatch();

        void start();

        void stop();

        chrono::milliseconds duration() const;

        bool timeLimitReached(const chrono::milliseconds &timeLimit) const;
    };

}


#endif //ROBUSTENERGYAWARESCHEDULING_STOPWATCH_H
