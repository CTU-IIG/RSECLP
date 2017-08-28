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

#include <iostream>
#include "TabuSearch.h"
#include "../../utils/Stopwatch.h"
#include "../../feasibility-checkers/FeasibilityChecker.h"
#include "GreedyHeuristics.h"

namespace rseclp {
    const string TabuSearch::Config::KEY_NUM_RESTARTS = "numRestarts";
    const string TabuSearch::Config::KEY_NUM_ITERATIONS = "numIterations";
    const string TabuSearch::Config::KEY_NEIGHBOURHOOD_SIZE = "neighbourhoodSize";
    const string TabuSearch::Config::KEY_TABU_LIST_LENGTH = "tabuListLength";
    const string TabuSearch::Config::KEY_MAX_NONIMPROVING_ITERATIONS = "maxNonimprovingIterations";

    TabuSearch::Config TabuSearch::Config::createFrom(const Solver::Config &cfg) {
        auto &scfg = cfg.getSpecialisedConfig();

        return TabuSearch::Config(scfg.getValue<int>(KEY_SOLVER, KEY_NUM_ITERATIONS, 100),
                                  scfg.getValue<int>(KEY_SOLVER, KEY_NUM_RESTARTS, 5),
                                  scfg.getValue<int>(KEY_SOLVER, KEY_NEIGHBOURHOOD_SIZE, 200),
                                  scfg.getValue<int>(KEY_SOLVER, KEY_TABU_LIST_LENGTH, 5),
                                  scfg.getValue<int>(KEY_SOLVER, KEY_MAX_NONIMPROVING_ITERATIONS, -1));
    }

    TabuSearch::Config::Config(int numIterations,
                               int numRestarts,
                               int neighbourhoodSize,
                               int tabuListLength,
                               int maxNonimprovingIterations)
            : mNumIterations(numIterations),
              mNumRestarts(numRestarts),
              mNeighbourhoodSize(neighbourhoodSize),
              mTabuListLength(tabuListLength), mMaxNonimprovingIterations(maxNonimprovingIterations) {}

    const string TabuSearch::KEY_SOLVER = "TabuSearch";

    TabuSearch::TabuSearch(const Instance &ins) : mIns(ins), mRandomEngine(42) {}

    Solver::Result TabuSearch::solve(const Solver::Config &cfg) {
        Stopwatch stopwatch;
        stopwatch.start();

        auto scfg = TabuSearch::Config::createFrom(cfg);

        TotalTardiness obj;
        Solver::Result result(mIns.getNumOperations(), obj.worstValue());
        if (cfg.getUseInitStartTimes() && FeasibilityChecker(mIns).areFeasible(cfg.getInitStartTimes())) {
            result.setSolution(Solver::Result::Status::FEASIBLE,
                               cfg.getInitStartTimes(),
                               obj.compute(mIns, cfg.getInitStartTimes()));
        }

        for (int restart = 0; restart < scfg.mNumRestarts; restart++) {
            if (stopwatch.timeLimitReached(cfg.getTimeLimit())) {
                break;
            }

            vector<const Operation*> ordered;
            bool isFeasible;
            StartTimes startTimes;
            if (restart == 0 && cfg.getUseInitStartTimes()) {
                startTimes = cfg.getInitStartTimes();
                ordered = startTimes.getOperationsOrdered(mIns);
                isFeasible = FeasibilityChecker(mIns).areFeasible(startTimes);
            }
            else {
                RobustScheduleFixedOrderDefault alg(mIns);
                ordered = GreedyHeuristics(mIns).ruleRandom();
                isFeasible = alg.create(ordered) == FeasibilityResult::FEASIBLE;
                startTimes = alg.getStartTimes();
            }

            double objVal = obj.compute(mIns, startTimes);
            Solution startSolution(move(ordered), move(startTimes), isFeasible, isFeasible ? objVal : obj.worstValue());

            auto restartSolution = newRestart(cfg, scfg, startSolution, stopwatch);
            if (result.getStatus() == Solver::Result::Status::NO_SOLUTION
                || (restartSolution.mIsFeasible && obj.isBetter(restartSolution.mObjectiveValue, result.getObjectiveValue()))) {
                auto status = restartSolution.mIsFeasible ? Solver::Result::Status::FEASIBLE : Solver::Result::Status::NO_SOLUTION;
                result.setSolution(status,
                                   move(restartSolution.mStartTimes),
                                   restartSolution.mObjectiveValue);
//                cout << "New best objective: " << result.getObjectiveValue() << endl;
            }
        }


        stopwatch.stop();
        result.setSolverRuntime(stopwatch.duration());
        return result;
    }

    TabuSearch::Solution TabuSearch::newRestart(const Solver::Config &cfg,
                                                const TabuSearch::Config &scfg,
                                                const TabuSearch::Solution &startSolution,
                                                Stopwatch &stopwatch) {
        vector<vector<const Operation*>> tabuList;
        Solution bestSolution(startSolution.mOrdered,
                              startSolution.mStartTimes,
                              startSolution.mIsFeasible,
                              startSolution.mObjectiveValue);
        Solution currentSolution(startSolution.mOrdered,
                                 startSolution.mStartTimes,
                                 startSolution.mIsFeasible,
                                 startSolution.mObjectiveValue);
        int iteration = 0;
        int numNonimprovingIterations = 0;
        while (iteration < scfg.mNumIterations) {
            if (stopwatch.timeLimitReached(cfg.getTimeLimit())) {
                break;
            }

            auto candidateSolution = neighbourhoodSearch(cfg, scfg, currentSolution, bestSolution, tabuList, stopwatch);
            if (candidateSolution.mIsFeasible && cfg.getObjective()->isBetter(candidateSolution.mObjectiveValue, bestSolution.mObjectiveValue)) {
                bestSolution = candidateSolution;
                numNonimprovingIterations = 0;
            }
            else {
                numNonimprovingIterations++;
            }

            currentSolution = move(candidateSolution);

            tabuList.push_back(currentSolution.mOrdered);

            while ((int)tabuList.size() > scfg.mTabuListLength) {
                tabuList.erase(tabuList.begin());
            }

            iteration++;

            // Stop if the number of nonimproving iterations is larger than allowed.
            if (scfg.mMaxNonimprovingIterations > 0 && numNonimprovingIterations > scfg.mMaxNonimprovingIterations) {
                break;
            }
        }

        return bestSolution;
    }

    TabuSearch::Solution TabuSearch::neighbourhoodSearch(const Solver::Config &cfg,
                                                         const Config &scfg,
                                                         const Solution &currentSolution,
                                                         const Solution &bestSolution,
                                                         vector<vector<const Operation*>> &tabuList,
                                                         Stopwatch &stopwatch) {
        Solution neighbourhoodBestSolution;

        vector<CandidateGenerator<default_random_engine>*> candidateGenerators = {
            new SwapCandidateGenerator<default_random_engine>(mIns, currentSolution),
            new InsertCandidateGenerator<default_random_engine>(mIns, currentSolution)
        };

        uniform_int_distribution<int> candidateGeneratorDist(0, (int)candidateGenerators.size() - 1);
        for (int iter = 0; iter < scfg.mNeighbourhoodSize && !stopwatch.timeLimitReached(cfg.getTimeLimit()); iter++) {
            auto &candidateGenerator = *candidateGenerators[candidateGeneratorDist(mRandomEngine)];
            auto candidateSolution = candidateGenerator.randomCandidate(mRandomEngine);

            if (!candidateSolution.mIsFeasible) {
                continue;
            }

            bool candidateBetterThanBest = candidateSolution.mObjectiveValue < bestSolution.mObjectiveValue;
            bool candidateBetterThanBestNeighbourhood = candidateSolution.mObjectiveValue < neighbourhoodBestSolution.mObjectiveValue;
            if (candidateBetterThanBest && candidateBetterThanBestNeighbourhood) {
                neighbourhoodBestSolution = move(candidateSolution);
            }
            else {
                if (isTabu(cfg, scfg, candidateSolution.mOrdered, tabuList)) {
                    continue;
                }
                else if (candidateBetterThanBestNeighbourhood) {
                    neighbourhoodBestSolution = move(candidateSolution);
                }
            }
        }

        for (auto *pCandidateGenerator : candidateGenerators) {
            delete pCandidateGenerator;
        }

        if (neighbourhoodBestSolution.mOrdered.size() == 0) {
            return currentSolution;
        }
        else {
            return neighbourhoodBestSolution;
        }
    }

    bool TabuSearch::isTabu(const Solver::Config &/*cfg*/,
                            const TabuSearch::Config &/*scfg*/,
                            const vector<const Operation*> &ordered,
                            const vector<vector<const Operation*>> &tabuList) {
        for (auto &tabuOrdered: tabuList) {
            bool areSame = true;
            for (int position = 0; position < mIns.getNumOperations(); position++) {
                if (ordered[position]->getIndex() != tabuOrdered[position]->getIndex()) {
                    areSame = false;
                    break;
                }
            }
            if (areSame) {
                return true;
            }
        }

        return false;
    }


    TabuSearch::Solution::Solution() : mIsFeasible(false), mObjectiveValue(numeric_limits<double>::max()) {}

    TabuSearch::Solution::Solution(vector<const Operation *> ordered,
                                   StartTimes startTimes,
                                   bool isFeasible,
                                   double objectiveValue)
            : mOrdered(move(ordered)),
              mStartTimes(move(startTimes)),
              mIsFeasible(isFeasible),
              mObjectiveValue(objectiveValue) {}
}
