# Robust production scheduling with energy consumption limits

This repository contains the source code (see `src/`), benchmark instances (see `experiment-data/datasets/`) and experiment results from [this paper](#citing) (see `experiment-data/experiments/<experiment-name>/results`) for the robust production scheduling problem with energy consumption limits.
For more details about the algorithms and instances, see the paper in [this section](#citing).

## Building the project
The project has been tested and known to work on Fedora 26 operating system.
Other systems (Windows, MacOS) should also work, although they may require some changes to the build script.

The project has following dependencies:
- [CMake](https://cmake.org/) (>= 3.7) - build system
- [RapidJSON](http://rapidjson.org/) (>= 1.1.0) - library for handling JSON files
- [Gurobi](http://www.gurobi.com/) (>= 7.0) - library for Mixed Integer Linear Programming (environment variable `$GUROBI_HOME` has to be set, please see the Gurobi documentation)
- [Boost system & filesystem](http://www.boost.org/) (>= 1.63.0) - utility libraries
- C++ compiler with support for C++11 standard

If all the dependencies are correctly installed, then it should be enough to do
```bash
mkdir ./bin
cd bin
cmake -DCMAKE_BUILD_TYPE=Release ../
make
```
which will generate the following executables
- `bin/rseclp-solver`
- `bin/rseclp-experiment`
- `bin/rseclp-datasetgen`

**Important:** if you are using `g++` compiler in version 5.0 or newer, it is possible that you have to built the C++ interface for Gurobi by yourself
```bash
cd $GUROBI_HOME/src/build
make
cp libgurobi_c++.a $GUROBI_HOME/lib
```

## File formats
All files are using JSON, which is a simple key-value file format.
To document the *fields* (key and value) of the files, we use `key: valueType` syntax, e.g. `dueDates: int[]` represents a field with key `dueDates` and integer array as a type.

The type may contain `|` character, which represents *union type*.
For example, `dueDates: int[] | int` is a field which may be specified either as an array or as a single value.
If a single value is given, then the file loader will automatically create an appropriately sized array (the length depends on the context) where each value is the same as the single value specified in the field, e.g. all the operations will have the same due date.

### Instance
An instance file has following fields
- `numOperations: int`
- `releaseTimes: int[] | int`
- `dueDates: int[] | int`
- `processingTimes: int[] | int`
- `powerConsumptions: double[] | double`
- `maxDeviation: int`
- `numMeteringIntervals: int`
- `lengthMeteringInterval: int`
- `maxEnergyConsumptions: double[] | double`

All the fields are mandatory.
The values of release times, due dates, processing times and power consumptions are given as a vector with increasing index of the operations (starting from 1).

Consider the following example instance (corresponds to the example instance from Section 2 in our [paper](#citing))
```json
{
  "numOperations": 5,
  "releaseTimes": [0, 6, 8, 10, 18],
  "dueDates": [5, 10, 15, 17, 30],
  "processingTimes": [2, 2, 7, 4, 3],
  "powerConsumptions": [50.0, 70.0, 150.0, 120.0, 30.0],
  "maxDeviation": 3,
  "numMeteringIntervals": 5,
  "lengthMeteringInterval": 15,
  "maxEnergyConsumptions": 1200.0
}
```
Notice that since the maximum energy consumption in every metering interval is the same, the limit is passed as a single value instead of vector.

### Instance result
A result file has following fields
- `status: int` the status code of the result, for the interpretation see enum `Solver::Result::Status` in file `src/rseclp/solvers/Solver.h`
- `startTimes: int[]`
- `objectiveValue: double`
- `solverRuntimeInMilliseconds: int`
- `optional: object` additional data that could be provided by the solver, e.g. Lazy Constraints solver will insert lower bound and the number of generated lazy constraints in this object.

### Solver configuration
A solver configuration file has following fields
- `timeLimitInMilliseconds: int`
- `gurobiEnvParams: object` parameters for Gurobi environment given as object.
The values of the parameters are always given as strings and the supported keys are listed in [Gurobi documentation](http://www.gurobi.com/documentation/7.5/refman/parameters.html#sec:Parameters).
- `initStartTimes: int[] | undefined`
- `useInitStartTimes: bool | undefined`
- `previousStage: string | undefined` used in experiments for passing the result of the previous solver to the current one.
- `solverStage: { name: string, cfg: object }` solver to use use and its specialized configuration passed in `cfg` object.
The name of the solver is defined in the solver source code as static field `KEY_SOLVER`, e.g. `LazyConstraints::KEY_SOLVER`.
The specialized config is an object where keys correspond to the values of static fields `KEY_*` in `Config` class in the solver source code, e.g. `GreedyHeuristics::Config::KEY_RULE` is a key for configuring the priority rule for the Greedy heuristics.

As an example consider the following configuration for Lazy Constraints
```json
{
  "timeLimitInMilliseconds": 1000,
  "gurobiEnvParams": {
    "OutputFlag": "1",
    "Threads": "1"
  },
  "initStartTimes": [ 8, 6, 10, 30, 18 ],
  "useInitStartTimes": true,
  "solverStage": {
    "name": "LazyConstraints",
    "cfg": {
      "generateCuttingConstraintsOneSolution": "0",
      "noEnergyConsumptionLimits": "0",
      "noDeviations": "0",
      "generateCuttingConstraintsTowardsOptimal": "1"
    }
  }
}
```


## Running a solver on a single instance
The command line interface is following
```bash
./bin/rseclp-solver SOLVER_CONFIG_PATH INSTANCE_PATH RESULT_PATH
```
where 
- `SOLVER_CONFIG_PATH` is the path to solver configuration file
- `INSTANCE_PATH` is the path to instance
- `RESULT_PATH` is the path where to store the result

## Running the experiment on a dataset
The command line interface is following
```bash
./bin/rseclp-experiment DATASET_NAME SOLVER_CONFIG NUM_THREADS
```
where
- `DATASET_NAME` is the name of the dataset in `experiment-data/datasets` to run the solver on
- `SOLVER_CONFIG` is the name of solver configuration in `experiment-data/experiments/$DATASET_NAME/` to use
- `NUM_THREADS` is the number of threads that will be solving different instances in parallel

For example,
```bash
./bin/rseclp-experiment n=10 lazy.json 3
```
will run the Lazy Constraints solver on `n=10` dataset with 3 instances being solved in parallel.

## License
[MIT license](LICENSE.txt)

## Authors
Please see file [AUTHORS.txt](AUTHORS.txt) for the list of authors.

## <a name="citing"></a>Citing
If you find our code or benchmark instances useful, we kindly request that you cite the following paper
```
@article{modos2017,
title = "Algorithms for robust production scheduling with energy consumption limits",
journal = "Computers & Industrial Engineering",
volume = "112",
pages = "391 - 408",
year = "2017",
issn = "0360-8352",
doi = "https://doi.org/10.1016/j.cie.2017.08.011",
author = "István Módos and Přemysl Šůcha and Zdeněk Hanzálek",
}
```