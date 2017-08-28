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

#ifndef ROBUSTENERGYAWARESCHEDULING_TABUSEARCH_H
#define ROBUSTENERGYAWARESCHEDULING_TABUSEARCH_H

#include <random>
#include "../../instance/Instance.h"
#include "../Solver.h"
#include "../fixed-order/RobustScheduleFixedOrderDefault.h"
#include "../../feasibility-checkers/FeasibilityChecker.h"
#include "../../objectives/TotalTardiness.h"
#include "../../utils/Stopwatch.h"
#include "../../utils/GeneralUtils.h"

namespace rseclp {
    class TabuSearch : public Solver {
    public:
        static const string KEY_SOLVER;

        class Config {
        public:
            static const string KEY_NUM_ITERATIONS;
            static const string KEY_NUM_RESTARTS;
            static const string KEY_NEIGHBOURHOOD_SIZE;
            static const string KEY_TABU_LIST_LENGTH;
            static const string KEY_MAX_NONIMPROVING_ITERATIONS;

            const int mNumIterations;
            const int mNumRestarts;
            const int mNeighbourhoodSize;
            const int mTabuListLength;
            const int mMaxNonimprovingIterations;

            Config(const int numIterations,
                   const int numRestarts,
                   const int neighbourhoodSize,
                   const int tabuListLength,
                   const int maxNonimprovingIterations);

            static Config createFrom(const Solver::Config &cfg);
        };

        TabuSearch(const Instance &ins);

        virtual Solver::Result solve(const Solver::Config &cfg);

    private:

        // TODO (refactoring): use FeasibilityResult
        class Solution {
        public:
            vector<const Operation*> mOrdered;
            StartTimes mStartTimes;
            bool mIsFeasible;
            double mObjectiveValue;

            Solution();

            Solution(vector<const Operation *> ordered, StartTimes startTimes, bool isFeasible, double objectiveValue);
        };

        template <typename RNG>
        class CandidateGenerator {
        public:
            virtual ~CandidateGenerator() {}
            virtual Solution randomCandidate(RNG &randomEngine) = 0;
        };

        template <typename RNG>
        class InsertCandidateGenerator : public CandidateGenerator<RNG> {
        private:
            const Instance &mIns;
            const Solution &mCurrentSolution;

            Solution generateForPositions(int position, int newPosition) {
                vector<const Operation*> ordered(mCurrentSolution.mOrdered);
                auto *pOperation = ordered[position];
                ordered.erase(ordered.begin() + position);
                ordered.insert(ordered.begin() + newPosition, pOperation);

                RobustScheduleFixedOrderDefault alg(mIns);
                auto feasibilityResult = alg.create(ordered);

                double objVal = TotalTardiness().compute(mIns, alg.getStartTimes());
                return Solution(move(ordered), alg.getStartTimes(), feasibilityResult == FeasibilityResult::FEASIBLE, objVal);
            }

        public:

            InsertCandidateGenerator(const Instance &ins, const Solution &currentSolution)
                    : mIns(ins), mCurrentSolution(currentSolution) {}

            virtual Solution randomCandidate(RNG &randomEngine) {
                auto values = GeneralUtils::twoDifferentRandomIntegers(0, mIns.getNumOperations() - 1, randomEngine);
                return generateForPositions(values.first, values.second);
            }
        };

        template <typename RNG>
        class SwapCandidateGenerator : public CandidateGenerator<RNG> {
        private:
            const Instance &mIns;
            const Solution &mCurrentSolution;

            Solution generateForPositions(int position1, int position2) {
                vector<const Operation*> ordered(mCurrentSolution.mOrdered);
                iter_swap(ordered.begin() + position1, ordered.begin() + position2);

                RobustScheduleFixedOrderDefault alg(mIns);
                auto feasibilityResult = alg.create(ordered);

                double objVal = TotalTardiness().compute(mIns, alg.getStartTimes());
                return Solution(move(ordered), alg.getStartTimes(), feasibilityResult == FeasibilityResult::FEASIBLE, objVal);
            }

        public:

            SwapCandidateGenerator(const Instance &ins, const Solution &currentSolution)
                    : mIns(ins), mCurrentSolution(currentSolution) {}

            virtual Solution randomCandidate(RNG &randomEngine) {
                auto values = GeneralUtils::twoDifferentRandomIntegers(0, mIns.getNumOperations() - 1, randomEngine);
                return generateForPositions(values.first, values.second);
            }
        };

    private:
        const Instance &mIns;
        default_random_engine mRandomEngine;

        Solution newRestart(const Solver::Config &cfg,
                            const Config &scfg,
                            const Solution &startSolution,
                            Stopwatch &stopwatch);

        Solution neighbourhoodSearch(const Solver::Config &cfg,
                                     const Config &scfg,
                                     const Solution &currentSolution,
                                     const Solution &bestSolution,
                                     vector<vector<const Operation*>> &tabuList,
                                     Stopwatch &stopwatch);

        bool isTabu(const Solver::Config &cfg,
                    const Config &scfg,
                    const vector<const Operation*> &ordered,
                    const vector<vector<const Operation*>> &tabuList);

    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_TABUSEARCH_H
