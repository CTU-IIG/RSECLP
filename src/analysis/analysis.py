'''
    
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
'''

import os.path
import json
from enum import Enum

import numpy as np
import matplotlib.pyplot as plt

ROOT_EXPERIMENT_DATA_PATH = os.path.join(os.path.abspath("./"), "experiment-data")


class Status(Enum):
    no_solution = 0
    optimal = 1
    infeasible = 2
    feasible = 3


class Results:

    def __init__(self, results, friendly_solver_names, solver_order):
        self.results = results
        self.friendly_solver_names = friendly_solver_names
        self.solver_order = solver_order

    @staticmethod
    def load_results(results_path, friendly_solver_names, solver_order):
        results = dict()

        for solver_name in os.listdir(results_path):
            results[solver_name] = dict()
            solver_path = os.path.join(results_path, solver_name)
            for prescription_name in os.listdir(solver_path):
                prescription_path = os.path.join(solver_path, prescription_name)
                for result_filename in os.listdir(prescription_path):
                    instance_name = os.path.splitext(result_filename)[0]
                    results[solver_name][prescription_name + "-" + instance_name] = load_result(os.path.join(prescription_path, result_filename))

        return Results(results, friendly_solver_names, solver_order)

    def solver_names(self):
        return list(self.results.keys())


def has_solution(result):
    return result["status"] == Status.optimal or result["status"] == Status.feasible


def load_result(result_path):
    with open(result_path, "r") as f:
        result_raw = json.load(f)
        result_raw["status"] = Status(result_raw["status"])
        result_raw["startTimes"] = np.array(result_raw["startTimes"])
        return result_raw


def get_instances_without_solution(results):
    instances_without_solution = set()
    for solver_results in results.results.values():
        for instance_name, result in solver_results.items():
            if not has_solution(result):
                instances_without_solution.add(instance_name)

    return instances_without_solution


def figs_objective_values_boxplots(results):
    instances_without_solution = get_instances_without_solution(results)
    solver_name_to_objective_values = dict()
    for solver_name, solver_results in results.results.items():
        solver_name_to_objective_values[solver_name] = []
        for instance_name, result in solver_results.items():
            if instance_name not in instances_without_solution:
                solver_name_to_objective_values[solver_name].append(result["objectiveValue"])

    plt.boxplot([solver_name_to_objective_values[solver_name]
                 for solver_name in results.solver_order],
                labels=results.solver_order)


def solved_instances_to_optimality(results):
    solver_name_to_optimal_results = dict()
    for solver_name, solver_results in results.results.items():
        solver_name_to_optimal_results[solver_name] = []
        for instance_name, result in solver_results.items():
            if result["status"] == Status.optimal:
                solver_name_to_optimal_results[solver_name].append(result)

    for solver_name, optimal_results in solver_name_to_optimal_results.items():
        print("Optimal instances for {0}: {1}".format(solver_name, len(optimal_results)))


def main():
    results_path = os.path.join(ROOT_EXPERIMENT_DATA_PATH, "experiments", "n=15", "results")
    friendly_solver_names = {"tabu1": "Tabu", "greedy": "Greedy", "lazy": "Lazy", "random": "Random", "bab": "BranchAndBound"}
    solver_order = ["greedy", "tabu1", "bab", "lazy"]
    results = Results.load_results(results_path, friendly_solver_names, solver_order)
    figs_objective_values_boxplots(results)
    solved_instances_to_optimality(results)

    plt.show()


if __name__ == "__main__":
    main()
