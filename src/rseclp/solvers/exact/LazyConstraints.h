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

#ifndef ROBUSTENERGYAWARESCHEDULING_LAZYCONSTRAINTS_H
#define ROBUSTENERGYAWARESCHEDULING_LAZYCONSTRAINTS_H

#include <gurobi_c++.h>
#include <cassert>
#include "../Solver.h"
#include "../../utils/MultiArray.h"

namespace rseclp {

    class LazyConstraints : public Solver {

    public:
        static const string KEY_SOLVER;

        LazyConstraints(const Instance &ins, GRBEnv &env);

        class Config {
        public:
            static const string KEY_GENERATE_CUTTING_CONSTRAINTS_ONE_SOLUTION;
            static const string KEY_GENERATE_CUTTING_CONSTRAINTS_TOWARDS_OPTIMAL;
            static const string KEY_GENERATE_CUTTING_CONSTRAINTS_ETFA2016;
            static const string KEY_NO_ENERGY_CONSUMPTION_LIMITS;
            static const string KEY_NO_DEVIATIONS;

            const bool mGenerateCuttingConstraintsOneSolution;
            const bool mGenerateCuttingConstraintsTowardsOptimal;
            const bool mGenerateCuttingConstraintsETFA2016;
            const bool mNoEnergyConsumptionLimits;
            const bool mNoDeviations;

            const bool mNoCallback;

            Config(bool generateCuttingConstraintsOneSolution,
                   bool generateCuttingConstraintsTowardsOptimal,
                   bool generateCuttingConstraintsETFA2016,
                   bool noEnergyConsumptionLimits,
                   bool noDeviations);

            static Config createFrom(const Solver::Config &cfg);
        };

        virtual Solver::Result solve(const Solver::Config &cfg);

    private:
        const Instance &mIns;
        GRBEnv &mEnv;
        unique_ptr<GRBModel> mMasterModel;

        class Vars {
        private:
            const LazyConstraints &mSuper;
            int mMinReleaseTime;

        public:
            GRBVar z;
            MultiArray<GRBVar> bs;
            MultiArray<GRBVar> eTime;

            Vars(const LazyConstraints &super);

            int getStartTimeMin(const Operation &operation) const;

            int getStartTimeMax(const Operation &operation) const;

            int getAllocMin() const;

            int getAllocMax() const;

        } mMasterVars;

        class Callback : public GRBCallback {
        public:
            LazyConstraints &mSuper;
            LazyConstraints::Config mScfg;

            int mNumGeneratedLazyConstraints = 0;

            Callback(LazyConstraints &super, LazyConstraints::Config scfg);

        protected:
            virtual void callback();

        private:
            StartTimes getStartTimes();

            void generateCuttingConstraintsOneSolution(const StartTimes &startTimes);

            void generateCuttingConstraintsTowardsOptimal(const StartTimes &startTimes);

            void generateCuttingConstraintsETFA2016(const StartTimes &startTimes);

            void generateCuttingConstraints(const vector<const Operation*> ordered,
                                            const vector<Interval<int>> cuttingIntervals);

            void generateCuttingConstraints(const vector<Interval<int>> cuttingIntervals);

            void generateCuttingConstraint(const Operation &operation,
                                           const Interval<int> &cuttingInterval,
                                           GRBLinExpr &expr);
        };

        friend class Callback;

        StartTimes getStartTimesFromMasterSolution() const;

        void setInitialSolution(const Solver::Config &cfg);

        void createVariablesMasterModel(const Solver::Config &cfg, const LazyConstraints::Config &scfg);

        void createConstraintsMasterModel(const Solver::Config &cfg, const LazyConstraints::Config &scfg);

        void createObjectiveMasterModel(const Solver::Config &cfg, const LazyConstraints::Config &scfg);
    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_LAZYCONSTRAINTS_H
