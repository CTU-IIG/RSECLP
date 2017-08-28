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

#include "InstanceJsonWriter.h"
#include "../utils/JsonUtils.h"

namespace rseclp {
    void InstanceJsonWriter::write(const Instance &ins, const path &writePath) {
        Document doc;
        doc.SetObject();

        doc.AddMember("numOperations", Value(ins.getNumOperations()), doc.GetAllocator());
        auto releaseTimes = ins.collectReleaseTimes(); JsonUtils::write(doc, doc, "releaseTimes", releaseTimes);
        auto dueDates = ins.collectDueDates(); JsonUtils::write(doc, doc, "dueDates", dueDates);
        auto processingTimes = ins.collectProcessingTimes(); JsonUtils::write(doc, doc, "processingTimes", processingTimes);
        auto powerConsumptions = ins.collectPowerConsumptions(); JsonUtils::write(doc, doc, "powerConsumptions", powerConsumptions);
        doc.AddMember("maxDeviation", Value(ins.getMaxDeviation()), doc.GetAllocator());
        doc.AddMember("numMeteringIntervals", Value(ins.getNumMeteringIntervals()), doc.GetAllocator());
        doc.AddMember("lengthMeteringInterval", Value(ins.getLengthMeteringInterval()), doc.GetAllocator());
        auto maxEnergyConsumptions = ins.collectMaxEnergyConsumptions(); JsonUtils::write(doc, doc, "maxEnergyConsumptions", maxEnergyConsumptions);

        Value metadata;
        metadata.SetObject();
        for (auto &param : ins.getMetadata()) {
            metadata.AddMember(Value(param.first.c_str(), doc.GetAllocator()),
                               Value(param.second.c_str(), doc.GetAllocator()),
                               doc.GetAllocator());
        }

        doc.AddMember("metadata", metadata, doc.GetAllocator());

        JsonUtils::writeJsonDocument(writePath, doc);
    }
}
