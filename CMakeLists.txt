project(RobustSchedulingWithEnergyConsumptionLimits)
cmake_minimum_required(VERSION 3.7)

# ETFA 2016
# add_definitions(-D_2016A)

# WIP Journal paper
add_definitions(-D_2016B)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -march=native -pthread -pedantic -Wall -Wextra -Werror -Wconversion")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules/")

set(SOLVER_NAME rseclp-solver)
set(DATASETGEN_NAME rseclp-datasetgen)
set(EXPERIMENT_NAME rseclp-experiment)

set(APP_SRC
        ./src/app/main.cpp
        )

set(DATASETGEN_SRC
        src/datasetgen/main.cpp
        src/datasetgen/dataset-generators/DatasetGenerator.h
        src/datasetgen/dataset-generators/DatasetGenerator2016B_1.cpp src/datasetgen/dataset-generators/DatasetGenerator2016B_1.h
        src/datasetgen/dataset-generators/DatasetGenerator2016B_2.cpp src/datasetgen/dataset-generators/DatasetGenerator2016B_2.h
        )

set(EXPERIMENT_SRC
        src/experiment/main.cpp
        src/experiment/ExperimentPrescription.cpp src/experiment/ExperimentPrescription.h
        )

set(LIB_SRC
        src/rseclp/instance/Instance.cpp src/rseclp/instance/Instance.h
        src/rseclp/instance/MeteringInterval.cpp src/rseclp/instance/MeteringInterval.h
        src/rseclp/instance/Operation.cpp src/rseclp/instance/Operation.h
        src/rseclp/instance/InstanceJsonReader.cpp src/rseclp/instance/InstanceJsonReader.h
        src/rseclp/instance/InstanceJsonWriter.cpp src/rseclp/instance/InstanceJsonWriter.h
        src/rseclp/utils/JsonUtils.cpp src/rseclp/utils/JsonUtils.h
        src/rseclp/utils/StartTimes.cpp src/rseclp/utils/StartTimes.h
        src/rseclp/utils/GeneralUtils.cpp src/rseclp/utils/GeneralUtils.h
        src/rseclp/utils/Stopwatch.cpp src/rseclp/utils/Stopwatch.h
        src/rseclp/utils/MultiArray.cpp src/rseclp/utils/MultiArray.h
        src/rseclp/utils/IlpUtils.cpp src/rseclp/utils/IlpUtils.h
        src/rseclp/utils/Interval.h
        src/rseclp/solvers/exact/BruteForceSearch.cpp src/rseclp/solvers/exact/BruteForceSearch.h
        src/rseclp/solvers/exact/BranchAndBoundOnOrder.cpp src/rseclp/solvers/exact/BranchAndBoundOnOrder.h
        src/rseclp/solvers/exact/LazyConstraints.cpp src/rseclp/solvers/exact/LazyConstraints.h
        src/rseclp/solvers/fixed-order/RobustScheduleFixedOrder.cpp src/rseclp/solvers/fixed-order/RobustScheduleFixedOrder.h
        src/rseclp/solvers/fixed-order/RobustScheduleFixedOrderDefault.cpp src/rseclp/solvers/fixed-order/RobustScheduleFixedOrderDefault.h
        src/rseclp/solvers/fixed-order/RobustScheduleFixedOrderOptimised.cpp src/rseclp/solvers/fixed-order/RobustScheduleFixedOrderOptimised.h
        src/rseclp/solvers/fixed-order/RobustScheduleFixedOrder2016B.cpp src/rseclp/solvers/fixed-order/RobustScheduleFixedOrder2016B.h
        src/rseclp/solvers/heuristics/GreedyHeuristics.cpp src/rseclp/solvers/heuristics/GreedyHeuristics.h
        src/rseclp/solvers/Solver.cpp src/rseclp/solvers/Solver.h
        src/rseclp/solvers/MultiStageSolver.cpp src/rseclp/solvers/MultiStageSolver.h
        src/rseclp/solvers/SolverResultJsonWriter.cpp src/rseclp/solvers/SolverResultJsonWriter.h
        src/rseclp/feasibility-checkers/FeasibilityChecker.cpp src/rseclp/feasibility-checkers/FeasibilityChecker.h
        src/rseclp/objectives/Objective.cpp src/rseclp/objectives/Objective.h
        src/rseclp/objectives/TotalTardiness.cpp src/rseclp/objectives/TotalTardiness.h
        src/rseclp/solvers/SolverResultJsonReader.cpp src/rseclp/solvers/SolverResultJsonReader.h
        src/rseclp/solvers/heuristics/TabuSearch.cpp src/rseclp/solvers/heuristics/TabuSearch.h
        src/rseclp/solvers/SolverPrescription.cpp src/rseclp/solvers/SolverPrescription.h)

find_package(GUROBI REQUIRED)
include_directories(${GUROBI_INCLUDE_DIR})

find_package(RapidJSON REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

add_library(rseclp ${LIB_SRC})

add_executable(${SOLVER_NAME} ${APP_SRC})
target_link_libraries(${SOLVER_NAME}
    rseclp
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${GUROBI_LIBRARIES}
    )

add_executable(${DATASETGEN_NAME} ${DATASETGEN_SRC})
target_link_libraries(${DATASETGEN_NAME}
    rseclp
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${GUROBI_LIBRARIES}
    )

add_executable(${EXPERIMENT_NAME} ${EXPERIMENT_SRC})
target_link_libraries(${EXPERIMENT_NAME}
    rseclp
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${GUROBI_LIBRARIES}
    )
