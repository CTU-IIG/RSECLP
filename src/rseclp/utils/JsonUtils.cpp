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

#include <rapidjson/filewritestream.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/prettywriter.h>
#include "JsonUtils.h"

namespace rseclp {

    void JsonUtils::mergeDocuments(Document &source, Document &target) {
        for (Value::MemberIterator member = source.MemberBegin(); member != source.MemberEnd(); member++) {
            if (target.HasMember(member->name)) {
                target.RemoveMember(member->name);
            }

            Value newValue;
            newValue.CopyFrom(member->value, target.GetAllocator());

            Value nameValue;
            nameValue.CopyFrom(member->name, target.GetAllocator());

            target.AddMember(nameValue, newValue, target.GetAllocator());
        }
    }

    // TODO (refactoring): names of parameters are confusing.
    void JsonUtils::writeJsonDocument(const path &sourcePath, Document &source) {
        FILE *pathFile = fopen(sourcePath.c_str(), "w");
        char writeBuffer[JsonUtils::BUFFER_SIZE];
        FileWriteStream pathStream(pathFile, writeBuffer, sizeof(writeBuffer));
        PrettyWriter<FileWriteStream> pathStreamWriter(pathStream);
        source.Accept(pathStreamWriter);
        fclose(pathFile);
    }

    // TODO (refactoring): names of parameters are confusing.
    void JsonUtils::readJsonDocument(const path &targetPath, Document &target, bool readParent) {
        if (!boost::filesystem::exists(targetPath)) {
            std::stringstream msg;
            msg << "Json file '" << targetPath << "' not found." << endl;
            throw std::invalid_argument(msg.str());
        }

        if (readParent == false) {
            FILE *pathFile = fopen(targetPath.c_str(), "r");

            char readBuffer[JsonUtils::BUFFER_SIZE];
            FileReadStream pathStream(pathFile, readBuffer, sizeof(readBuffer));
            target.ParseStream(pathStream);
            fclose(pathFile);
        }
        else {
            Document tempDoc;
            FILE *pathFile = fopen(targetPath.c_str(), "r");

            char readBuffer[JsonUtils::BUFFER_SIZE];
            FileReadStream pathStream(pathFile, readBuffer, sizeof(readBuffer));
            tempDoc.ParseStream(pathStream);
            fclose(pathFile);

            if (tempDoc.HasMember("parentPath")) {
                path basePath = targetPath.parent_path();
                path parentPath = tempDoc["parentPath"].GetString();
                path canonicalParentPath = canonical(parentPath, basePath);

                Document parentDoc;
                parentDoc.SetObject();
                JsonUtils::readJsonDocument(canonicalParentPath, parentDoc, true);
                JsonUtils::mergeDocuments(tempDoc, parentDoc);
                JsonUtils::mergeDocuments(parentDoc, target);
            }
            else {
                JsonUtils::mergeDocuments(tempDoc, target);
            }
        }
    }

}
