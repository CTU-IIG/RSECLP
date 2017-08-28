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

#include "LazyConstraints.h"
#include "../../utils/IlpUtils.h"
#include "../../utils/GeneralUtils.h"
#include "../../utils/Stopwatch.h"
#include "../../feasibility-checkers/FeasibilityChecker.h"
#include "../fixed-order/RobustScheduleFixedOrderDefault.h"
#include "../../objectives/TotalTardiness.h"

namespace rseclp {
    const string LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_ONE_SOLUTION = "generateCuttingConstraintsOneSolution";
    const string LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_TOWARDS_OPTIMAL = "generateCuttingConstraintsTowardsOptimal";
    const string LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_ETFA2016 = "generateCuttingConstraintsETFA2016";
    const string LazyConstraints::Config::KEY_NO_ENERGY_CONSUMPTION_LIMITS = "noEnergyConsumptionLimits";
    const string LazyConstraints::Config::KEY_NO_DEVIATIONS = "noDeviations";

    LazyConstraints::Config::Config(bool generateCuttingConstraintsOneSolution,
                                    bool generateCuttingConstraintsTowardsOptimal,
                                    bool generateCuttingConstraintsETFA2016,
                                    bool noEnergyConsumptionLimits,
                                    bool noDeviations)
            : mGenerateCuttingConstraintsOneSolution(generateCuttingConstraintsOneSolution),
              mGenerateCuttingConstraintsTowardsOptimal(generateCuttingConstraintsTowardsOptimal),
              mGenerateCuttingConstraintsETFA2016(generateCuttingConstraintsETFA2016),
              mNoEnergyConsumptionLimits(noEnergyConsumptionLimits),
              mNoDeviations(noDeviations || noEnergyConsumptionLimits),
              mNoCallback(mNoDeviations) {}

    LazyConstraints::Config LazyConstraints::Config::createFrom(const Solver::Config &cfg) {
        auto &scfg = cfg.getSpecialisedConfig();

        return LazyConstraints::Config(scfg.getValue<bool>(KEY_SOLVER,
                                                           KEY_GENERATE_CUTTING_CONSTRAINTS_ONE_SOLUTION,
                                                           false),
                                       scfg.getValue<bool>(KEY_SOLVER,
                                                           KEY_GENERATE_CUTTING_CONSTRAINTS_TOWARDS_OPTIMAL,
                                                           true),
                                       scfg.getValue<bool>(KEY_SOLVER,
                                                           KEY_GENERATE_CUTTING_CONSTRAINTS_ETFA2016,
                                                           false),
                                       scfg.getValue<bool>(KEY_SOLVER,
                                                           KEY_NO_ENERGY_CONSUMPTION_LIMITS,
                                                           false),
                                       scfg.getValue<bool>(KEY_SOLVER,
                                                           KEY_NO_DEVIATIONS,
                                                           false));
    }


    LazyConstraints::Vars::Vars(const LazyConstraints &super) : mSuper(super) {
        mMinReleaseTime = (*min_element(super.mIns.getOperations().begin(),
                                        super.mIns.getOperations().end(),
                                        [](const Operation *lhs, const Operation *rhs) {
                                            return lhs->getReleaseTime() < rhs->getReleaseTime();
                                        }))->getReleaseTime();
    }

    int LazyConstraints::Vars::getStartTimeMin(const Operation &operation) const {
        return operation.getReleaseTime();
    }

    int LazyConstraints::Vars::getStartTimeMax(const Operation &/*operation*/) const {
        return mSuper.mIns.getMaximumStartTime();
    }

    int LazyConstraints::Vars::getAllocMin() const {
        return mMinReleaseTime;
    }

    int LazyConstraints::Vars::getAllocMax() const {
        return mSuper.mIns.getHorizon() - 1;
    }

    LazyConstraints::Callback::Callback(LazyConstraints &super, LazyConstraints::Config scfg)
            : mSuper(super), mScfg(scfg) {}

    void LazyConstraints::Callback::callback() {
        if (where == GRB_CB_MIPSOL) {
            const auto startTimes = getStartTimes();

            if (mScfg.mGenerateCuttingConstraintsOneSolution) {
                generateCuttingConstraintsOneSolution(startTimes);
            }

            if (mScfg.mGenerateCuttingConstraintsTowardsOptimal) {
                generateCuttingConstraintsTowardsOptimal(startTimes);
            }

            if (mScfg.mGenerateCuttingConstraintsETFA2016) {
                generateCuttingConstraintsETFA2016(startTimes);
            }
        }
    }


    void LazyConstraints::Callback::generateCuttingConstraintsOneSolution(const StartTimes &startTimes) {
        FeasibilityChecker checker(mSuper.mIns);
        if (checker.areFeasible(startTimes)) {
            return;
        }

        vector<Interval<int>> cuttingIntervals;
        for (auto pOperation : mSuper.mIns.getOperations()) {
            const Operation &operation = *pOperation;
            int startTimeToForbid = startTimes[operation];
            cuttingIntervals.push_back(Interval<int>(startTimeToForbid, startTimeToForbid + 1));
        }

        generateCuttingConstraints(cuttingIntervals);
    }

    void LazyConstraints::Callback::generateCuttingConstraintsTowardsOptimal(const StartTimes &startTimes) {
        auto ordered = startTimes.getOperationsOrdered(mSuper.mIns);
        RobustScheduleFixedOrderDefault alg(mSuper.mIns);
        FeasibilityResult feasibilityResult = alg.create(ordered);
        const StartTimes &robustStartTimes = alg.getStartTimes();

        vector<Interval<int>> cuttingIntervals;
        if (feasibilityResult == FeasibilityResult::INFEASIBLE) {
            // Infeasible order, forbid it.
            for (auto itOperation = ordered.begin(); itOperation != ordered.end(); itOperation++) {
                const Operation &operation = **itOperation;

                auto itNextOperation = next(itOperation, 1);
                if (itNextOperation != ordered.end()) {
                    const Operation &nextOperation = **itNextOperation;
                    cuttingIntervals.push_back(Interval<int>(startTimes[operation],
                                                             min(startTimes[nextOperation] + nextOperation.getProcessingTime() - 1,
                                                                 mSuper.mIns.getMaximumStartTime())
                                                             + 1));
                }
                else {
                    cuttingIntervals.push_back(Interval<int>(startTimes[operation],
                                                             mSuper.mIns.getMaximumStartTime() + 1));
                }
            }
        }
        else {
            assert(FeasibilityChecker(mSuper.mIns).areFeasible(robustStartTimes));

            // Find the first operation in ordered which starts in current solution at different time than in robust
            // start times.
            int positionDiff = 0;
            for (auto *pOperation : ordered) {
                const Operation &operation = *pOperation;
                if (startTimes[operation] != robustStartTimes[operation]) {
                    break;
                }
                else {
                    positionDiff++;
                }
            }

            if (positionDiff >= mSuper.mIns.getNumOperations()) {
                // Start times are robust.
                return;
            }

            const Operation &operationDiff = *ordered[positionDiff];
            auto itOperationDiff = next(ordered.begin(), positionDiff);
            if (startTimes[operationDiff] < robustStartTimes[operationDiff]) {
                // Operations in ordered up to operationDiff (exclusive)
                for (auto itOperation = ordered.begin(); itOperation != itOperationDiff; itOperation++) {
                    const Operation &operation = **itOperation;
                    const Operation &nextOperation = **next(itOperation, 1);
                    cuttingIntervals.push_back(Interval<int>(startTimes[operation],
                                                             min(startTimes[nextOperation] + nextOperation.getProcessingTime() - 1,
                                                                 mSuper.mIns.getMaximumStartTime())
                                                             + 1));
                }

                if (next(itOperationDiff, 1) == ordered.end()) {
                    // operationDiff
                    cuttingIntervals.push_back(Interval<int>(startTimes[operationDiff],
                                                             robustStartTimes[operationDiff] - 1 + 1));
                }
                else {
                    const Operation &nextOperationDiff = **next(itOperationDiff, 1);
                    int completionTimeNextOperationDiff = startTimes[nextOperationDiff] + nextOperationDiff.getProcessingTime();

                    // operationDiff
                    cuttingIntervals.push_back(Interval<int>(startTimes[operationDiff],
                                                             min(robustStartTimes[operationDiff],
                                                                 completionTimeNextOperationDiff)
                                                             - 1 + 1));

                    // Operations after operationDiff
                    for (auto itOperation = next(itOperationDiff, 1); itOperation != ordered.end(); itOperation++) {
                        const Operation &operation = **itOperation;
                        cuttingIntervals.push_back(Interval<int>(min(robustStartTimes[operationDiff],
                                                                     completionTimeNextOperationDiff - operation.getProcessingTime()),
                                                                 mSuper.mIns.getMaximumStartTime() + 1));
                    }
                }
            }
            else {
                // Operations before operationDiff (exclusive)
                for (auto itOperation = ordered.begin(); itOperation != itOperationDiff; itOperation++) {
                    const Operation &operation = **itOperation;
                    const Operation &nextOperation = **next(itOperation, 1);
                    cuttingIntervals.push_back(Interval<int>(robustStartTimes[operation],
                                                             min(robustStartTimes[nextOperation] + nextOperation.getProcessingTime() - 1,
                                                                 mSuper.mIns.getMaximumStartTime())
                                                             + 1));
                }

                // operationDiff
                cuttingIntervals.push_back(Interval<int>(robustStartTimes[operationDiff] + 1,
                                                         startTimes[operationDiff] + 1));

                // Operations after operationDiff
                for (auto itOperation = next(itOperationDiff, 1); itOperation != ordered.end(); itOperation++) {
                    const Operation &operation = **itOperation;
                    cuttingIntervals.push_back(Interval<int>(startTimes[operationDiff] - operation.getProcessingTime() + 1,
                                                             mSuper.mIns.getMaximumStartTime() + 1));
                }
            }
        }

        if (cuttingIntervals.size() > 0) {
            generateCuttingConstraints(ordered, cuttingIntervals);
        }
    }

    void LazyConstraints::Callback::generateCuttingConstraintsETFA2016(const StartTimes &startTimes) {
        FeasibilityChecker feasibilityChecker(mSuper.mIns);
        if (feasibilityChecker.areFeasible(startTimes)) {
            return;
        }

        auto ordered = startTimes.getOperationsOrdered(mSuper.mIns);
        StartTimes realisedStartTimes(mSuper.mIns.getNumOperations());
        startTimes.computeRealisedStartTimes(mSuper.mIns, ordered, feasibilityChecker.mUncertaintyScenario, realisedStartTimes);

        // First and last intersecting operations in the realised schedule.
        int firstIntersectingPosition = 0;
        for (auto *pOperation : ordered) {
            const Operation &operation = *pOperation;
            auto startTime = realisedStartTimes[operation];
            auto completionTime = startTime + operation.getProcessingTime();
            int intersectionLength = GeneralUtils::computeIntervalsIntersectionLength(feasibilityChecker.mViolatedMeteringInterval->getStart(),
                                                                                      feasibilityChecker.mViolatedMeteringInterval->getEnd(),
                                                                                      startTime,
                                                                                      completionTime);
            if (intersectionLength > 0) {
                break;
            }
            else {
                firstIntersectingPosition++;
            }
        }

        int lastIntersectingPosition = firstIntersectingPosition;
        for (auto itOperation = next(ordered.begin(), firstIntersectingPosition); itOperation != ordered.end(); itOperation++) {
            const Operation &operation = **itOperation;
            auto startTime = realisedStartTimes[operation];
            auto completionTime = startTime + operation.getProcessingTime();
            int intersectionLength = GeneralUtils::computeIntervalsIntersectionLength(feasibilityChecker.mViolatedMeteringInterval->getStart(),
                                                                                      feasibilityChecker.mViolatedMeteringInterval->getEnd(),
                                                                                      startTime,
                                                                                      completionTime);
            if (intersectionLength == 0) {
                break;
            }
            else {
                lastIntersectingPosition++;
            }
        }
        lastIntersectingPosition -= 1;
        const Operation &lastIntersectingOperation = *ordered[lastIntersectingPosition];

        int headPosition = firstIntersectingPosition;
        while (headPosition > 0) {
            const Operation &operation = *ordered[headPosition];
            const Operation &prevOperation = *ordered[headPosition - 1];

            auto startTime = startTimes[operation];
            auto prevLatestStartTime = startTimes[prevOperation];
            if ((prevLatestStartTime + prevOperation.getProcessingTime()) <= startTime) {
                break;
            }
            else {
                headPosition--;
            }
        }
        const Operation &headOperation = *ordered[headPosition];

        int tBoundary = (lastIntersectingPosition + 1) < mSuper.mIns.getNumOperations() ?
                    min(realisedStartTimes[lastIntersectingOperation], startTimes[*ordered[lastIntersectingPosition + 1]]) :
                    realisedStartTimes[lastIntersectingOperation];


        // TODO (refactoring): current implementation of cutting constraints is not able to handle union of intervals.
        GRBLinExpr expr = 0;

        // Operations before head and after lastIntersecting.
        {
            int position = 0;
            while (position < mSuper.mIns.getNumOperations()) {
                if (position == headPosition) {
                    position = lastIntersectingPosition + 1;
                    continue;
                }
                const Operation &operation = *ordered[position];
                generateCuttingConstraint(operation, Interval<int>(0, startTimes[headOperation] + 1), expr);
                generateCuttingConstraint(operation, Interval<int>(tBoundary, mSuper.mIns.getMaximumStartTime() + 1), expr);
                position++;
            }
        }

        // Operations [head, ..., lastIntersecting - 1].
        for (int position = headPosition; position < lastIntersectingPosition; position++) {
            const Operation &operation = *ordered[position];
            const Operation &nextOperation = *ordered[position + 1];
            generateCuttingConstraint(operation, Interval<int>(startTimes[operation], startTimes[nextOperation] + nextOperation.getProcessingTime() + 1 - 1), expr);
        }

        // Operation lastIntersecting.
        generateCuttingConstraint(lastIntersectingOperation, Interval<int>(startTimes[lastIntersectingOperation], tBoundary + 1), expr);

        addLazy(expr <= mSuper.mIns.getNumOperations() - 1);
        mNumGeneratedLazyConstraints++;
    }

    void LazyConstraints::Callback::generateCuttingConstraints(const vector<const Operation*> ordered,
                                                               const vector<Interval<int>> cuttingIntervals) {
        GRBLinExpr expr = 0;
        for (int position = 0; position < mSuper.mIns.getNumOperations(); position++) {
            generateCuttingConstraint(*ordered[position], cuttingIntervals[position], expr);
        }

        addLazy(expr <= mSuper.mIns.getNumOperations() - 1);
        mNumGeneratedLazyConstraints++;
    }

    void LazyConstraints::Callback::generateCuttingConstraints(const vector<Interval<int>> cuttingIntervals) {
        GRBLinExpr expr = 0;
        for (int operationIndex = 0; operationIndex < mSuper.mIns.getNumOperations(); operationIndex++) {
            generateCuttingConstraint(*mSuper.mIns.getOperation(operationIndex),
                                      cuttingIntervals[operationIndex],
                                      expr);
        }

        addLazy(expr <= mSuper.mIns.getNumOperations() - 1);
        mNumGeneratedLazyConstraints++;
    }

    void LazyConstraints::Callback::generateCuttingConstraint(const Operation &operation,
                                                              const Interval<int> &cuttingInterval,
                                                              GRBLinExpr &expr) {
        const int tMin = max(cuttingInterval.getStart(), mSuper.mMasterVars.getStartTimeMin(operation));
        const int tMax = min(cuttingInterval.getEnd() - 1, mSuper.mMasterVars.getStartTimeMax(operation));
        for (int t = tMin; t <= tMax; t++) {
            expr += mSuper.mMasterVars.bs(operation.getIndex(), t);
        }
    }

    StartTimes LazyConstraints::Callback::getStartTimes() {
        StartTimes startTimes(mSuper.mIns.getNumOperations());

        for (const Operation *pOperation : mSuper.mIns.getOperations()) {
            const Operation &operation = *pOperation;

            int tMin = mSuper.mMasterVars.getStartTimeMin(operation);
            int tMax = mSuper.mMasterVars.getStartTimeMax(operation);
            for (int t = tMin; t <= tMax; t++) {
                if (getSolution(mSuper.mMasterVars.bs(operation.getIndex(), t)) >= 0.5) {
                    startTimes[operation] = t;
                    break;
                }
            }
        }

        return startTimes;
    }

    const string LazyConstraints::KEY_SOLVER = "LazyConstraints";

    LazyConstraints::LazyConstraints(const Instance &ins, GRBEnv &env)
            : mIns(ins),
              mEnv(env),
              mMasterVars(*this) { }

    Solver::Result LazyConstraints::solve(const Solver::Config &cfg) {
        Stopwatch stopwatch;
        stopwatch.start();

        auto scfg = LazyConstraints::Config::createFrom(cfg);

        auto envLazyConstraintsBckp = mEnv.get(GRB_IntParam_LazyConstraints);
        if (scfg.mNoCallback == false) {
            mEnv.set(GRB_IntParam_LazyConstraints, 1);
        }

        mMasterModel.reset(new GRBModel(mEnv));

        createVariablesMasterModel(cfg, scfg);
        mMasterModel->update();
        setInitialSolution(cfg);
        createConstraintsMasterModel(cfg, scfg);
        mMasterModel->update();
        createObjectiveMasterModel(cfg, scfg);
        mMasterModel->update();

        Solver::Result result(mIns.getNumOperations(), cfg.getObjective()->worstValue());

        auto callback = unique_ptr<LazyConstraints::Callback>(nullptr);
        if (scfg.mNoCallback == false) {
            callback.reset(new LazyConstraints::Callback(*this, scfg));
            mMasterModel->setCallback(callback.get());
        }

        double timeLimitInSeconds = (double)chrono::duration_cast<chrono::seconds>(cfg.getTimeLimit() - stopwatch.duration()).count();
        mMasterModel->getEnv().set(GRB_DoubleParam_TimeLimit, timeLimitInSeconds);
        mMasterModel->update();

        mMasterModel->optimize();

        if (mMasterModel->get(GRB_IntAttr_Status) == GRB_INFEASIBLE) {
            result.setStatus(Solver::Result::Status::INFEASIBLE);
        }
        else if (mMasterModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL || mMasterModel->get(GRB_IntAttr_Status) == GRB_TIME_LIMIT) {
            result.setSolution(mMasterModel->get(GRB_IntAttr_Status) == GRB_OPTIMAL ? Solver::Result::Status::OPTIMAL : Solver::Result::Status::FEASIBLE,
                               getStartTimesFromMasterSolution(),
                               mMasterModel->getObjective().getValue());
        }

        if (scfg.mNoCallback == false) {
            result.setOptional("numGeneratedLazyConstraints", to_string(callback->mNumGeneratedLazyConstraints));
            result.setOptional("lowerBound", to_string(mMasterModel->get(GRB_DoubleAttr_ObjBoundC)));
        }


        if (scfg.mNoCallback == false) {
            mEnv.set(GRB_IntParam_LazyConstraints, envLazyConstraintsBckp);
        }

        stopwatch.stop();
        result.setSolverRuntime(stopwatch.duration());
        return result;
    }

    void LazyConstraints::setInitialSolution(const Solver::Config &cfg) {
        if (cfg.getUseInitStartTimes()) {
            auto &initStartTimes = cfg.getInitStartTimes();
            for (const Operation *pOperation : mIns.getOperations()) {
                const Operation &operation = *pOperation;
                mMasterVars.bs(operation.getIndex(), initStartTimes[operation]).set(GRB_DoubleAttr_Start, 1);
            }
        }
    }

    void LazyConstraints::createVariablesMasterModel(const Solver::Config &/*cfg*/, const LazyConstraints::Config &scfg) {
        mMasterVars.bs = MultiArray<GRBVar>({mIns.getNumOperations(), mIns.getMaximumStartTime() + 1}, GRBVar());
        mMasterVars.eTime = MultiArray<GRBVar>({mIns.getHorizon()}, GRBVar());
        mMasterVars.z = mMasterModel->addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, IlpUtils::varName("z", {}));

        for (const Operation *pOperation : mIns.getOperations()) {
            const Operation &operation = *pOperation;

            int tMin = mMasterVars.getStartTimeMin(operation);
            int tMax = mMasterVars.getStartTimeMax(operation);
            for (int t = tMin; t <= tMax; t++) {
                mMasterVars.bs(operation.getIndex(), t) = mMasterModel->addVar(0, 1, 0, GRB_BINARY, IlpUtils::varName("bs", {operation.getIndex(), t}));
            }
        }

        if (scfg.mNoEnergyConsumptionLimits == false) {
            int tMin = mMasterVars.getAllocMin();
            int tMax = mMasterVars.getAllocMax();
            for (int t = tMin; t <= tMax; t++) {
                mMasterVars.eTime(t) = mMasterModel->addVar(0, GRB_INFINITY, 0, GRB_CONTINUOUS, IlpUtils::varName("e", {t}));
            }
        }
    }

    void LazyConstraints::createConstraintsMasterModel(const Solver::Config &cfg, const LazyConstraints::Config &scfg) {

        // Each operation starts at some time.
        for (const Operation *pOperation : mIns.getOperations()) {
            const Operation &operation = *pOperation;

            GRBLinExpr expr = 0;
            int tMin = mMasterVars.getStartTimeMin(operation);
            int tMax = mMasterVars.getStartTimeMax(operation);
            for (int t = tMin; t <= tMax; t++) {
                expr += mMasterVars.bs(operation.getIndex(), t);
            }
            mMasterModel->addConstr(expr == 1);
        }

        // No overlapping of operations.
        // Modeled as SOS of type 1.
        {
            int tMin = mMasterVars.getAllocMin();
            int tMax = mMasterVars.getAllocMax();
            for (int t = tMin; t <= tMax; t++) {
                vector<GRBVar> vars;
                vector<double> weights;

                for (const Operation *pOperation : mIns.getOperations()) {
                    const Operation &operation = *pOperation;

                    int tpMin = max(mMasterVars.getStartTimeMin(operation), t - operation.getProcessingTime() + 1);
                    int tpMax = min(t, mMasterVars.getStartTimeMax(operation));
                    for (int tp = tpMin; tp <= tpMax; tp++) {
                        vars.push_back(mMasterVars.bs(operation.getIndex(), tp));
                        weights.push_back(1.0);
                    }
                }

                if (vars.size() != 0) {
                    mMasterModel->addSOS(&vars[0], &weights[0], (int)vars.size(), GRB_SOS_TYPE1);
                }
            }
        }

        if (scfg.mNoEnergyConsumptionLimits == false) {
            // Energy in time t.
            {
                int tMin = mMasterVars.getAllocMin();
                int tMax = mMasterVars.getAllocMax();
                for (int t = tMin; t <= tMax; t++) {
                    GRBLinExpr expr = 0;

                    for (const Operation *pOperation : mIns.getOperations()) {
                        const Operation &operation = *pOperation;

                        int tpMin = max(mMasterVars.getStartTimeMin(operation), t - operation.getProcessingTime() + 1);
                        int tpMax = min(t, mMasterVars.getStartTimeMax(operation));
                        for (int tp = tpMin; tp <= tpMax; tp++) {
                            expr += mMasterVars.bs(operation.getIndex(), tp) * operation.getPowerConsumption();
                        }

                    }

                    mMasterModel->addConstr(expr == mMasterVars.eTime(t));
                }
            }

            // Maximum energy in metering interval.
            for (const MeteringInterval *pMeteringInterval : mIns.getMeteringIntervals()) {
                const MeteringInterval &meteringInterval = *pMeteringInterval;

                GRBLinExpr expr = 0;

                int tMin = max(meteringInterval.getStart(), mMasterVars.getAllocMin());
                int tMax = min(meteringInterval.getEnd() - 1, mMasterVars.getAllocMax());
                for (int t = tMin; t <= tMax; t++) {
                    expr += mMasterVars.eTime(t);
                }

                mMasterModel->addConstr(expr <= meteringInterval.getMaxEnergyConsumption());
            }
        }

        // Maximum energy in metering interval - single deviation.
        if (scfg.mNoDeviations == false) {
            for (const MeteringInterval *pMeteringInterval : mIns.getMeteringIntervals()) {
                const MeteringInterval &meteringInterval = *pMeteringInterval;

                for (int deviation = 1; deviation <= mIns.getMaxDeviation(); deviation++) {
                    GRBLinExpr expr = 0;
                    for (int tRel = 0; tRel < mIns.getLengthMeteringInterval(); tRel++) {
                        int t = meteringInterval.getStart() - deviation + tRel;
                        if (mMasterVars.getAllocMin() <= t && t <= mMasterVars.getAllocMax()) {
                            expr += mMasterVars.eTime(t);
                        }
                    }

                    mMasterModel->addConstr(expr <= meteringInterval.getMaxEnergyConsumption());
                }
            }
        }

        switch (cfg.getObjective()->getType()) {
            case Objective::Type::TOTAL_TARDINESS: {
                GRBLinExpr expr = 0;
                for (const Operation *pOperation : mIns.getOperations()) {
                    const Operation &operation = *pOperation;

                    int tMin = mMasterVars.getStartTimeMin(operation);
                    int tMax = mMasterVars.getStartTimeMax(operation);
                    for (int t = tMin; t <= tMax; t++) {
                        expr += max(0,
                                    (t + operation.getProcessingTime() - operation.getDueDate())) * mMasterVars.bs(operation.getIndex(), t);
                    }
                }

                mMasterModel->addConstr(mMasterVars.z == expr);
            }
            break;

            default:
                assert(false);
                break;
        }
    }

    void LazyConstraints::createObjectiveMasterModel(const Solver::Config &cfg, const LazyConstraints::Config &/*scfg*/) {
        switch (cfg.getObjective()->getType()) {
            case Objective::Type::TOTAL_TARDINESS:
            {
                mMasterModel->setObjective(mMasterVars.z + 0, GRB_MINIMIZE);
            }
            break;

            default:
                assert(false);
                break;
        }
    }

    StartTimes LazyConstraints::getStartTimesFromMasterSolution() const {
        StartTimes startTimes(mIns.getNumOperations());
        MultiArray<int> startTimesBinary = IlpUtils::binaryVariablesToValues(mMasterVars.bs);

        for (const Operation *pOperation : mIns.getOperations()) {
            const Operation &operation = *pOperation;

            int tMin = mMasterVars.getStartTimeMin(operation);
            int tMax = mMasterVars.getStartTimeMax(operation);
            for (int t = tMin; t <= tMax; t++) {
                if (startTimesBinary(operation.getIndex(), t) == 1) {
                    startTimes[operation] = t;
                    break;
                }
            }
        }

        return startTimes;
    }

}
