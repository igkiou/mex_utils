/*
 * test_utils.cpp
 *
 *  Created on: Feb 2, 2013
 *      Author: igkiou
 */

#include <iostream>

#include "mex_utils.h"
#include "mat_utils.h"

mex::MxString test() {
	return mex::MxString("gkiou");
}

mex::MxString test2() {
	mex::MxString ret("gkiou");
	mex::MxString ret2(ret);
	return ret2;
}

std::string test3() {
	return std::string("gkiou");
}

template <typename T>
void permute_inplace(T* data, std::vector<int> &perm) {
	for (size_t i=0; i<perm.size(); i++) {
		if (perm[i] != i) {
			/* The start of a new cycle has been found. Save
			   the value at this position, since it will be
			   overwritten */
			int j = (int) i;
			T curval = data[i];

			do {
				/* Shuffle backwards */
				int k = perm[j];
				data[j] = data[k];

				/* Also fix the permutations on the way */
				perm[j] = j;
				j = k;

				/* Until the end of the cycle has been found */
			} while (perm[j] != i);

			/* Fix the final position with the saved value */
			data[j] = curval;
			perm[j] = j;
		}
	}
}

template < class T >
inline std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
{
    os << "[";
    for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii)
    {
        os << " " << *ii;
    }
    os << " ]";
    return os;
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	int height = 10;
	int width = 10;
//	const int dims[3] = {height, width, 3};
//	plhs[0] = mxCreateNumericArray(3, dims, mxSINGLE_CLASS, mxREAL); /* x */
	mex::MxNumeric<float> temp(height, width);
//	plhs[0] = temp.get_array();
	temp = mex::MxNumeric<float>(height, width);
	mex::MxNumeric<float> temp2(temp);
//	mex::MxString tempg(mex::MxArray(temp.get_array()));
//	tempg[0] = 0;
	plhs[2] = temp.get_array();
	temp[1] = 5.0;
	std::vector<float> vec;
	vec.push_back(10);
	vec.push_back(20);
	vec.push_back(30);
	plhs[3] = mex::MxNumeric<float>(vec).get_array();
	mex::MxNumeric<int> foo(height, width);
	foo.getData()[0] = 10;
	plhs[4] = foo.get_array();
	std::vector<int> veci = foo.vectorize();
	std::cout << "length " << veci.size() << std::endl;
	for (int iter = 0, end = veci.size(); iter < end; ++iter) {
		std::cout << "iter " << iter << " value "<< veci[iter] << std::endl;
	}
	std::vector<bool> dummy(10, false);
	plhs[5] = mex::MxNumeric<bool>(dummy).get_array();
//	mexPrintf("M %d N %d size %d numel %d.\n", foo.M(), foo.N(), foo.size(), foo.numel());
	plhs[6] = mex::MxString(std::string("gkiou")).get_array();
	plhs[7] = mex::MxString("gkiou").get_array();
	std::string gkiou("gkiou");
	plhs[8] = mex::MxString(gkiou).get_array();
	mex::MxString gkiou2("gkiou");
	plhs[9] = mex::MxString(gkiou2).get_array();
	plhs[10] = test().get_array();
	plhs[11] = test2().get_array();
	std::vector<std::string> names;
	std::vector<mex::MxArray*> vars;
	names.push_back("field1");
	names.push_back("field2");
	names.push_back("field3");
	mex::MxArray* tempp = new mex::MxNumeric<float>(height,width);
	vars.push_back(tempp);
	tempp = new mex::MxNumeric<bool>(height,width);
	vars.push_back(tempp);
	tempp = new mex::MxString("field");
	vars.push_back(tempp);
	plhs[12] = mex::MxStruct(names, vars).get_array();
	delete vars[0];
	delete vars[1];
	delete vars[2];
	std::vector<int> dims;
	dims.push_back(10);
	dims.push_back(5);
	dims.push_back(2);
	temp = mex::MxNumeric<float>(int(dims.size()), &dims[0]);
	for (int iter = 0; iter < temp.getNumberOfElements(); ++iter) {
		temp[iter] = iter;
	}
	std::vector<int> perm;
	perm.push_back(2);
	perm.push_back(1);
	perm.push_back(3);
	plhs[0] = temp.permute(perm).get_array();
	plhs[1] = temp.get_array();
	double* a = (double*) malloc(100 * sizeof(double));
	double* b = (double*) malloc(100 * sizeof(double));
	memcpy(a, b, 100 * sizeof(double));
	mex::MatFile file("test.mat", "u");
	std::vector<std::string> varnames = file.getVariableNames();
	std::cout << "number of variables: " << varnames.size() << std::endl;
	for (int iter = 0, end = varnames.size(); iter < end; ++iter) {
		std::cout << "variable " << iter << " name: " << varnames[iter] << std::endl;
	}
	file.write(temp, "temp");
	plhs[1] = file.read(varnames[0]).get_array();
	file.clear(varnames[0]);
}
