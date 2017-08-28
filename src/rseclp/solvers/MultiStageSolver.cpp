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

#include "MultiStageSolver.h"
#include "../utils/Stopwatch.h"
#include "../feasibility-checkers/FeasibilityChecker.h"

namespace rseclp {

    MultiStageSolver::MultiStageSolver(const Instance &ins, GRBEnv &env) : mIns(ins), mEnv(env) {}

    MultiStageSolver::~MultiStageSolver() {
        for (auto solver: mSolvers) {
            delete solver;
        }

        mSolvers.clear();
    }

    void MultiStageSolver::mergeOptionalResults(Result &source, Result &target) {
        for (auto &item : source.getOptional()) {
            target.setOptional(item.first, item.second);
        }
    }

    void MultiStageSolver::addSolver(Solver *solver) {
        mSolvers.push_back(solver);
    }

    const Solver::Result &MultiStageSolver::getStageResult(int stage) {
        return mSolverResults[stage];
    }

    const vector<Solver::Result> &MultiStageSolver::getStageResults() {
        return mSolverResults;
    }

    Solver::Result MultiStageSolver::solve(const Solver::Config &cfg) {
        Stopwatch allStagesStopwatch;
        allStagesStopwatch.start();

        mSolverResults.clear();

        Solver::Result currentResult(mIns.getNumOperations(), cfg.getObjective()->worstValue());
        if (cfg.getUseInitStartTimes()) {
            auto &initStartTimes = cfg.getInitStartTimes();
            if (FeasibilityChecker(mIns).areFeasible(initStartTimes)) {
                currentResult.setSolution(Solver::Result::Status::FEASIBLE,
                                          initStartTimes,
                                          cfg.getObjective()->compute(mIns, initStartTimes));
            }
        }

        chrono::milliseconds remainingTime = cfg.getTimeLimit();

        int currentStage = 0;
        bool stop = false;
        for (auto solver: mSolvers) {
            Solver::Config newCfg(remainingTime,
                                  cfg.getObjective(),
                                  currentResult.getStatus() == Solver::Result::Status::FEASIBLE,
                                  currentResult.getStartTimes(),
                                  cfg.getSpecialisedConfig());

            Stopwatch stageStopwatch;
            stageStopwatch.start();
            auto result = solver->solve(newCfg);
            stageStopwatch.stop();

            remainingTime -= stageStopwatch.duration();

            MultiStageSolver::mergeOptionalResults(result, currentResult);
            switch (result.getStatus()) {
                case Solver::Result::Status::FEASIBLE:
                    cout << "Stage " << currentStage << ": " << result.getObjectiveValue() << " (feasible solution)" << endl;
                    currentResult.setSolution(Solver::Result::Status::FEASIBLE,
                                              result.getStartTimes(),
                                              result.getObjectiveValue());
                    break;

                case Solver::Result::Status::INFEASIBLE:
                    cout << "Stage " << currentStage << ": NA (infeasible model)" << endl;
                    currentResult.setStatus(Solver::Result::Status::INFEASIBLE);
                    stop = true;
                    break;

                case Solver::Result::Status::NO_SOLUTION:
                    cout << "Stage " << currentStage << ": NA (no solution)" << endl;
                    break;

                case Solver::Result::Status::OPTIMAL:
                    cout << "Stage " << currentStage << ": " << result.getObjectiveValue() << " (optimal solution)" << endl;
                    currentResult.setSolution(Solver::Result::Status::OPTIMAL,
                                              result.getStartTimes(),
                                              result.getObjectiveValue());
                    stop = true;
                    break;
            }

            mSolverResults.push_back(move(result));

            if (remainingTime.count() < 0) {
                cout << "Time out" << endl;
                stop = true;
            }

            if (stop) {
                break;
            }

            currentStage++;
        }

        allStagesStopwatch.stop();
        currentResult.setSolverRuntime(allStagesStopwatch.duration());

        return currentResult;
    }
}
