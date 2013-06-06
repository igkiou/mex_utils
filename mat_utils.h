/*
 * mex_utils//mex_utils/mat_utils.h/mat_utils.h
 *
 *  Created on: Jun 5, 2013
 *      Author: igkiou
 */

#ifndef MAT_UTILS_H_
#define MAT_UTILS_H_

#include <fstream>
#include <string>
#include <vector>
#include <ctype.h>

#include "mat.h"
#include "mex_utils.h"

/*
 * TODO: Maybe create an enum for matfile types, instead of storing them as
 * strings, and util function for mapping enum to strings and vice versa.
 * However, opening with "r"/"w" is consistent with C-types.
 * TODO: Maybe change class hierarchy. It would be nice to have a hierarchy that
 * contains a base class for opening and closing, input with read and peeking,
 * output with write, and input/output with both. It would enforce read/write
 * safety at compile time. Need to avoid diamond pattern. Maybe some form of
 * static polymorphism, especially if matfile types become enum.
 */

namespace mex {

namespace {
typedef MATFile MatFileNative;
typedef MATFile* PMatFileNative;
} /* namespace */

class MatFile {
public:
	explicit MatFile(const std::string& fileName)
					: m_file(matOpen(fileName.c_str(), "r")),
					  m_accessType("r") {
		mexAssert(m_file != NULL);
	}

	MatFile(const std::string& fileName, const std::string accessType)
		: m_file(matOpen(fileName.c_str(), accessType.c_str())),
		  m_accessType(accessType.c_str()) {
		mexAssert(m_file != NULL);
	}

//	explicit MatFile(const PMatFileNative file)
//					: m_file(file),
//					  m_accessType("r") {	}

	MatFile(const MatFile& matFile)
		: m_file(matFile.get_file()),
		  m_accessType(matFile.get_accessType()) {	}

	MatFile& operator=(const MatFile& matFile) {
		if (this != &matFile) {
			this->m_file = matFile.get_file();
			this->m_accessType = matFile.get_accessType();
		}
		return *this;
	}

	inline const PMatFileNative get_file() const {
		return m_file;
	}

	inline PMatFileNative get_file() {
		return m_file;
	}

	inline const std::string& get_accessType() const {
		return m_accessType;
	}

	inline std::vector<std::string> getVariableNames() const {
		int numberVariables;
		char** variableNames = (char**) matGetDir(m_file, &numberVariables);
		mexAssert(variableNames != NULL);
		std::vector<std::string> retArg(variableNames,
										variableNames + numberVariables);
		mxFree(variableNames);
		return retArg;
	}

	inline MxArray getVariableInfo(const std::string& variableName) const {
		mxArray* variableHeader = matGetVariableInfo(m_file,
													variableName.c_str());
		mexAssert(variableHeader != NULL);
		return MxArray(variableHeader);
	}

	inline MxArray getNextVariableInfo(std::string& variableName) const {
		const char* variableNameTemp;
		mxArray* variableHeader = matGetNextVariableInfo(m_file,
														&variableNameTemp);
		mexAssert(variableHeader != NULL);
		variableName = std::string(variableNameTemp);
		mxFree((void *) variableNameTemp);
		return MxArray(variableHeader);
	}

	template <typename MxArrayType>
	inline MxArrayType read(const std::string& variableName) {
		mexAssert((!strcmp(m_accessType.c_str(), "r"))
				|| (!strcmp(m_accessType.c_str(), "u")));
		mxArray* variable = matGetVariable(m_file, variableName.c_str());
		mexAssert(variable != NULL);
		return MxArrayType(variable);
	}

	inline MxArray read(const std::string& variableName) {
		return read<MxArray>(variableName);
	}

	/*
	 * TODO: Get next variable, maybe rename to getNext.
	 */
	template <typename MxArrayType>
	inline MxArrayType get(std::string& variableName) {
		mexAssert((!strcmp(m_accessType.c_str(), "r"))
				|| (!strcmp(m_accessType.c_str(), "u")));
		const char* variableNameTemp;
		mxArray* variable = matGetNextVariable(m_file, &variableNameTemp);
		mexAssert(variable != NULL);
		variableName = std::string(variableNameTemp);
		mxFree((void *) variableNameTemp);
		return MxArrayType(variable);
	}

	inline MxArray get(std::string& variableName) {
		return get<MxArray>(variableName);
	}

	template <typename MxArrayType>
	inline bool write(const MxArrayType& variable,
					const std::string& variableName) {
		mexAssert((!strcmp(m_accessType.c_str(), "w"))
				|| (!strcmp(m_accessType.c_str(), "w7.3"))
				|| (!strcmp(m_accessType.c_str(), "u")));
		return matPutVariable(m_file, variableName.c_str(),
							variable.get_array());
	}

	inline bool clear(const std::string& variableName) {
		mexAssert((!strcmp(m_accessType.c_str(), "w"))
				|| (!strcmp(m_accessType.c_str(), "w7.3"))
				|| (!strcmp(m_accessType.c_str(), "u")));
		return (matDeleteVariable(m_file, variableName.c_str()) == 0);
	}

	~MatFile() {
		mexAssert(matClose(m_file) == 0);
	}

protected:
	PMatFileNative m_file;
	std::string m_accessType;
};

//class MatInputFile: public MatFile {
//	explicit MatInputFile(const std::string& fileName)
//			: MatFile(fileName, std::string("r")) {	}
//
//	MatInputFile(const MatInputFile& matInputFile)
//		: m_file(matInputFile.get_file()),
//		  m_accessType(matInputFile.get_accessType()) {	}
//
//	MatInputFile& operator=(const MatInputFile& matInputFile) {
//		if (this != &matInputFile) {
//			this->m_file = matInputFile.get_file();
//			this->m_accessType = matInputFile.get_accessType();
//		}
//		return *this;
//	}
//};
//
//class MatOutputFile: public MatFile {
//	explicit MatOutputFile(const std::string& fileName)
//			: MatFile(fileName, std::string("w7.3")) {	}
//
//	MatOutputFile(const MatOutputFile& matOutputFile)
//		: m_file(matOutputFile.get_file()),
//		  m_accessType(matOutputFile.get_accessType()) {	}
//
//	/*
//	 * Only copies pointer, does not do "deep copy".
//	 */
//	MatOutputFile& operator=(const MatOutputFile& matOutputFile) {
//		if (this != &matOutputFile) {
//			this->m_file = matOutputFile.get_file();
//			this->m_accessType = matOutputFile.get_accessType();
//		}
//		return *this;
//	}
//};
//
//class MatInputOutputFile: public MatFile {
//	explicit MatInputOutputFile(const std::string& fileName)
//			: MatFile(fileName, std::string("u")) {	}
//
//	MatInputOutputFile(const MatInputOutputFile& matInputOutputFile)
//		: m_file(matInputOutputFile.get_file()),
//		  m_accessType(matInputOutputFile.get_accessType()) {	}
//
//	/*
//	 * Only copies pointer, does not do "deep copy".
//	 */
//	MatInputOutputFile& operator=(const MatInputOutputFile& matInputOutputFile) {
//		if (this != &matInputOutputFile) {
//			this->m_file = matInputOutputFile.get_file();
//			this->m_accessType = matInputOutputFile.get_accessType();
//		}
//		return *this;
//	}
//};

}	/* namespace mex */

#endif /* MAT_UTILS_H_ */
