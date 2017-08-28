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
#include "Solver.h"

namespace rseclp {

    Solver::~Solver() { }

    void Solver::SpecialisedConfig::addValue(const string &keySolver, const string &key, const string &value) {
        if (mSolverToParams.find(keySolver) == mSolverToParams.end()) {
            mSolverToParams[keySolver] = map<string, string>();
        }

        mSolverToParams[keySolver][key] = value;
    }

    void Solver::SpecialisedConfig::addValue(const string &keySolver, const string &key, const bool value) {
        addValue(keySolver, key, value == false ? string("0") : string("1"));
    }

    void Solver::SpecialisedConfig::addValue(const string &keySolver, const string &key, const int value) {
        addValue(keySolver, key, to_string(value));
    }

    bool Solver::SpecialisedConfig::testAndGetValue(const string &keySolver, const string &key, string &valueOut) const {
        if (mSolverToParams.find(keySolver) != mSolverToParams.cend()) {
            const auto &keyToValue = mSolverToParams.at(keySolver);
            if (keyToValue.find(key) != keyToValue.cend()) {
                valueOut = keyToValue.at(key);
                return true;
            }
            return false;
        }
        return false;
    }


    bool Solver::SpecialisedConfig::testAndGetValue(const string &keySolver, const string &key, bool &valueOut) const {
        string stringValue = "";
        if (testAndGetValue(keySolver, key, stringValue)) {
            valueOut = stringValue != "0";
            return true;
        }
        else {
            return false;
        }
    }

    int Solver::SpecialisedConfig::testAndGetValue(const string &keySolver, const string &key, int &valueOut) const {
        string stringValue = "";
        if (testAndGetValue(keySolver, key, stringValue)) {
            valueOut = stoi(stringValue);
            return true;
        }
        else {
            return false;
        }
    }

    Solver::Config::Config(chrono::milliseconds timeLimit,
                           const Objective *objective,
                           bool useInitStartTimes,
                           StartTimes initStartTimes,
                           Solver::SpecialisedConfig specialisedConfig)
            : mTimeLimit(timeLimit),
              mObjective(objective),
              mUseInitStartTimes(useInitStartTimes),
              mInitStartTimes(move(initStartTimes)),
              mSpecialisedConfig(move(specialisedConfig)) { }

    const chrono::milliseconds &Solver::Config::getTimeLimit() const {
        return mTimeLimit;
    }

    const Objective *Solver::Config::getObjective() const {
        return mObjective;
    }

    bool Solver::Config::getUseInitStartTimes() const {
        return mUseInitStartTimes;
    }

    const StartTimes &Solver::Config::getInitStartTimes() const {
        return mInitStartTimes;
    }

    const Solver::SpecialisedConfig &Solver::Config::getSpecialisedConfig() const {
        return mSpecialisedConfig;
    }

    Solver::Result::Result(const int numOperations, const double objectiveValue)
            : mStatus(Solver::Result::Status::NO_SOLUTION),
              mStartTimes(numOperations),
              mObjectiveValue(objectiveValue),
              mSolverRuntime(0) { }

    void Solver::Result::setStatus(const Status status) {
        mStatus = status;
    }

    void Solver::Result::setSolution(const Solver::Result::Status status,
                                     const StartTimes &startTimes,
                                     const double objectiveValue) {
        mStatus = status;
        mStartTimes = startTimes;
        mObjectiveValue = objectiveValue;
    }

    void Solver::Result::setSolverRuntime(const chrono::milliseconds &solverRuntime) {
        mSolverRuntime = solverRuntime;
    }

    void Solver::Result::setOptional(const string &key, const string &value) {
        mOptional[key] = value;
    }

    Solver::Result::Status Solver::Result::getStatus() const {
        return mStatus;
    }

    const StartTimes &Solver::Result::getStartTimes() const {
        return mStartTimes;
    }

    double Solver::Result::getObjectiveValue() const {
        return mObjectiveValue;
    }

    const chrono::milliseconds &Solver::Result::getSolverRuntime() const {
        return mSolverRuntime;
    }

    const map<string, string> &Solver::Result::getOptional() const {
        return mOptional;
    }

    Solver::Result::Result(Status status,
                           StartTimes startTimes,
                           double objectiveValue,
                           chrono::milliseconds solverRuntime,
                           map<string, string> optional)
            : mStatus(status),
              mStartTimes(move(startTimes)),
              mObjectiveValue(objectiveValue),
              mSolverRuntime(solverRuntime),
              mOptional(move(optional)) {}
}
