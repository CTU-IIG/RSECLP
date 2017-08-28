# Robust production scheduling with energy consumption limits

This repository contains the source code (see `src/`) and benchmark instances (see `experiment-data/datasets/`) for the robust production scheduling problem with energy consumption limits.
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
cmake ../
make
```
which will generate the `bin/experiment` executable.

**Important:** if you are using `g++` compiler in version 5.0 or newer, it is possible that you have to built the C++ interface for Gurobi by yourself
```bash
cd $GUROBI_HOME/src/build
make
cp libgurobi_c++.a $GUROBI_HOME/lib
```

## Running the experiment on dataset
The command line interface is following
```bash
./bin/experiment DATASET_NAME SOLVER_CONFIG NUM_THREADS
```
where
- `DATASET_NAME` is the name of the dataset in `experiment-data/datasets` to run the solver on
- `SOLVER_CONFIG` is the solver configuration in `experiment-data/experiments/$DATASET_NAME/` to use
- `NUM_THREADS` is the number of threads that will be solving different instances in parallel

For example,
```bash
./bin/experiment n=10 lazy.json 3
```
will run the Lazy Constraints solver on `n=10` dataset with 3 instances being solved in parallel.

# License
[MIT license](LICENSE.txt)

# Authors
Please see file [AUTHORS.txt](AUTHORS.txt) for the list of authors.

# <a name="citing"></a>Citing
If you find our code or benchmark instances useful, we kindly request that you cite the following paper
```
@article{modos2017,
title = "Algorithms for robust production scheduling with energy consumption limits",
journal = "Computers & Industrial Engineering",
volume = "",
number = "",
pages = "",
year = "",
issn = "",
doi = "",
url = "",
author = "István Módos and Přemysl Šůcha and Zdeněk Hanzálek",
}
```