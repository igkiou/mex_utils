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
 * TODO: Maybe change class hierarchy. It would be nice to have a hierarchy that
 * contains a base class for opening and closing, input with read and peeking,
 * output with write, and input/output with both. It would enforce read/write
 * safety at compile time. Need to avoid diamond pattern. Maybe some form of
 * static polymorphism, policy design or trait-based design. Alternatively, see
 * whether derived types can delete virtual members of ancestors.
 * TODO: Find safe way to handle mxArray headers, to make sure they don't
 * accidentally get passed on later.
 */

namespace mex {

class Mat


class MatFile {
public:

//	 * "r"    == read only.
//	 * "w"    == write only (deletes any existing file with name <filename>).
//	 * "w4"   == as "w", but create a MATLAB 4.0 MAT-file.
//	 * "w7.3" == as "w", but create a MATLAB 7.3 MAT-file.
//	 * "u"    == update.  Read and write allowed, existing file is not deleted.

	explicit MatFile(const std::string& fileName) :
			MatFile(fileName.c_str(), "u") {}

	MatFile(const MatFile& other) = delete;
	MatFile& operator=(const MatFile& other) = delete;
	MatFile(MatFile&& other) = delete;
	MatFile& operator=(MatFile&& other) = delete;

	/*
	 * TODO: Maybe better to implement this by getting name list and scanning
	 * for queryname.
	 */
	virtual inline bool hasVariable(const std::string& variableName) const {
		return (matGetVariableInfo(m_file, variableName.c_str()) != nullptr);
	}

	virtual inline std::vector<std::string> getVariableNames() const {
		int numberVariables;
		char** variableNames = (char**) matGetDir(m_file, &numberVariables);
		mexAssert(variableNames != nullptr);
		std::vector<std::string> retArg(variableNames,
										variableNames + numberVariables);
		mxFree(variableNames);
		return retArg;
	}

	virtual inline MxArrayHeader getVariableInfo(const std::string& variableName) const {
		mxArray* variableHeader = matGetVariableInfo(m_file,
													variableName.c_str());
		mexAssert(variableHeader != nullptr);
		return MxArrayHeader(variableHeader);
	}

	virtual inline MxVariableHeader getNextVariableInfo() const {
		const char* variableNameTemp;
		mxArray* variableHeader = matGetNextVariableInfo(m_file,
														&variableNameTemp);
		mexAssert(variableHeader != nullptr);
		std::string variableName = std::string(variableNameTemp);
		mxFree((void *) variableNameTemp);
		return MxVariableHeader{variableName, MxArrayHeader(variableHeader)};
	}

	template <typename MxArrayType>
	virtual inline MxArrayType readVariable(const std::string& variableName) const {
		mxArray* variable = matGetVariable(m_file, variableName.c_str());
		mexAssert(variable != nullptr);
		return MxArrayType(variable);
	}

	virtual inline MxArray readVariable(const std::string& variableName) const {
		return readVariable<MxArray>(variableName);
	}

	template <typename MxArrayType>
	virtual inline MxVariable readNextVariable() {
		const char* variableNameTemp;
		mxArray* variable = matGetNextVariable(m_file, &variableNameTemp);
		mexAssert(variable != nullptr);
		std::string variableName = std::string(variableNameTemp);
		mxFree((void *) variableNameTemp);
		return MxVariable{variableName, MxArrayType(variable)};
	}

	virtual inline MxVariable readNextVariable() {
		return readNextVariable<MxArray>();
	}

	template <typename MxArrayType>
	virtual inline void writeVariable(const MxArrayType& variable,
							const std::string& variableName) {
		int errorCode = matPutVariable(m_file, variableName.c_str(),
									variable.get_array());
		mexAssert(errorCode == 0);
	}

	virtual inline void writeVariable(const MxVariable& variable) {
		writeVariable(*(variable.m_array), variable.m_name);
	}

	virtual inline void deleteVariable(const std::string& variableName) {
		int errorCode = matDeleteVariable(m_file, variableName.c_str());
		mexAssert(errorCode == 0);
	}

	~MatFile() {
		int errorCode = matClose(m_file);
		mexAssert(errorCode == 0);
	}

protected:
	MatFile(const char* fileName, const char* accessType) :
			m_file(matOpen(fileName, accessType)) {
		mexAssert(m_file != nullptr);
	}

private:
	/*
	 * TODO: Maybe declare const.
	 */
	MATFile* m_file;
};

class MatInputFile : public MatFile {
public:
	explicit MatInputFile(const std::string& fileName) :
			MatFile(fileName.c_str(), "r") {}

	void writeVariable(const MxVariable& variable) override = delete;
	void deleteVariable(const std::string& vaiableName) override = delete;

	virtual ~MatInputFile() = default;
};

class MatOutputFile : MatFile {
public:
	explicit MatOutputFile(const std::string& fileName) :
			MatFile(fileName.c_str(), "w7.3") {}

	bool hasVariable(const std::string& variableName) const override = delete;
	std::vector<std::string> getVariableNames() const override = delete;
	MxArrayHeader getVariableInfo(const std::string& variableName) const override = delete;
	MxVariableHeader getNextVariableInfo() const override = delete;


	template <typename MxArrayType>
	MxArrayType readVariable(const std::string& variableName) const override = delete;
	MxArray readVariable(const std::string& variableName) const override = delete;

	template <typename MxArrayType>
	MxVariable readNextVariable() override = delete;

	virtual ~MatOutputFile() = default;
};

}	/* namespace mex */

#endif /* MAT_UTILS_H_ */
