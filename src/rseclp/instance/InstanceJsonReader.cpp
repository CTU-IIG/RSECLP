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

#include "../utils/JsonUtils.h"
#include "InstanceJsonReader.h"

namespace rseclp {

    Instance *InstanceJsonReader::read(const path &instancePath) {
        Document doc;
        JsonUtils::readJsonDocument(instancePath, doc);

        int numOperations = doc["numOperations"].GetInt();

        vector<int> releaseTimes = JsonUtils::getExpandedVector<int>(doc, "releaseTimes", numOperations);
        vector<int> dueDates = JsonUtils::getExpandedVector<int>(doc, "dueDates", numOperations);
        vector<int> processingTimes = JsonUtils::getExpandedVector<int>(doc, "processingTimes", numOperations);
        vector<double> powerConsumptions = JsonUtils::getExpandedVector<double>(doc, "powerConsumptions", numOperations);

        int maxDeviation = doc["maxDeviation"].GetInt();
        int numMeteringIntervals = doc["numMeteringIntervals"].GetInt();
        int lengthMeteringInterval = doc["lengthMeteringInterval"].GetInt();
        vector<double> maxEnergyConsumptions = JsonUtils::getExpandedVector<double>(doc,
                                                                                    "maxEnergyConsumptions",
                                                                                    numMeteringIntervals);

        map<string, string> metadata;
        if (doc.HasMember("metadata")) {
            for (auto itMetadata = doc["metadata"].MemberBegin(); itMetadata != doc["metadata"].MemberEnd(); itMetadata++) {
                metadata[itMetadata->name.GetString()] = move(string(itMetadata->value.GetString()));
            }
        }

        return Instance::create(numOperations,
                                releaseTimes,
                                dueDates,
                                processingTimes,
                                powerConsumptions,
                                maxDeviation,
                                numMeteringIntervals,
                                lengthMeteringInterval,
                                maxEnergyConsumptions,
                                metadata);
    }
}
