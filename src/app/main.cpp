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
#include "../rseclp/solvers/SolverResultJsonWriter.h"
#include "../rseclp/instance/InstanceJsonReader.h"
#include "../rseclp/solvers/SolverPrescription.h"

using namespace std;
using namespace rseclp;

int main(int argc, char **argv) {
    if (argc != 4) {
        cout << "Error: not every argument provided";
        exit(1);
    }

    boost::filesystem::path solverPrescriptionPath = argv[1];
    boost::filesystem::path instancePath = argv[2];
    boost::filesystem::path resultPath = argv[3];

    unique_ptr<SolverPrescription> solverPrescription(SolverPrescription::read(solverPrescriptionPath));
    unique_ptr<Instance> ins(rseclp::InstanceJsonReader::read(instancePath));
    unique_ptr<Solver> solver((*solverPrescription).createSolver(*ins));

    auto result = (*solver).solve((*solverPrescription).getConfig());
    switch (result.getStatus()) {
        case Solver::Result::Status::NO_SOLUTION:
            cout << "Result status: no solution found" << endl;
            break;

        case Solver::Result::Status::INFEASIBLE:
            cout << "Result status: the instance is infeasible" << endl;
            break;

        case Solver::Result::Status::FEASIBLE:
            cout << "Result status: feasible solution found" << endl;
            cout << "Solution objective: " << result.getObjectiveValue() << endl;
            cout << "Solution: " << result.getStartTimes() << endl;
            break;

        case Solver::Result::Status::OPTIMAL:
            cout << "Result status: optimal solution found" << endl;
            cout << "Solution objective: " << result.getObjectiveValue() << endl;
            cout << "Solution: " << result.getStartTimes() << endl;
            break;
    }

    cout << "Solver runtime [ms]: " << result.getSolverRuntime().count() << endl;

    SolverResultJsonWriter::write(result, resultPath);

    return 0;
}
