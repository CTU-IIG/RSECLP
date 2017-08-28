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

#include "SolverResultJsonReader.h"
#include "../utils/JsonUtils.h"

namespace rseclp {
    Solver::Result SolverResultJsonReader::read(const path &resultPath) {
        Document doc;
        JsonUtils::readJsonDocument(resultPath, doc);

        map<string, string> optional;
        for (auto itOptional = doc["optional"].MemberBegin(); itOptional != doc["optional"].MemberEnd(); itOptional++) {
            optional[itOptional->name.GetString()] = move(string(itOptional->value.GetString()));
        }

        return Solver::Result(static_cast<Solver::Result::Status>(doc["status"].GetInt()),
                              move(StartTimes(JsonUtils::getVector<int>(doc, "startTimes"))),
                              doc["objectiveValue"].GetDouble(),
                              chrono::milliseconds(doc["solverRuntimeInMilliseconds"].GetInt64()),
                              move(optional));
    }
}
