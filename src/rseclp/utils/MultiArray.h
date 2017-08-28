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

#ifndef ROBUSTENERGYAWARESCHEDULING_MULTIARRAY_H
#define ROBUSTENERGYAWARESCHEDULING_MULTIARRAY_H

namespace rseclp {
    using namespace std;

    template<typename T>
    class MultiArray {
    private:
        vector<T> mArray;
        vector<int> mShape;

        int initShape(const vector<int> &shape) {
            int numEntries = 1;
            mShape.reserve(shape.size());
            for (int i = 0; i < (int)shape.size(); i++) {
                mShape.push_back(shape[i]);
                numEntries *= shape[i];
            }

            return numEntries;
        }

    public:

        MultiArray() { }

        MultiArray(const MultiArray<T> &arr);

        MultiArray(const vector<int> &shape);

        MultiArray(const vector<int> &shape, const vector<T> fillWith) {
            int numEntries = initShape(shape);
            mArray = vector<T>(fillWith.begin(), fillWith.end());
        }

        MultiArray(const vector<int> &shape, T fillWith) {
            int numEntries = initShape(shape);
            mArray = vector<T>(numEntries, fillWith);
        }

        int computeIndex(const vector<int> &indices) const {
            int index = 0;
            int stride = 1;
            for (int i = (int) (mShape.size() - 1); i >= 0; i--) {
                index += stride * indices[i];
                stride *= mShape[i];
            }

            return index;
        }

        T &operator()(const int i0) {
            return (*this)(vector<int>{i0});
        }

        const T &operator()(const int i0) const {
            return (*this)(vector<int>{i0});
        }

        T &operator()(const int i0, const int i1) {
            return (*this)(vector<int>{i0, i1});
        }

        const T &operator()(const int i0, const int i1) const {
            return (*this)(vector<int>{i0, i1});
        }

        T &operator()(const int i0, const int i1, const int i2) {
            return (*this)({i0, i1, i2});
        }

        const T &operator()(const int i0, const int i1, const int i2) const {
            return (*this)({i0, i1, i2});
        }

        T &operator()(const int i0, const int i1, const int i2, const int i3) {
            return (*this)({i0, i1, i2, i3});
        }

        const T &operator()(const int i0, const int i1, const int i2, const int i3) const {
            return (*this)({i0, i1, i2, i3});
        }

        T &operator()(const vector<int> &indices) {
            return mArray[computeIndex(indices)];
        }

        const T &operator()(const vector<int> &indices) const {
            return mArray[computeIndex(indices)];
        }

        const vector<int> &shape() const {
            return mShape;
        }

        int size() const {
            return (int)mArray.size();
        }

        int shape(int dim) const {
            return mShape[dim];
        }

        int numDims() const {
            return (int)mShape.size();
        }

        void directSet(const int index, const T &value) {
            mArray[index] = value;
        }

        const vector<T> &backingArray() const {
            return mArray;
        }
    };
}


#endif //ROBUSTENERGYAWARESCHEDULING_MULTIARRAY_H
