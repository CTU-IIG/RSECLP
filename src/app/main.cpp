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

#include <vector>
#include <boost/filesystem.hpp>
#include <iostream>
#include "../rseclp/utils/JsonUtils.h"
#include "../rseclp/instance/InstanceJsonReader.h"
#include "../rseclp/utils/StartTimes.h"
#include "../rseclp/solvers/fixed-order/RobustScheduleFixedOrder.h"
#include "../rseclp/solvers/fixed-order/RobustScheduleFixedOrder2016B.h"
#include "../rseclp/solvers/heuristics/GreedyHeuristics.h"
#include "../rseclp/utils/Stopwatch.h"
#include "../rseclp/solvers/exact/BruteForceSearch.h"
#include "../rseclp/objectives/TotalTardiness.h"
#include "../rseclp/solvers/exact/BranchAndBoundOnOrder.h"
#include "../rseclp/solvers/exact/LazyConstraints.h"
#include "../rseclp/feasibility-checkers/FeasibilityChecker.h"
#include "../rseclp/instance/InstanceJsonWriter.h"
#include "../rseclp/solvers/MultiStageSolver.h"
#include "../rseclp/solvers/heuristics/TabuSearch.h"

using namespace std;
using namespace rseclp;

int main() {
    auto env = GRBEnv();
    env.set(GRB_IntParam_OutputFlag, 1);

    boost::filesystem::path p = "./example-instances/5/example.json";
//    boost::filesystem::path p = "/home/modosist/workspace/phd/robust-energy-aware-scheduling/experiment-data/datasets/n=5/8/instances/4.json";
    auto ins = rseclp::InstanceJsonReader::read(p);

//    if (ins->getMetadata().find("noDeviationsObjectiveValue") != ins->getMetadata().end()) {
//        cout << "No deviations objective value: " << ins->getMetadata().find("noDeviationsObjectiveValue")->second << endl;
//    }

    Stopwatch sw;
    sw.start();
    
//     StartTimes startTimes({0, 6, 9, 16, 20});
//     
//      cout << "Solution feasible? " << FeasibilityChecker(*ins).areFeasible(startTimes) << endl;

//    for (int i = 0; i < 5000000; i++) {
//        auto ordered = GreedyHeuristics(*ins).ruleRandom();

//        RobustScheduleFixedOrderOptimised alg1(*ins);
//        RobustScheduleFixedOrder2016B alg2(*ins);

//        auto alg1Result = alg1.create(ordered);
//        auto alg2Result = alg2.create(ordered);

//        if(alg1Result != alg2Result) {
//            cout << "error same result" << endl;
//            return 1;
//        }

//        if (alg2Result == FeasibilityResult::FEASIBLE) {
////            cout << alg1.getStartTimes() << endl;
////            cout << alg2.getStartTimes() << endl;
//            for (int operationIndex = 0; operationIndex < ins->getNumOperations(); operationIndex++) {
//                const Operation &operation = *ins->getOperation(operationIndex);
//                if (alg1.getStartTimes()[operation] != alg2.getStartTimes()[operation]) {
//                    cout << "error times" << endl;
//                    return 1;
//                }
//            }
//        }
//    }


    {
        MultiStageSolver solver(*ins, env);
        Solver::SpecialisedConfig scfg;

        // Greedy stage.
        scfg.addValue(GreedyHeuristics::KEY_SOLVER,
                      GreedyHeuristics::Config::KEY_RULE,
                      GreedyHeuristics::Config::RULE_TARDINESS);
        solver.addSolver(new GreedyHeuristics(*ins));

        // Tabu search stage.
        scfg.addValue(TabuSearch::KEY_SOLVER, TabuSearch::Config::KEY_NUM_RESTARTS, 10);
        scfg.addValue(TabuSearch::KEY_SOLVER, TabuSearch::Config::KEY_NUM_ITERATIONS, 200);
        scfg.addValue(TabuSearch::KEY_SOLVER, TabuSearch::Config::KEY_NEIGHBOURHOOD_SIZE, 50);
        scfg.addValue(TabuSearch::KEY_SOLVER, TabuSearch::Config::KEY_TABU_LIST_LENGTH, 5);
        solver.addSolver(new TabuSearch(*ins));

        // Optimality stage
        {
            // Lazy constraints stage.
            scfg.addValue(LazyConstraints::KEY_SOLVER,
                          LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_ONE_SOLUTION,
                          false);
            scfg.addValue(LazyConstraints::KEY_SOLVER,
                          LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_TOWARDS_OPTIMAL,
                          false);
            scfg.addValue(LazyConstraints::KEY_SOLVER,
                          LazyConstraints::Config::KEY_GENERATE_CUTTING_CONSTRAINTS_ETFA2016,
                          true);
            scfg.addValue(LazyConstraints::KEY_SOLVER,
                          LazyConstraints::Config::KEY_NO_ENERGY_CONSUMPTION_LIMITS,
                          false);
            scfg.addValue(LazyConstraints::KEY_SOLVER,
                          LazyConstraints::Config::KEY_NO_DEVIATIONS,
                          false);
            solver.addSolver(new LazyConstraints(*ins, env));

            // Branch and bound stage
//            solver.addSolver(new BranchAndBoundOnOrder(*ins));
        }

        auto obj = new rseclp::TotalTardiness();
        Solver::Config cfg(chrono::seconds(3600), obj, false, StartTimes(), scfg);

        auto result = solver.solve(cfg);

        cout << "Solution feasible? " << FeasibilityChecker(*ins).areFeasible(result.getStartTimes()) << endl;
        cout << result.getStartTimes() << endl;
        cout << "Objective: " << result.getObjectiveValue() << endl;
        cout << "Status: " << result.getStatus() << endl;

        delete obj;
    }

    sw.stop();
    cout << "Elapsed time: " << sw.duration().count() << " ms" << endl;

    delete ins;
    return 0;
}
