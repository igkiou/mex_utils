/*
 * mex_utils.h
 *
 *  Created on: Jan 30, 2013
 *      Author: igkiou
 */

#ifndef MEX_UTILS_H_
#define MEX_UTILS_H_

#include <algorithm>
#include <array>
#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <cstring>

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
 * TODO: Add support for initialization by const mxArray*. Probably will need to
 * create const_MxArray class and class hierarchy.
 * TODO: Is giving access to a data element with [] in MxCell and MxStruct
 * equivalent to using mxSetCell and mxSetField, respectively?
 * TODO: Add general matrix views to avoid permute when possible?
 * TODO: Maybe add support for mxSetData? Would be unsafe.
 * TODO: Find way to do safe resource management when a malloc occurs
 * (construction is not done by an already existing PMxArrayNative). Maybe using
 * some smart pointer, or using onwer member.
 * TODO: Update to take advantage of C++11 move semantics, especially for better
 * safety.
 * TODO: Maybe replace default implementations with copy-and-swap?
 * TODO: Provide iterators for easy use with STL and new C++11 loops.
 * TODO: Maybe provide StringCell specialization.
 * TODO: Change MxNumeric template definition to prevent instantiations by
 * non-appropriate types.
 * TODO: Remove unsafe conversions for MxClass.
 * TODO: Make ID in MxClass directly accessible.
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

class MxClass {
public:
	MxClass() :
			m_classId(mxUNKNOWN_CLASS) {}

	MxClass(const MxClass& other) = default;

	MxClass& operator=(const MxClass& other) = default;

	MxClass(MxClass&& other) = default;
	MxClass& operator=(MxClass&& other) = default;

	friend void swap(MxClass& first, MxClass& second) {
		using std::swap;
		swap(first.m_classId, second.m_classId);
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

	virtual ~MxClass() = default;

protected:
	explicit MxClass(const mxClassID mxClass)
		: m_classId(mxClass) {}

private:
	mxClassID m_classId;
};

template <typename NumericType>
class MxNumericClass : public MxClass {
public:
	MxNumericClass()
				: MxClass(mxUNKNOWN_CLASS) {}
};

/*
 * Specialization for MATLAB declared types. On Linux x86_64, the
 * correspondences are:
 * 		UINT8_T		-	unsigned char
 * 		INT8_T		-	char
 * 		INT16_T		-	short
 * 		UINT16_T	-	unsigned short
 * 		INT32_T		-	int
 * 		UINT32_T	-	unsigned int
 * 		INT64_T		-	long
 * 		UINT64_T	-	unsigned long
 * 		single		-	float
 * 		double		-	double
 * 		mxLogical	-	bool
 */
template <> inline MxNumericClass<UINT8_T>::MxNumericClass()
											: MxClass(mxUINT8_CLASS) {}
template <> inline MxNumericClass<INT8_T>::MxNumericClass()
											: MxClass(mxINT8_CLASS) {}
template <> inline MxNumericClass<INT16_T>::MxNumericClass()
											: MxClass(mxINT16_CLASS) {}
template <> inline MxNumericClass<UINT16_T>::MxNumericClass()
											: MxClass(mxUINT16_CLASS) {}
template <> inline MxNumericClass<INT32_T>::MxNumericClass()
											: MxClass(mxINT32_CLASS) {}
template <> inline MxNumericClass<UINT32_T>::MxNumericClass()
											: MxClass(mxUINT32_CLASS) {}
template <> inline MxNumericClass<INT64_T>::MxNumericClass()
											: MxClass(mxINT64_CLASS) {}
template <> inline MxNumericClass<UINT64_T>::MxNumericClass()
											: MxClass(mxUINT64_CLASS) {}
template <> inline MxNumericClass<float>::MxNumericClass()
											: MxClass(mxSINGLE_CLASS) {}
template <> inline MxNumericClass<double>::MxNumericClass()
											: MxClass(mxDOUBLE_CLASS) {}
template <> inline MxNumericClass<mxLogical>::MxNumericClass()
											: MxClass(mxLOGICAL_CLASS) {}

class MxStringClass : public MxClass {
public:
	MxStringClass()
				: MxClass(mxCHAR_CLASS) {}
};

class MxCellClass : public MxClass {
public:
	MxCellClass()
			: MxClass(mxCELL_CLASS) {}
};

class MxStructClass : public MxClass {
public:
	MxStructClass()
				: MxClass(mxSTRUCT_CLASS) {}
};

namespace detail {
using MxArrayNative = mxArray;
using PMxArrayNative = MxArrayNative* ;
}  // namespace detail

class MxArray {
public:
	MxArray() = default;

	/*
	 * The following only copy pointer, and do not perform "deep copy". Also,
	 * care must be taken when using copy constructor and assignment, to avoid
	 * slicing of derived classes.
	 */
	MxArray(const MxArray& other) = default;
	MxArray& operator=(const MxArray& other) = default;

	MxArray(MxArray&& other) = default;
	MxArray& operator=(MxArray&& other) = default;

	friend void swap(MxArray& first, MxArray& second) {
		using std::swap;
		swap(first.m_array, second.m_array);
		swap(first.m_class, second.m_class);
	}

	/*
	 * Only copies pointer, does not do "deep copy".
	 */
	explicit MxArray(const detail::PMxArrayNative array) :
			m_array(array),
			m_class() {}

	inline detail::PMxArrayNative get_array() const {
		return m_array;
	}

	inline detail::PMxArrayNative get_array() {
		return m_array;
	}

	inline const MxClass& get_class() const {
		return m_class;
	}

	template <typename IndexType>
	inline IndexType getNumberOfElements() const {
		return static_cast<IndexType>(mxGetNumberOfElements(get_array()));
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
		return static_cast<IndexType>(mxGetM(get_array()));
	}

	inline int getNumberOfRows() const {
		return getNumberOfRows<int>();
	}

	template <typename IndexType>
	inline IndexType getNumberOfColumns() const {
		return static_cast<IndexType>(mxGetN(get_array()));
	}

	inline int getNumberOfColumns() const {
		return getNumberOfColumns<int>();
	}

	template <typename IndexType>
	inline IndexType getNumberOfDimensions() const {
		return static_cast<IndexType>(mxGetNumberOfDimensions(get_array()));
	}

	inline int getNumberOfDimensions() const {
		return getNumberOfDimensions<int>();
	}

	template <typename IndexType>
	inline std::vector<IndexType> getDimensions() const {
		const size_t numberOfDimensions = getNumberOfDimensions<size_t>();
		const mwSize* tempDims = mxGetDimensions(get_array());
		return std::vector<IndexType>(tempDims, tempDims + numberOfDimensions);
	}

	inline std::vector<int> getDimensions() const {
		return getDimensions<int>();
	}

	inline bool isEmpty() const {
		return mxIsEmpty(get_array());
	}

	template <typename NumericType>
	inline bool isNumeric() const {
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
		mxDestroyArray(get_array());
	}

	virtual ~MxArray() = default;

protected:
	explicit MxArray(const MxClass& mxClass)
					: m_array(),
					  m_class(mxClass) {}

	MxArray(const detail::PMxArrayNative array, const MxClass& mxClass) :
			m_array(array),
			m_class(mxClass) {}

	inline mxClassID getInternalClass() const {
		return mxGetClassID(get_array());
	}

private:
	detail::PMxArrayNative m_array;
	MxClass m_class;
};

namespace detail {
using PMxArray = MxArray*;
using array2D = std::array<mwSize, 2>;
}  // namespace detail


template <typename NumericType>
class MxNumeric : public MxArray {
public:
	MxNumeric() :
			MxArray(MxNumericClass<NumericType>()) {}

	MxNumeric(const MxNumeric<NumericType>& other) = default;

	MxNumeric<NumericType>& operator=(const MxNumeric<NumericType>& other)
																	= default;

	MxNumeric(MxNumeric<NumericType>&& other) = default;
	MxNumeric<NumericType>& operator=(MxNumeric<NumericType>&& other) = default;

	explicit MxNumeric(const detail::PMxArrayNative array) :
			MxArray(array, MxNumericClass<NumericType>()) {
		mexAssert(get_class().get_classId() == mxGetClassID(array));
	}

	template <typename IndexType>
	MxNumeric(const IndexType numDims, const IndexType *dims) :
			MxNumeric(mxCreateNumericArray(static_cast<mwSize>(numDims),
										std::vector<mwSize>(dims,
															dims + numDims)
											.data(),
										MxNumericClass<NumericType>()
											.get_classId(),
										mxREAL)) {}

//	template <>
//	MxNumeric(const mwSize numDims, const mwSize *dims) :
//			MxNumeric(mxCreateNumericArray(numDims, dims,
//										MxNumericClass<NumericType>()
//											.get_classId(),
//										mxREAL)) {}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar,
			const IndexType numDims,
			const IndexType *dims)
			: MxNumeric(numDims, dims) {
		init(arrVar);
	}

	template <typename IndexType>
	MxNumeric(const IndexType numRows, const IndexType numColumns) :
			MxNumeric(static_cast<mwSize>(2),
					detail::array2D{numRows, numColumns}.data()) {}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar, const IndexType numRows,
			const IndexType numColumns) :
			MxNumeric(arrVar, static_cast<mwSize>(2),
					detail::array2D{numRows, numColumns}.data()) {}

	explicit MxNumeric(const NumericType scalarVar) :
			MxNumeric(&scalarVar, static_cast<mwSize>(2),
					detail::array2D{1, 1}.data()) {}

	explicit MxNumeric(const std::vector<NumericType>& vecVar) :
			MxNumeric(vecVar.data(), static_cast<mwSize>(2),
					detail::array2D{vecVar.size(), 1}.data()) {}

	template <typename IndexType>
	MxNumeric(const NumericType* arrVar, const std::vector<IndexType>& dims) :
			MxNumeric(arrVar, dims.size(), dims.data()) {}

	template <typename IndexType>
	inline NumericType& operator[](IndexType i) {
		mexAssert(i < getNumberOfElements<IndexType>());
		NumericType* temp = static_cast<NumericType*>(mxGetData(get_array()));
		return temp[i];
	}

	template <typename IndexType>
	inline const NumericType& operator[](IndexType i) const {
		mexAssert(i < getNumberOfElements<IndexType>());
		NumericType* temp = static_cast<NumericType*>(mxGetData(get_array()));
		return temp[i];
	}

	inline const NumericType* getData() const {
		return static_cast<const NumericType*>(mxGetData(get_array()));
	}

	inline NumericType* getData() {
		return static_cast<NumericType*>(mxGetData(get_array()));
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

	virtual ~MxNumeric() = default;

private:
	void init(const NumericType* arrVar) {
		NumericType *val = static_cast<NumericType*>(mxGetData(get_array()));
		std::memcpy(static_cast<void*>(val), static_cast<const void*>(arrVar),
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
		typename std::vector<IndexType>::const_iterator first(
													indexPermutation.begin());
		typename std::vector<IndexType>::const_iterator last(
													indexPermutation.end());
		typename std::vector<IndexType>::iterator d_first(indexVector.begin());
		if (first != last) {
			typename std::vector<IndexType>::iterator d_last = d_first;
			std::advance(d_last, std::distance(first, last));
			for (typename std::vector<IndexType>::const_iterator i = first;
				i != last;
				++i) {
				if (i != std::find(first, i, *i)) {
					continue;  // already counted this *i
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

	MxString() :
			MxArray(MxStringClass()),
			m_string() {}

	MxString(const MxString& other) = default;
//	MxString(const MxString& other)
//			: MxArray(other.get_array(), MxStringClass()),
//			  m_string(other.get_string()) {}

	MxString& operator=(const MxString& other) = default;
//	MxString& operator=(const MxString& other) {
//		if (this != &other) {
//			this->m_array = other.m_array;
//			this->m_class = other.m_class;
//			this->m_string = other.m_string;
//		}
//		return *this;
//	}

	MxString(MxString&& other) = default;
	MxString& operator=(MxString&& other) = default;

	friend void swap(MxString& first, MxString& second) {
		using std::swap;
		swap(static_cast<MxArray&>(first), static_cast<MxArray&>(second));
		swap(first.m_string, second.m_string);
	}

	explicit MxString(const detail::PMxArrayNative array)
					: MxArray(array, MxStringClass()),
					  m_string() {
		mexAssert(get_class().get_classId() == mxGetClassID(array));
		char* temp = mxArrayToString(get_array());
		m_string = std::string(temp);
		mxFree(temp);
	}

	explicit MxString(const std::string& string)
					: MxArray(mxCreateString(string.c_str()), MxStringClass()),
					  m_string(string) {}

	explicit MxString(const char* cString)
					: MxArray(mxCreateString(cString), MxStringClass()),
					  m_string(cString) {}

	inline void clone(const MxString& other) {
		mexAssert(getDimensions() == other.getDimensions());
		mxChar *destination = static_cast<mxChar*>(mxGetData(get_array()));
		const mxChar *origin = static_cast<const mxChar*>(
											mxGetData(other.get_array()));
		std::memcpy(static_cast<void*>(destination),
			static_cast<const void*>(origin),
			getNumberOfElements<size_t>() * sizeof(mxChar));
		m_string = other.get_string();
	}

	inline const std::string& get_string() const {
		return m_string;
	}

	inline const char* c_str() const {
		return m_string.c_str();
	}

	template <typename IndexType>
	inline char& operator[](IndexType i) {
		mexAssert(i < length<IndexType>());
		return m_string[i];
	}

	template <typename IndexType>
	inline const char* operator[](IndexType i) const {
		mexAssert(i < length<IndexType>());
		return m_string[i];
	}

	template <typename IndexType>
	inline IndexType length() const {
		return getNumberOfElements<IndexType>();
	}

	inline int length() const {
		return getNumberOfElements<int>();
	}

	virtual ~MxString() = default;

private:
	std::string m_string;
};

class MxCell : public MxArray {
public:

	MxCell() :
			MxArray(MxCellClass()) {}

	MxCell(const MxCell& other) = default;
//	MxCell(const MxCell& other)
//		: MxArray(other.get_array(), MxCellClass()) {}

	MxCell& operator=(const MxCell& other) = default;
//	MxCell& operator=(const MxCell& other) {
//		if (this != &other) {
//			this->m_array = other.m_array;
//			this->m_class = other.m_class;
//		}
//		return *this;
//	}

	MxCell(MxCell&& other) = default;
	MxCell& operator=(MxCell&& other) = default;

	/*
	 * TODO: Because of this, initialization from scalar PMxArrayNative is
	 * disabled.
	 */
	explicit MxCell(const detail::PMxArrayNative array)
					: MxArray(array, MxCellClass()) {
		mexAssert(get_class().get_classId() == mxGetClassID(array));
	}

	template <typename IndexType>
	MxCell(const IndexType numRows, const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {}

	template <typename IndexType>
	MxCell(const IndexType numDims, const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {}

	template <typename IndexType>
	MxCell(const detail::PMxArrayNative* arrVar,
		const IndexType numRows,
		const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const detail::PMxArray* arrVar,
		const IndexType numRows,
		const IndexType numColumns)
		: MxArray(mxCreateCellMatrix(static_cast<mwSize>(numRows),
									static_cast<mwSize>(numColumns)),
				MxCellClass()) {
		init(arrVar);
	}

	explicit MxCell(const std::vector<detail::PMxArrayNative>& vecVar)
				: MxArray(mxCreateCellMatrix(static_cast<mwSize>(vecVar.size()),
											static_cast<mwSize>(1)),
						MxCellClass()) {
		init(&vecVar[0]);
	}

	explicit MxCell(const std::vector<detail::PMxArray>& vecVar)
				: MxArray(mxCreateCellMatrix(static_cast<mwSize>(vecVar.size()),
											static_cast<mwSize>(1)),
						MxCellClass()) {
		init(&vecVar[0]);
	}

	/*
	 * TODO: Remember to change this if I add constructor from MxArray to all
	 * derived classes of MxArray.
	 */
	explicit MxCell(const detail::PMxArray scalarVar)
				: MxArray(mxCreateCellMatrix(static_cast<mwSize>(1),
											static_cast<mwSize>(1)),
						MxCellClass()) {
		init(&scalarVar);
	}

	template <typename IndexType>
	MxCell(const detail::PMxArrayNative* arrVar,
		const IndexType numDims,
		const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const detail::PMxArray* arrVar,
		const IndexType numDims,
		const IndexType *dims)
		: MxArray(mxCreateCellArray(static_cast<mwSize>(numDims),
									&(std::vector<mwSize>(dims,
														dims + numDims)[0])),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const detail::PMxArrayNative* arrVar,
		const std::vector<IndexType>& dims)
		: MxArray(mxCreateCellArray(static_cast<IndexType>(dims.size()),
									&dims[0]),
				MxCellClass()) {
		init(arrVar);
	}

	template <typename IndexType>
	MxCell(const detail::PMxArray* arrVar, const std::vector<IndexType>& dims)
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
	inline detail::PMxArrayNative operator[](IndexType i) {
		mexAssert(i < getNumberOfElements<IndexType>());
		return mxGetCell(get_array(), i);
	}

	template <typename IndexType>
	inline detail::PMxArrayNative operator[](IndexType i) const {
		mexAssert(i < getNumberOfElements<IndexType>());
		return mxGetCell(get_array(), i);
	}

	inline const detail::PMxArrayNative* getData() const {
		return static_cast<const detail::PMxArrayNative*>(mxGetData(
																get_array()));
	}

	inline detail::PMxArrayNative* getData() {
		return static_cast<detail::PMxArrayNative*>(mxGetData(get_array()));
	}

	inline const std::vector<detail::PMxArrayNative> vectorize() const {
		std::vector<detail::PMxArrayNative> retArg;
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			retArg.push_back((mxGetCell(get_array(), iter)));
		}
		return retArg;
	}

	inline std::vector<detail::PMxArrayNative> vectorize() {
		std::vector<detail::PMxArrayNative> retArg;
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			retArg.push_back((mxGetCell(get_array(), iter)));
		}
		return retArg;
	}

	virtual ~MxCell() = default;
private:
	void init(const detail::PMxArrayNative* arrVar) {
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			mxSetCell(get_array(), iter, arrVar[iter]);
		}
	}

	void init(const detail::PMxArray* arrVar) {
		for (int iter = 0, end = getNumberOfElements(); iter < end; ++iter) {
			mxSetCell(get_array(), iter, arrVar[iter]->get_array());
		}
	}
};

class MxStruct : public MxArray {
public:
	MxStruct() :
			MxArray(MxStructClass()) {}

	MxStruct(const MxStruct& other) = default;
//	MxStruct(const MxStruct& other)
//			: MxArray(other.get_array(), MxStructClass()) {}

	MxStruct& operator=(const MxStruct& other) = default;
//	MxStruct& operator=(const MxStruct& other) {
//		if (this != &other) {
//			this->m_array = other.m_array;
//			this->m_class = other.m_class;
//		}
//		return *this;
//	}

	MxStruct(MxStruct&& other) = default;
	MxStruct& operator=(MxStruct&& other) = default;

	explicit MxStruct(const detail::PMxArrayNative array)
					: MxArray(array, MxStructClass()) {
		mexAssert(get_class().get_classId() == mxGetClassID(array));
	}

	MxStruct(const std::string& scalarName,
			const detail::PMxArrayNative scalarVar)
			: MxArray(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(0),
										nullptr),
					MxStructClass()) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	MxStruct(const std::string& scalarName, const detail::PMxArray scalarVar)
	: MxArray(mxCreateStructMatrix(static_cast<mwSize>(1),
								static_cast<mwSize>(1),
								static_cast<mwSize>(0),
								nullptr),
			MxStructClass()) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	MxStruct(const std::vector<std::string>& vecName,
			const std::vector<detail::PMxArrayNative>& vecVar)
			: MxArray((vecName.size() == vecVar.size())
					?(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(0),
										nullptr))
					:(nullptr),
					MxStructClass()) {
		mexAssert(get_array() != nullptr);
		addField_sub(&vecName[0],
					&vecVar[0],
					static_cast<mwSize>(vecName.size()));
	}

	MxStruct(const std::vector<std::string>& vecName,
			const std::vector<detail::PMxArray>& vecVar)
			: MxArray((vecName.size() == vecVar.size())
					?(mxCreateStructMatrix(static_cast<mwSize>(1),
										static_cast<mwSize>(1),
										static_cast<mwSize>(0),
										nullptr))
					:(nullptr),
					MxStructClass()) {
		mexAssert(get_array() != nullptr);
		addField_sub(&vecName[0],
					&vecVar[0],
					static_cast<mwSize>(vecName.size()));
	}

	void addField(const std::string& scalarName,
				const detail::PMxArrayNative scalarVar) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	void addField(const std::string& scalarName,
				const detail::PMxArray scalarVar) {
		addField_sub(&scalarName, &scalarVar, static_cast<mwSize>(1));
	}

	void addField(const std::vector<std::string>& vecName,
				const std::vector<detail::PMxArrayNative>& vecVar) {
		mexAssert(vecVar.size() == vecName.size());
		addField_sub(&vecName[0], &vecVar[0], vecVar.size());
	}

	void addField(const std::vector<std::string>& vecName,
				const std::vector<detail::PMxArray>& vecVar) {
		mexAssert(vecVar.size() == vecName.size());
		addField_sub(&vecName[0], &vecVar[0], vecVar.size());
	}

	inline bool isField(const std::string& name) const {
		return (mxGetFieldNumber(get_array(), name.c_str()) != -1);
	}

	template <typename IndexType>
	inline IndexType getNumberOfFields() const {
		return static_cast<IndexType>(mxGetNumberOfFields(get_array()));
	}

	inline int getNumberOfFields() const {
		return getNumberOfFields<int>();
	}

	template <typename IndexType>
	inline IndexType getFieldNumber(const std::string& name) const {
		return static_cast<IndexType>(mxGetFieldNumber(get_array(),
													name.c_str()));
	}

	inline int getFieldNumber(const std::string& name) const {
		return getFieldNumber<int>(name);
	}

	template <typename IndexType>
	inline std::string getFieldName(const IndexType i) const {
		return std::string(mxGetFieldNameByNumber(get_array(),
												static_cast<int>(i)));
	}

	inline std::vector<std::string> getFieldNames() const {
		std::vector<std::string> retArg(0);
		for (int iter = 0, end = getNumberOfFields(); iter < end; ++iter) {
			retArg.push_back(std::string(mxGetFieldNameByNumber(get_array(),
															iter)));
		}
		return retArg;
	}

	/*
	 * TODO: The following field access operators (getData, [], and vectorize)
	 * require some thought for the const case. Should I make the pointers const
	 * too?
	 */
	template <typename IndexType>
	inline detail::PMxArrayNative operator[](IndexType i) {
		mexAssert(i < getNumberOfElements());
		return mxGetFieldByNumber(get_array(), 0, static_cast<int>(i));
	}

	template <typename IndexType>
	inline detail::PMxArrayNative operator[](IndexType i) const {
		return mxGetFieldByNumber(get_array(), 0, static_cast<int>(i));
	}

	inline detail::PMxArrayNative operator[](const std::string& name) {
		return mxGetField(get_array(), 0, name.c_str());
	}

	inline detail::PMxArrayNative operator[](const std::string& name) const {
		return mxGetField(get_array(), 0, name.c_str());
	}

	inline const detail::PMxArrayNative* getData() const {
		return static_cast<detail::PMxArrayNative*>(mxGetData(get_array()));
	}

	inline detail::PMxArrayNative* getData() {
		return static_cast<detail::PMxArrayNative*>(mxGetData(get_array()));
	}

	inline void vectorize(std::vector<std::string>& vecName,
						std::vector<detail::PMxArrayNative>& vecVar) const {
		vecName.resize(0);
		vecVar.resize(0);
		for (int iter = 0, end = getNumberOfFields(); iter < end; ++iter) {
			const char* tempName = mxGetFieldNameByNumber(get_array(), iter);
			vecName.push_back(std::string(tempName));
			detail::PMxArrayNative tempVar = mxGetFieldByNumber(get_array(), 0,
																iter);
			vecVar.push_back(tempVar);
		}
	}

	virtual ~MxStruct() = default;
private:
	static const size_t kMxMaxNameLength = mxMAXNAM - 1;

	template <typename IndexType>
	void addField_sub(const std::string* arrName,
					const detail::PMxArrayNative* arrVar,
				IndexType numFields) {
		for (IndexType iter = 0; iter < numFields; ++iter) {
			mexAssert(arrName[iter].size() <= kMxMaxNameLength);
			mxAddField(get_array(), arrName[iter].c_str());
			mxSetField(get_array(), 0, arrName[iter].c_str(), arrVar[iter]);
		}
	}

	template <typename IndexType>
	void addField_sub(const std::string* arrName,
				const detail::PMxArray* arrVar,
				IndexType numFields) {
		for (IndexType iter = 0; iter < numFields; ++iter) {
			mexAssert(arrName[iter].size() <= kMxMaxNameLength);
			mxAddField(get_array(), arrName[iter].c_str());
			mxSetField(get_array(), 0, arrName[iter].c_str(),
					arrVar[iter]->get_array());
		}
	}
};

template <typename T, typename U>
class ConstMap {
public:
	/// Constructor with a single key-value pair
	ConstMap(const T& key, const U& value) :
		m_map() {
		m_map[key] = value;
	}

	/// Consecutive insertion operator
	ConstMap<T, U>& operator()(const T& key, const U& value) {
		m_map[key] = value;
		return *this;
	}

	inline const std::map<T, U>& get_map() const {
		return m_map;
	}

	/// Lookup operator; fail if not found
	inline U operator[](const T& key) const {
		typename std::map<T, U>::const_iterator iter = m_map.find(key);
		mexAssertEx(iter != m_map.end(), "Value not found");
		return (*iter).second;
	}

private:
	std::map<T, U> m_map;
};


template <typename T, typename U>
class ConstBiMap {
public:
	/// Constructor with a single key-value pair
	ConstBiMap(const T& key, const U& value) :
		m_mapLeftToRight(),
		m_mapRightToLeft() {
		m_mapLeftToRight[key] = value;
		m_mapRightToLeft[value] = key;
	}

	/// Consecutive insertion operator
	ConstBiMap<T, U>& operator()(const T& key, const U& value) {
		m_mapLeftToRight[key] = value;
		m_mapRightToLeft[value] = key;
		return *this;
	}

	inline const std::map<T, U>& get_mapLeftToRight() const {
		return m_mapLeftToRight;
	}

	inline const std::map<U, T>& get_mapRightToLeft() const {
		return m_mapRightToLeft;
	}

	/// Lookup operator; fail if not found
	inline U operator[](const T& key) const {
		typename std::map<T, U>::const_iterator iter = m_mapLeftToRight.find(key);
		mexAssertEx(iter != m_mapLeftToRight.end(), "Value not found");
		return (*iter).second;
	}

	inline T find(const U& value) const {
		typename std::map<U, T>::const_iterator iter = m_mapRightToLeft.find(value);
		mexAssertEx(iter != m_mapRightToLeft.end(), "Key not found");
		return (*iter).second;
	}

private:
	std::map<T, U> m_mapLeftToRight;
	std::map<U, T> m_mapRightToLeft;
};

//class MxAttributeInterface {
//public:
//
//	virtual MxArray getValue() const = 0;
//	virtual void setValue(const MxArray& value) const = 0;
//	virtual MxString getName() const = 0;
//
//	virtual ~MxAttributeInterface() {};
//
//};

//class MxObjectInterface {
//public:
//
//	virtual MxArray getAttribute(const mex::MxString& attributeName) const = 0;
//	virtual MxArray getAttribute() const const = 0;
//
//	virtual void setAttribute(const mex::MxString& attributeName,
//					const mex::MxArray& attribute) = 0;
//	virtual void setAttribute(const mex::MxStruct& attributes) = 0;
//
//	virtual ~MxObjectInterface() {};
//};

}  // namespace mex

#endif  // MEX_UTILS_H_
