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

#ifndef ROBUSTENERGYAWARESCHEDULING_DATASETGENERATOR2016B_1_H
#define ROBUSTENERGYAWARESCHEDULING_DATASETGENERATOR2016B_1_H

#include <boost/filesystem.hpp>
#include <rapidjson/document.h>
#include "DatasetGenerator.h"
#include "../../rseclp/instance/Instance.h"

namespace rseclp {
    using namespace std;
    using namespace rapidjson;
    using namespace boost::filesystem;

    class DatasetGenerator2016B_1 : public DatasetGenerator {
    private:
        class PrescriptionRealisation {
        public:
            const int mNumInstances;
            const int mNumOperations;
            const int mNumMeteringIntervalsMul;
            const double mAlpha1;
            const double mAlpha2;
            const double mAlpha3;
            const int mMaxDeviation;

            PrescriptionRealisation(int numInstances,
                                    int numOperations,
                                    int numMeteringIntervalsMul,
                                    double alpha1,
                                    double alpha2,
                                    double alpha3,
                                    int maxDeviation);

            static void writeToJson(const path &targetPath, const PrescriptionRealisation &pr);
        };

        class Prescription {
        public:
            const int mNumInstances;
            const vector<int> mNumOperationsVector;
            const vector<int> mNumMeteringIntervalsMulVector;
            const vector<double> mAlpha1Vector;
            const vector<double> mAlpha2Vector;
            const vector<double> mAlpha3Vector;
            const vector<int> mMaxDeviationVector;

            static Prescription load(const Document &doc);

            Prescription(int numInstances,
                         vector<int> numOperationsVector,
                         vector<int> numMeteringIntervalsMulVector,
                         vector<double> alpha1Vector,
                         vector<double> alpha2Vector,
                         vector<double> alpha3Vector,
                         vector<int> maxDeviationVector);
        };

        void generate(const DatasetGenerator2016B_1::Prescription &p, const path &prescriptionOutputDir) const;

    public:

        virtual void fromPrescription(Document &prescriptionDocument,
                                      const string &datasetName,
                                      const path &outputDir);
    };
}

#endif //ROBUSTENERGYAWARESCHEDULING_DATASETGENERATOR2016B_1_H
