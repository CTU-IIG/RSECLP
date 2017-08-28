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
#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include "dataset-generators/DatasetGenerator2016B_1.h"
#include "dataset-generators/DatasetGenerator2016B_2.h"
#include "../rseclp/utils/JsonUtils.h"

using namespace std;
using namespace rseclp;
using namespace boost::filesystem;
using namespace rapidjson;

int main(int argc, char **argv) {
    if (argc != 2) {
        cout << "Error: dataset generator prescription filename not provided";
        exit(1);
    }
    path generatorPrescriptionFilename(argv[1]);
    string datasetName = generatorPrescriptionFilename.stem().string();

    path experimentDataPath = current_path() / "experiment-data";
    path generatorPrescriptionPath = experimentDataPath / "dataset-generator-prescriptions" / generatorPrescriptionFilename;

    cout << "Starting generator for " << datasetName << " dataset." << endl;

    Document doc;
    JsonUtils::readJsonDocument(generatorPrescriptionPath, doc);

    if ("DatasetGenerator2016B_1" == string(doc["generator"].GetString())) {
        DatasetGenerator2016B_1().fromPrescription(doc, datasetName, experimentDataPath / "datasets");
    }
    else if ("DatasetGenerator2016B_2" == string(doc["generator"].GetString())) {
        DatasetGenerator2016B_2().fromPrescription(doc, datasetName, experimentDataPath / "datasets");
    }
    else {
        cout << "Invalid generator " << doc["generator"].GetString() << endl;
        exit(1);
    }

    cout << "Done." << endl;
    return 0;
}
