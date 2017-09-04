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

#ifndef ROBUSTENERGYAWARESCHEDULING_JSONUTILS_H
#define ROBUSTENERGYAWARESCHEDULING_JSONUTILS_H

#include <vector>
#include <stdexcept>
#include <sstream>
#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include <rapidjson/allocators.h>


namespace rseclp {

    using namespace std;
    using namespace rapidjson;
    using namespace boost::filesystem;

    class JsonUtils {

    private:
        static const int BUFFER_SIZE = 65536;

    public:

        static void mergeDocuments(Document& source, Document& target);

        static void writeJsonDocument(const path &sourcePath, Document &source);

        static void readJsonDocument(const path &targetPath, Document &target, bool readParent = false);

        template <typename T>
        static vector<T> getVector(const Value &enclosingObject, const char *vectorName) {
            const Value &jsonVector = enclosingObject[vectorName];
            vector<T> v;
            for (Value::ConstValueIterator value = jsonVector.Begin(); value != jsonVector.End(); value++) {
                v.push_back(value->Get<T>());
            }
            return v;
        }

        template <typename T>
        static vector<T> getExpandedVector(const Value &enclosingObject, const char *memberName, int vectorSize) {
            const Value &member = enclosingObject[memberName];
            if (member.IsArray()) {
                return JsonUtils::getVector<T>(enclosingObject, memberName);
            }
            else {
                return vector<T>(vectorSize, member.Get<T>());
            }
        }

        template <typename T>
        static void write(Document &doc, Value &addTo, const char *arrayName, const vector<T> &values) {
            Value array;
            array.SetArray();
            for (auto &value : values) {
                array.PushBack(Value(value), doc.GetAllocator());
            }
            addTo.AddMember(Value(arrayName, doc.GetAllocator()), array, doc.GetAllocator());
        }

    };

}


#endif //ROBUSTENERGYAWARESCHEDULING_JSONUTILS_H
