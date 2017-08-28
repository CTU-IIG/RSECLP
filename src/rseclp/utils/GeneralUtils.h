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

#ifndef ROBUSTENERGYAWARESCHEDULING_GENERALUTILS_H
#define ROBUSTENERGYAWARESCHEDULING_GENERALUTILS_H

#include <algorithm>
#include <string>
#include <gurobi_c++.h>
#include <tuple>
#include "MultiArray.h"
#include "StartTimes.h"

namespace rseclp {
    using namespace std;

    class GeneralUtils {
    public:
        static int computeIntervalsIntersectionLength(const int start1,
                                                      const int end1,
                                                      const int start2,
                                                      const int end2);

        static bool isGreater(const double x, const double y, const double tolerance = 0.000001);
        static bool areClose(const double x, const double y, const double tolerance = 0.000001);

        template <typename T>
        static double average(const vector<T> values) {
            double sum = 0.0;
            for (auto &value : values) {
                sum += value;
            }

            return sum / ((double)values.size());
        }

        template <typename T>
        static T sum(const vector<T> values) {
            T sum = 0;
            for (auto &value : values) {
                sum += value;
            }

            return sum;
        }

        template <typename T, typename RNG>
        static const T &randomElement(const vector<T> &values, RNG &randomEngine) {
            uniform_int_distribution<int> dist(0, (int)values.size() - 1);
            return values[dist(randomEngine)];
        }

        template <typename RNG>
        static pair<int, int> twoDifferentRandomIntegers(int lowerBound, int upperBound, RNG &randomEngine) {
            int val1 = uniform_int_distribution<int>(lowerBound, upperBound)(randomEngine);

            int val2;
            if (val1 == lowerBound) {
                val2 = uniform_int_distribution<int>(lowerBound + 1, upperBound)(randomEngine);
            }
            else if (val1 == upperBound) {
                val2 = uniform_int_distribution<int>(lowerBound, upperBound - 1)(randomEngine);
            }
            else {
                if (uniform_int_distribution<int>(0, 1)(randomEngine) == 0) {
                    val2 = uniform_int_distribution<int>(lowerBound, val1 - 1)(randomEngine);
                }
                else {
                    val2 = uniform_int_distribution<int>(val1 + 1, upperBound)(randomEngine);
                }
            }

            return pair<int, int>(val1, val2);
        }

        static void computeRightShiftStartTimes(const vector<const Operation*> &ordered,
                                                const StartTimes &latestStartTimes,
                                                StartTimes &rightShiftStartTimes,
                                                int forPosition,
                                                int t,
                                                const MeteringInterval *pMeteringInterval = nullptr);

        static void computeLatestStartTimes(const Instance &ins,
                                            const vector<const Operation*> &ordered,
                                            const StartTimes &baselineStartTimes,
                                            StartTimes &latestStartTimes,
                                            int upToPosition);

        static void computeLatestStartTime(const Instance &ins,
                                           const vector<const Operation*> &ordered,
                                           const StartTimes &baselineStartTimes,
                                           StartTimes &latestStartTimes,
                                           int forPosition);

        const static MeteringInterval *firstNonZeroIntersectionMeteringInterval(const Instance &ins,
                                                                                const int startTime);

        const static MeteringInterval *lastNonZeroIntersectionMeteringInterval(const Instance &ins,
                                                                               const int completionTime);

        static double computeEnergyConsumptionInMeteringInterval(const vector<const Operation *> &ordered,
                                                                 const StartTimes &startTimes,
                                                                 int upToPosition,
                                                                 const MeteringInterval &meteringInterval);
    };

}


#endif //ROBUSTENERGYAWARESCHEDULING_GENERALUTILS_H
