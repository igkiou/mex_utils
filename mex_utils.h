/*
 * mex_utils.h
 *
 *  Created on: Jan 30, 2013
 *      Author: igkiou
 */

#ifndef MEX_UTILS_H_
#define MEX_UTILS_H_

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <string.h>

/*
 * TODO: Maybe include these inside mex namespace, to make sure mex:: needs to
 * be used? Check if this actually works.
 * TODO: Add a nargin/out checker.
 */
#include "mex.h"
#include "matrix.h"

/*
 * TODO: Overarching design goal: No MATLAB internals visible to public.
 * TODO: Add struct array support.
 * TODO: Add support for initialization by const mxArray*.
 * TODO: Is giving access to a data element with [] im MxCell and MxStruct
 * equivalent to using mxSetCell and mxSetField, respectively?
 * TODO: Provide cast from base MxArray to other types?
 * TODO: Add constructor of derived classes from base MxArray?
 * TODO: Provide templated version of MxArray that uses mxClassID instead of
 * C/C++ intrinsic types?
 * TODO: Provide index-based accessor to data?
 * TODO: Change uses of mxCELL, mxCHAR and mxSTRUCT to refer to MxClassId
 * typedefs.
 * TODO: Replace int for indices with template.
 */
namespace mex {

#ifdef NDEBUG
#define mexAssert(cond) ((void) 0)
#define mexAssertEx(cond, explanation) ((void) 0)
#else
#define mexAssert(cond) do { \
		if (!(cond)) \
		mexErrMsgIdAndTxt("MATLAB:mex", "Assertion \"%s\" failed in %s:%i\n", \
		#cond, __FILE__, __LINE__); \
	} while (0)

#define mexAssertEx(cond, explanation) do { \
		if (!(cond)) \
		mexErrMsgIdAndTxt(\
		"MATLAB:mex", "Assertion \"%s\" failed in %s:%i (" explanation ")\n", \
		#cond, __FILE__, __LINE__); \
	} while (0)
#endif

template <typename T, typename U>
class ConstMap {
public:
	/// Constructor with a single key-value pair
	ConstMap(const T& key, const U& val) {
		m_map[key] = val;
	}

	/// Consecutive insertion operator
	ConstMap<T, U>& operator()(const T& key, const U& val) {
		m_map[key] = val;
		return *this;
	}

	inline const std::map<T, U>& get_map() const {
		return m_map;
	}

	/// Lookup operator; fail if not found
	inline U operator [](const T& key) const {
		typename std::map<T,U>::const_iterator iter = m_map.find(key);
		mexAssertEx(iter != m_map.end(), "Value not found");
		return (*iter).second;
	}

private:
	std::map<T, U> m_map;
};

class MxClass {
public:
	MxClass()
		: m_classId(mxUNKNOWN_CLASS) {	}

	explicit MxClass(const MxClass& mxClass)
					: m_classId(mxClass.get_classId()) {	}

	MxClass& operator=(const MxClass& mxClass) {
		if (this != &mxClass) {
			this->m_classId = mxClass.m_classId;
		}
		return *this;
	}

	inline mxClassID get_classId() const {
		return m_classId;
	}

	inline mxClassID operator()() const {
		return m_classId;
	}

	inline operator mxClassID() const {
		return m_classId;
	}

	inline bool operator==(const MxClass& mxClass) const {
		return (m_classId == mxClass.get_classId());
	}

	virtual ~MxClass() {	};

protected:
	MxClass(const mxClassID mxClass)
		: m_classId(mxClass) {	}

private:
	mxClassID m_classId;
};

template <typename NumericType>
class MxNumericClass : public MxClass {
public:
	MxNumericClass()
				: MxClass(mxUNKNOWN_CLASS) {	}
};
template <> MxNumericClass<unsigned char>::MxNumericClass()
											: MxClass(mxUINT8_CLASS) {	}
template <> MxNumericClass<char>::MxNumericClass()
											: MxClass(mxINT8_CLASS) {	}
template <> MxNumericClass<short>::MxNumericClass()
											: MxClass(mxINT16_CLASS) {	}
template <> MxNumericClass<unsigned short>::MxNumericClass()
											: MxClass(mxUINT16_CLASS) {	}
template <> MxNumericClass<int>::MxNumericClass()
											: MxClass(mxINT32_CLASS) {	}
template <> MxNumericClass<unsigned int>::MxNumericClass()
											: MxClass(mxUINT32_CLASS) {	}
template <> MxNumericClass<long>::MxNumericClass()
											: MxClass(mxINT64_CLASS) {	}
template <> MxNumericClass<unsigned long>::MxNumericClass()
											: MxClass(mxUINT64_CLASS) {	}
template <> MxNumericClass<float>::MxNumericClass()
											: MxClass(mxSINGLE_CLASS) {	}
template <> MxNumericClass<double>::MxNumericClass()
											: MxClass(mxDOUBLE_CLASS) {	}
template <> MxNumericClass<bool>::MxNumericClass()
											: MxClass(mxLOGICAL_CLASS) {	}

class MxStringClass : public MxClass {
public:
	MxStringClass()
				: MxClass(mxCHAR_CLASS) {	}
};

class MxCellClass : public MxClass {
public:
	MxCellClass()
			: MxClass(mxCELL_CLASS) {	}
};

class MxStructClass : public MxClass {
public:
	MxStructClass()
				: MxClass(mxSTRUCT_CLASS) {	}
};

namespace {
typedef mxArray MxArrayNative;
typedef MxArrayNative* PMxArrayNative;
} /* namespace */

class MxArray {
public:
	MxArray()
		: m_array(),
		  m_class() {	}

	/*
	 * Only copies pointer, does not do "deep copy".
	 */
	MxArray(const MxArray& other)
		: m_array(other.m_array),
		  m_class() {	}

	/*
	 * Only copies pointer, does not do "deep copy".
	 */
	MxArray& operator=(const MxArray& other) {
		if (this != &other) {
			this->m_array = other.m_array;
		}
		return *this;
	}

	/*
	 * Only copies pointer, does not do "deep copy".
	 */
	explicit MxArray(const PMxArrayNative array)
					: m_array(array),
					  m_class() {	}

	virtual ~MxArray() {	}

	inline const PMxArrayNative get_array() const {
		return m_array;
	}

	inline PMxArrayNative get_array() {
		return m_array;
	}

	inline const MxClass& get_class() const {
		return m_class;
	}

	template <typename IndexType>
	inline IndexType getNumberOfElements() const {
		return static_cast<IndexType>(mxGetNumberOfElements(m_array));
	}

	inline int getNumberOfElements() const {
		return getNumberOfElements<int>();
	}

	template <typename IndexType>
	inline IndexType size() const {
		return getNumberOfElements<IndexType>();
	}

	inline int size() const {
		return size<int>();
	}

	template <typename IndexType>
	inline IndexType getNumberOfRows() const {
		return static_cast<IndexType>(mxGetM(m_array));
	}

	inline int getNumberOfRows() const {
		return getNumberOfRows<int>();
	}

	template <typename IndexType>
	inline IndexType getNumberOfColumns() const {
		return static_cast<IndexType>(mxGetN(m_array));
	}

	inline int getNumberOfColumns() const {
		return getNumberOfColumns<int>();
	}

	template <typename IndexType>
	inline IndexType getNumberOfDimensions() const {
		return static_cast<IndexType>(mxGetNumberOfDimensions(m_array));
	}

	inline int getNumberOfDimensions() const {
		return getNumberOfDimensions<int>();
	}

	template <typename IndexType>
	inline std::vector<IndexType> getDimensions() const {
		const size_t numberOfDimensions = getNumberOfDimensions<size_t>();
		const mwSize* tempDims = mxGetDimensions(m_array);
		return std::vector<IndexType>(tempDims, tempDims + numberOfDimensions);
	}

	inline std::vector<int> getDimensions() const {
		return getDimensions<int>();
	}

	inline bool isEmpty() const {
		return mxIsEmpty(m_array);
	}

	template <typename NumericType>
	inline bool is() const {
		return m_class == MxNumericClass<NumericType>();
	}

	inline bool isString() const {
		return m_class == MxStringClass();
	}

	inline bool isCell() const {
		return m_class == MxCellClass();
	}

	inline bool isStruct() const {
		return m_class == MxStructClass();
	}

	inline void destroy() {
		mxDestroyArray(m_array);
	}

protected:
	explicit MxArray(const MxClass& mxClass)
					: m_array(),
					  m_class(mxClass) {	}

	MxArray(const PMxArrayNative array, const MxClass& mxClass)
			: m_array(array),
			  m_class(mxClass) {	}

	inline mxClassID getInternalClass() const {
		return mxGetClassID(m_array);
	}

	PMxArrayNative m_array;
	MxClass m_class;
};

namespace {
	typedef MxArray* PMxArray;
} /* namespace */

template <typename NumericType>
class MxNumeric : public MxArray {
public:
	MxNumeric()
			: MxArray(MxNumericClass<NumericType>()) {	}

	/*
	 * TODO: Check if this allows copy-construction from base or other MxArrays.
	 */
	MxNumeric(const MxNumeric<NumericType>& mxNumeric)
			: MxArray(mxNumeric.get_array(),
					MxNumericClass<NumericType>()) {	}

	MxNumeric<NumericType>& operator=(const MxNumeric<NumericType>& mxNumeric) {
		if (this != &mxNumeric) {
			this->m_array = mxNumeric.m_array;
			this->m_class = mxNumeric.m_class;
		}
		return *this;
	}

	explicit MxNumeric(const PMxArrayNative array)
					: MxArray(array, MxNumericClass<NumericType>()) {
		mexAssert(m_class.get_classId() == mxGetClassID(array));
	}

	template <typename IndexType>
	MxNumeric(const IndexType numRows, const IndexType numColumns)
			: MxArray(mxCreateNumericMatrix(static_cast<mwSize>(numRows),
											static_cast<mwSize>(numColumns),
											MxNumericClass<NumericType>()
												.get_classId(),
											mxREAL),
					MxNumericClass<NumericType>()) {	}

	template <typename IndexType>
	MxNumeric(const IndexType numDims, const IndexType *dims)
			: MxArray(mxCreateNumericArray(static_cast<mwSize>(numDims),
										&(std::vector<mwSize>(dims,
															dims + numDims)[0]),
										MxNumericClass<NumericType>()
											.get_classId(),
										mxREAL),
					MxNumericClass<NumericType>()) {	}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar,
			const IndexType numRows,
			const IndexType numColumns)
			: MxArray(mxCreateNumericMatrix(static_cast<mwSize>(numRows),
											static_cast<mwSize>(numColumns),
											MxNumericClass<NumericType>()
												.get_classId(),
											mxREAL),
					MxNumericClass<NumericType>()) {
		init(arrVar);
	}

	explicit MxNumeric(const NumericType scalarVar)
					: MxArray(mxCreateNumericMatrix(static_cast<mwSize>(1),
												static_cast<mwSize>(1),
												MxNumericClass<NumericType>()
													.get_classId(),
												mxREAL),
							MxNumericClass<NumericType>()) {
		init(&scalarVar);
	}

	explicit MxNumeric(const std::vector<NumericType>& vecVar)
					: MxArray(mxCreateNumericMatrix(static_cast<mwSize>(
																vecVar.size()),
												static_cast<mwSize>(1),
												MxNumericClass<NumericType>()
													.get_classId(),
												mxREAL),
							MxNumericClass<NumericType>()) {
		const NumericType temp = vecVar[0];
		init(&temp);
	}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar,
			const IndexType numDims,
			const IndexType *dims)
			: MxArray(mxCreateNumericArray(static_cast<mwSize>(numDims),
										&(std::vector<mwSize>(dims,
															dims + numDims)[0]),
										MxNumericClass<NumericType>()
											.get_classId(),
										mxREAL),
					MxNumericClass<NumericType>()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar,
			const std::vector<IndexType>& dims)
			: MxArray(mxCreateNumericArray(static_cast<mwSize>(dims.size()),
										&(std::vector<mwSize>(dims.begin(),
															dims.end())[0]),
										MxNumericClass<NumericType>()
											.get_classId(),
										mxREAL),
					MxNumericClass<NumericType>()) {
		init(arrVar);
	}

	template <typename IndexType>
	inline NumericType& operator[] (IndexType i) {
		mexAssert(i < getNumberOfElements<IndexType>());
		NumericType* temp = static_cast<NumericType*>(mxGetData(m_array));
		return temp[i];
	}

	template <typename IndexType>
	inline const NumericType& operator[] (IndexType i) const {
		mexAssert(i < getNumberOfElements<IndexType>());
		NumericType* temp = static_cast<NumericType*>(mxGetData(m_array));
		return temp[i];
	}

	inline const NumericType* getData() const {
		return static_cast<const NumericType*>(mxGetData(m_array));
	}

	inline NumericType* getData() {
		return static_cast<NumericType*>(mxGetData(m_array));
	}

	inline std::vector<NumericType> vectorize() const {
		int numel = getNumberOfElements();
		const NumericType* pData = getData();
		return std::vector<NumericType>(pData, pData + numel);
	}

	template <typename IndexType>
	inline std::vector<IndexType> ind2sub(IndexType index) const {
		mexAssert(index < getNumberOfElements<IndexType>());
		std::vector<IndexType> dimensions = getDimensions<IndexType>();
		std::vector<IndexType> subscript(getNumberOfDimensions<size_t>());
		IndexType sliceSize = getNumberOfElements<IndexType>();
		for (int iter = 0, end = getNumberOfDimensions(); iter < end; ++iter) {
			sliceSize /= dimensions[end - 1 - iter];
			subscript[end - 1 - iter] = index / sliceSize;
			index %= sliceSize;
		}
		return subscript;
	}

	template <typename IndexType>
	inline IndexType sub2ind(const std::vector<IndexType>& subscript) const {
		std::vector<IndexType> dimensions = getDimensions<IndexType>();
		mexAssert(subscript.size() == getNumberOfDimensions<size_t>());
		for (int iter = 0, end = getNumberOfDimensions();
			iter < end;
			++iter) {
			mexAssert(subscript[iter] < dimensions[iter]);
		}
		IndexType index = 0;
		IndexType sliceSize = getNumberOfElements<IndexType>();
		for (int iter = 0, end = getNumberOfDimensions(); iter < end; ++iter) {
			sliceSize /= dimensions[end - 1 - iter];
			index += subscript[end - 1 - iter] * sliceSize;
		}
		return index;
	}

	/*
	 * TODO: Very cache-unfriendly and out-of-place multidimensional permute.
	 */
	template <typename IndexType>
	MxNumeric<NumericType> permute(
								const std::vector<IndexType>& indexPermutation)
								const {
		mexAssert(isIndexPermutation(indexPermutation));
		const std::vector<IndexType> dimensions = getDimensions<IndexType>();
		const std::vector<IndexType> permutedDimensions = permuteIndexVector(
															dimensions,
															indexPermutation);
		MxNumeric<NumericType> retArg(static_cast<IndexType>(
													permutedDimensions.size()),
									&permutedDimensions[0]);
		const NumericType* thisData = getData();
		NumericType* otherData = retArg.getData();
		for (IndexType iter = 0,
			end = static_cast<IndexType>(getNumberOfElements());
			iter < end;
			++iter) {
			otherData[retArg.sub2ind(permuteIndexVector(ind2sub(iter),
														indexPermutation))]
			          	  	  	  	  	  	  	  	  	= thisData[iter];
		}
		return retArg;
	}

private:
	void init(const NumericType* arrVar) {
		NumericType *val = static_cast<NumericType*>(mxGetData(m_array));
		memcpy(static_cast<void*>(val), static_cast<const void*>(arrVar),
			getNumberOfElements<size_t>() * sizeof(NumericType));
	}

	template <typename IndexType>
	inline bool isIndexPermutation(
								const std::vector<IndexType>& indexPermutation)
								const {
		if (indexPermutation.size() != getNumberOfDimensions<size_t>()) {
			return false;
		}
		std::vector<IndexType> indexVector;
		for (IndexType iter = 0,
			end = getNumberOfDimensions<IndexType>();
			iter < end;
			++iter) {
			indexVector.push_back(iter + 1);
		}
		typename std::vector<IndexType>::const_iterator first(indexPermutation.begin());
		typename std::vector<IndexType>::const_iterator last(indexPermutation.end());
		typename std::vector<IndexType>::iterator d_first(indexVector.begin());
		if (first != last) {
			typename std::vector<IndexType>::iterator d_last = d_first;
			std::advance(d_last, std::distance(first, last));
			for (typename std::vector<IndexType>::const_iterator i = first;
				i != last;
				++i) {
				if (i != std::find(first, i, *i)) {
					continue; // already counted this *i
				}

				IndexType m = std::count(d_first, d_last, *i);
				if (m == 0 || std::count(i, last, *i) != m) {
					return false;
				}
			}
		}
		return true;
	}

	template <typename IndexType>
	inline std::vector<IndexType> permuteIndexVector(
							const std::vector<IndexType>& indexVector,
							const std::vector<IndexType>& indexPermutation)
							const {
		std::vector<IndexType> permutedIndexVector(indexVector.size(), 0);
		for (IndexType iter = 0, end = indexVector.size(); iter < end; ++iter) {
			permutedIndexVector[iter] = indexVector[indexPermutation[iter] - 1];
		}
		return permutedIndexVector;
	}
};

/*
 * TODO: For memory safety reasons, and because of MATLAB's internal
 * representation for "string-arrays", this is relatively memory inefficient.
 */
class MxString: public MxArray {
public:
	MxString(const MxString& mxString)
			: MxArray(mxString.get_array(), MxStringClass()),
			  m_string(mxString.get_string()) {	}

	MxString& operator=(const MxString& mxString) {
		if (this != &mxString) {
			this->m_array = mxString.m_array;
			this->m_class = mxString.m_class;
			this->m_string = mxString.m_string;
		}
		return *this;
	}

	explicit MxString(const PMxArrayNative array)
					: MxArray(array, MxStringClass()),
					  m_string() {
		mexAssert(m_class.get_classId() == mxGetClassID(array));
		char* temp = mxArrayToString(m_array);
		m_string = std::string(temp);
		mxFree(temp);
	}

	MxString(const std::string& string)
			: MxArray(mxCreateString(string.c_str()), MxStringClass()),
			  m_string(string) {	}

	explicit MxString(const char* cString)
					: MxArray(mxCreateString(cString), MxStringClass()),
					  m_string(cString) {	}

	inline void clone(const MxString& mxString) {
		mexAssert(getDimensions() == mxString.getDimensions());
		mxChar *destination = static_cast<mxChar*>(mxGetData(m_array));
		const mxChar *origin = static_cast<const mxChar*>(
											mxGetData(mxString.get_array()));
		memcpy(static_cast<void*>(destination),
			static_cast<const void*>(origin),
			getNumberOfElements<size_t>() * sizeof(mxChar));
		m_string = mxString.get_string();
	}

	inline const std::string& get_string() const {
		return m_string;
	}

	inline const char* c_str() const {
		return m_string.c_str();
	}

	template <typename IndexType>
	inline IndexType length() const {
		return getNumberOfElements<IndexType>();
	}

	inline int length() const {
		return getNumberOfElements<int>();
	}

private:
	std::string m_string;
};

class MxCell : public MxArray {
public:
	MxCell(const MxCell& mxCell)
		: MxArray(mxCell.get_array(), MxCellClass()) {	}

	MxCell& operator=(const MxCell& mxCell) {
		if (this != &mxCell) {
			this->m_array = mxCell.m_array;
			this->m_class = mxCell.m_class;
		}
		return *this;
	}

	explicit MxCell(const PMxArrayNative array)
				: MxArray((mxGetClassID(array) == MxCellClass().get_classId())
						?(array)
						:(mxCreateCellMatrix(static_cast<mwSize>(1),
											static_cast<mwSize>(1))),
						MxCellClass()) {
		if (m_class.get_classId() != mxGetClassID(array)) {
			init(&array);
		}
	}

	template <typename IndexType>
	MxCell(const IndexType numRows, const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {	}

	template <typename IndexType>
	MxCell(const IndexType numDims, const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {	}

	template <typename IndexType>
	MxCell(const PMxArrayNative* arrVar,
		const IndexType numRows,
		const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const PMxArray* arrVar,
		const IndexType numRows,
		const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {
		init(arrVar);
	}

	explicit MxCell(const std::vector<PMxArrayNative>& vecVar)
				: MxArray(mxCreateCellMatrix(static_cast<mwSize>(vecVar.size()),
											static_cast<mwSize>(1)),
						MxCellClass()) {
		init(&vecVar[0]);
	}

	/*
	 * TODO: Remember to change this if I add constructor from MxArray to all
	 * derived classes of MxArray.
	 */
	explicit MxCell(const PMxArray scalarVar)
				: MxArray(mxCreateCellMatrix(static_cast<mwSize>(1),
											static_cast<mwSize>(1)),
						MxCellClass()) {
		init(&scalarVar);
	}

	template <typename IndexType>
	MxCell(const PMxArrayNative* arrVar,
		const IndexType numDims,
		const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const PMxArray* arrVar,
		const IndexType numDims,
		const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const PMxArrayNative* arrVar, const std::vector<IndexType>& dims)
		: MxArray(mxCreateCellArray(static_cast<IndexType>(dims.size()),
									&dims[0]),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const PMxArray* arrVar, const std::vector<IndexType>& dims)
		: MxArray(mxCreateCellArray(static_cast<IndexType>(dims.size()),
									&dims[0]),
				MxCellClass()) {
		init(arrVar);
	}

	/*
	 * TODO: The following field access operators (getData, [], and vectorize)
	 * require some thought for the const case. Should I make the pointers const
	 * too?
	 */
	template <typename IndexType>
	inline PMxArrayNative operator[] (IndexType i) {
		mexAssert(i < getNumberOfElements<IndexType>());
		return mxGetCell(m_array, i);
	}

	template <typename IndexType>
	inline const PMxArrayNative operator[] (IndexType i) const {
		mexAssert(i < getNumberOfElements<IndexType>());
		return mxGetCell(m_array, i);
	}

	inline const PMxArrayNative* getData() const {
		return static_cast<const PMxArrayNative*>(mxGetData(m_array));
	}

	inline PMxArrayNative* getData() {
		return static_cast<PMxArrayNative*>(mxGetData(m_array));
	}

	/*
	 * TODO: Template this, and add version for returning PMxArray, with default
	 * being as is now.
	 */
	inline const std::vector<PMxArrayNative> vectorize() const {
		std::vector<PMxArrayNative> retArg;
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			retArg.push_back((mxGetCell(m_array, iter)));
		}
		return retArg;
	}

	inline std::vector<PMxArrayNative> vectorize() {
		std::vector<PMxArrayNative> retArg;
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			retArg.push_back((mxGetCell(m_array, iter)));
		}
		return retArg;
	}

private:
	void init(const PMxArrayNative* arrVar) {
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			mxSetCell(m_array, iter, arrVar[iter]);
		}
	}

	void init(const PMxArray* arrVar) {
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			mxSetCell(m_array, iter, arrVar[iter]->get_array());
		}
	}
};

class MxStruct : public MxArray {
public:
	MxStruct(const MxStruct& mxStruct)
			: MxArray(mxStruct.get_array(), MxStructClass()) {	}

	MxStruct& operator=(const MxStruct& mxStruct) {
		if (this != &mxStruct) {
			this->m_array = mxStruct.m_array;
			this->m_class = mxStruct.m_class;
		}
		return *this;
	}

	explicit MxStruct(const PMxArrayNative array)
					: MxArray(array, MxStructClass()) {
		mexAssert(m_class.get_classId() == mxGetClassID(array));
	}

	/*
	 * TODO: Check that mxCreateStructMatrix can be called with NULL in name
	 * fields.
	 */
	MxStruct(const std::string& scalarName, const PMxArrayNative scalarVar)
			: MxArray(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										NULL),
					MxStructClass()) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	MxStruct(const std::string& scalarName, const PMxArray scalarVar)
	: MxArray(mxCreateStructMatrix(static_cast<mwSize>(1),
								static_cast<mwSize>(1),
								static_cast<mwSize>(1),
								NULL),
			MxStructClass()) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	MxStruct(const std::vector<std::string>& vecName,
			const std::vector<PMxArrayNative>& vecVar)
			: MxArray((vecName.size() == vecVar.size())
					?(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										NULL))
					:(NULL),
					MxStructClass()) {
		mexAssert(m_array != NULL);
		addField_sub(&vecName[0],
					&vecVar[0],
					static_cast<mwSize>(vecName.size()));
	}

	MxStruct(const std::vector<std::string>& vecName,
			const std::vector<PMxArray>& vecVar)
			: MxArray((vecName.size() == vecVar.size())
					?(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										NULL))
					:(NULL),
					MxStructClass()) {
		mexAssert(m_array != NULL);
		addField_sub(&vecName[0],
					&vecVar[0],
					static_cast<mwSize>(vecName.size()));
	}

	void addField(const std::string& scalarName,
				const PMxArrayNative scalarVar) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	void addField(const std::string& scalarName,
				const PMxArray scalarVar) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	void addField(const std::vector<std::string>& vecName,
				const std::vector<PMxArrayNative>& vecVar) {
		mexAssert(vecVar.size() == vecName.size());
		addField_sub(&vecName[0], &vecVar[0], vecVar.size());
	}

	void addField(const std::vector<std::string>& vecName,
				const std::vector<PMxArray>& vecVar) {
		mexAssert(vecVar.size() == vecName.size());
		addField_sub(&vecName[0], &vecVar[0], vecVar.size());
	}

	inline bool isField(const std::string& name) const {
		return (mxGetFieldNumber(m_array, name.c_str()) != -1);
	}

	template <typename IndexType>
	inline IndexType getNumberOfFields() const {
		return static_cast<IndexType>(mxGetNumberOfFields(m_array));
	}

	inline int getNumberOfFields() const {
		return getNumberOfFields<int>();
	}

	template <typename IndexType>
	inline IndexType getFieldNumber(const std::string& name) const {
		return static_cast<IndexType>(mxGetFieldNumber(m_array, name.c_str()));
	}

	inline int getFieldNumber(const std::string& name) const {
		return getFieldNumber<int>(name);
	}

	/*
	 * TODO: Check that here mxGetFieldNameByNumber does no allocation and
	 * copy, so no dealloc is needed.
	 */
	template <typename IndexType>
	inline std::string getFieldName(const IndexType i) const {
		return std::string(mxGetFieldNameByNumber(m_array,
												static_cast<int>(i)));
	}

	inline std::vector<std::string> getFieldNames() const {
		std::vector<std::string> retArg(0);
		for (int iter = 0, end = getNumberOfFields(); iter < end; ++iter) {
			retArg.push_back(std::string(mxGetFieldNameByNumber(m_array, iter)));
		}
		return retArg;
	}

	/*
	 * TODO: The following field access operators (getData, [], and vectorize)
	 * require some thought for the const case. Should I make the pointers const
	 * too?
	 */
	template <typename IndexType>
	inline PMxArrayNative operator[] (IndexType i) {
		mexAssert(i < getNumberOfElements());
		return mxGetFieldByNumber(m_array, 0, int(i));
	}

	template <typename IndexType>
	inline const PMxArrayNative operator[] (IndexType i) const {
		return mxGetFieldByNumber(m_array, 0, int(i));
	}

	inline PMxArrayNative operator[] (const std::string& name) {
		return mxGetField(m_array, 0, name.c_str());
	}

	inline const PMxArrayNative operator[] (const std::string& name) const {
		return mxGetField(m_array, 0, name.c_str());
	}

	inline const PMxArrayNative* getData() const {
		return (PMxArrayNative *) mxGetData(m_array);
	}

	inline PMxArrayNative* getData() {
		return (PMxArrayNative *) mxGetData(m_array);
	}

	inline void vectorize(std::vector<std::string>& vecName,
						std::vector<PMxArrayNative>& vecVar ) const {
		vecName.resize(0);
		vecVar.resize(0);
		for (int iter = 0, end = getNumberOfFields(); iter < end; ++iter) {
			const char* tempName = mxGetFieldNameByNumber(m_array, iter);
			vecName.push_back(std::string(tempName));
			PMxArrayNative tempVar = mxGetFieldByNumber(m_array, 0, iter);
			vecVar.push_back(tempVar);
		}
	}

private:
	static const size_t kMxMaxNameLength = mxMAXNAM - 1;

	template <typename IndexType>
	void addField_sub(const std::string* arrName,
				const PMxArrayNative* arrVar,
				IndexType numFields) {
		for (IndexType iter = 0; iter < numFields; ++iter) {
			mexAssert(arrName[iter].size() <= kMxMaxNameLength);
			mxAddField(m_array, arrName[iter].c_str());
			mxSetField(m_array, 0, arrName[iter].c_str(), arrVar[iter]);
		}
	}

	template <typename IndexType>
	void addField_sub(const std::string* arrName,
				const PMxArray* arrVar,
				IndexType numFields) {
		for (IndexType iter = 0; iter < numFields; ++iter) {
			mexAssert(arrName[iter].size() <= kMxMaxNameLength);
			mxAddField(m_array, arrName[iter].c_str());
			mxSetField(m_array, 0,
					arrName[iter].c_str(),
					arrVar[iter]->get_array());
		}
	}
};

} /* name space mex */

#endif /* MEX_UTILS_H_ */
