#include "NInputHandler.h"


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
uint NInputHandler::getParticleSizes(const string& particleSizesStr, vector<uint>& particleSizes) {
	uint stateSize = 1;
	stringstream sizeStrm(particleSizesStr);
	while (!sizeStrm.eof()) {
		uint currsize = 0;
		if (!extract(sizeStrm, currsize)) throw NError(RES_INVALID_SIZES);
		if (currsize == 0) throw NError(RES_INVALID_SIZES);
		particleSizes.push_back(currsize);
		stateSize *= currsize;
		if (stateSize > maxMatrixSize) throw NError(RES_STATE_TOO_BIG);
	}
	return stateSize;
}

void NInputHandler::getMatrix(const char* rows, uint stateSize, MatrixXcd& matrix) {
	stringstream matrixStrm(rows);
	uint currRow = 0;
	while (!matrixStrm.eof()) {
		string row;
		matrixStrm >> ws;
		getline(matrixStrm, row);

		// Output Text starts - ignore it!
		if (row.find("The") != string::npos) break;
		if (row.find("ERROR") != string::npos) break;
		if (matrixStrm.fail() && !matrixStrm.eof()) throw NError(RES_STRING_ERROR);
		if (currRow >= stateSize) {
			if (!row.empty()) throw NError(RES_MATRIX_TOO_BIG);
			continue;
		}
		uint col = 0;
		stringstream rowStrm(row);
		while (!rowStrm.eof()) {
			if (col >= stateSize) throw NError(RES_ROW_SIZE_MISMATCH);
			extract(rowStrm, matrix(currRow,col));
			++col;
		}
		if (col < stateSize) throw NError(RES_ROW_SIZE_MISMATCH);
		++currRow;
	}
	if (currRow < stateSize) throw NError(RES_MATRIX_TOO_SMALL);
}

double NInputHandler::getTargetDistance(const string& targetDistanceStr) {
	if (targetDistanceStr.empty()) return 0;
	double targetDistance = 0;
	if (!convert(targetDistanceStr, targetDistance)) throw NError(RES_INVALID_DISTANCE);
	if (targetDistance < 0) throw NError(RES_INVALID_DISTANCE);
	return targetDistance;
}

double NInputHandler::getMinProbForState(const string& minProbForStateStr) {
	if (minProbForStateStr.empty()) return 0;
	double minProbForState = 0;
	if (!convert(minProbForStateStr, minProbForState)) throw NError(RES_INVALID_PROB);
	if (minProbForState < 0 || minProbForState > 1) throw NError(RES_INVALID_PROB);
	return minProbForState;
}

uint NInputHandler::getTargetNumOfStates(const string& targetNumberOfStatesStr) {
	if (targetNumberOfStatesStr.empty()) return 0;
	uint targetNumberOfStates = 0;
	if (!convert(targetNumberOfStatesStr, targetNumberOfStates)) throw NError(RES_INVALID_NUM_OF_STATES);
	if (targetNumberOfStates < 1) throw NError(RES_INVALID_NUM_OF_STATES);
	return targetNumberOfStates;
}

uint NInputHandler::getOutputPrecision(const string& outputPrecisionStr) {
	if (outputPrecisionStr.empty()) throw NError(RES_INVALID_PRECISION);
	uint outputPrecision;
	if (!convert(outputPrecisionStr, outputPrecision)) throw NError(RES_INVALID_PRECISION);
	if (outputPrecision < 3 || outputPrecision > 15 || (outputPrecision%3)) {
		throw NError(RES_INVALID_PRECISION);
	}
	return outputPrecision;
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
bool NInputHandler::extract(stringstream& strm, uint& store) {
	double temp;
	strm >> temp >> ws;
	if (strm.fail()) return false;
	store = (int)temp;
	return ((double)store == temp);
}

void NInputHandler::extract(stringstream& strm, ValType& val) {
	double realVal = 0;
	double imagVal = 0;
	string valStr;
	string imagStr;
	string realStr;
	strm >> valStr >> ws;

	if (valStr.empty()) {
		throw NError(RES_INVALID_MATRIX_VAL);
	}
	size_t iDelim = valStr.find('i');
	size_t firstDelim = std::min(valStr.find('+'),valStr.find('-'));
	size_t secondDelim = (firstDelim != std::string::npos) ? std::min(valStr.substr(firstDelim+1).find('-'),valStr.substr(firstDelim+1).find('+')) : std::string::npos;
	// No i:
	if (iDelim == string::npos) {
		imagStr = "";
		realStr = valStr;
	} else { // i present:
		// i is first char:
		// Format: [imaginary]  || [+/-][imaginary]
		if ((firstDelim == std::string::npos)
		   || ((firstDelim == 0) && (secondDelim == std::string::npos))) {
			imagStr = (valStr.substr(0,iDelim) + valStr.substr(iDelim+1));
			realStr = "";
		// Format: [real][+/-][imaginary]
		} else if ((firstDelim != std::string::npos) && (firstDelim > 0) && (iDelim > firstDelim)) {
			imagStr = (valStr.substr(0,iDelim) + valStr.substr(iDelim+1)).substr(firstDelim);
			realStr = valStr.substr(0,firstDelim);
		// Format: [+/-][real][+/-][imaginary]
		} else if ((firstDelim == 0) && (secondDelim != std::string::npos) && (iDelim > secondDelim)) {
			imagStr = (valStr.substr(0,iDelim) + valStr.substr(iDelim+1)).substr(secondDelim+1);
			realStr = valStr.substr(0,secondDelim+1);
		// Format: [imaginary][+/-][real]
		} else if ((firstDelim != std::string::npos) && (firstDelim > 0) && (iDelim < firstDelim)) {
			imagStr = (valStr.substr(0,iDelim) + valStr.substr(iDelim+1)).substr(0,firstDelim-1);
			realStr = valStr.substr(firstDelim);
		// Format: [+/-][imaginary][+/-][real]
		} else if ((firstDelim == 0) && (secondDelim != std::string::npos) && (iDelim <= secondDelim)) {
			imagStr = (valStr.substr(0,iDelim) + valStr.substr(iDelim+1)).substr(0,secondDelim);
			realStr = valStr.substr(secondDelim+1);
		// Format - Illegal!
		} else {
			throw NError(RES_INVALID_MATRIX_VAL);
		}
		// No [number] after/before i, we treat 'i' as '1*i'
		if (imagStr.find_first_of("0123456789") == std::string::npos) {
			imagStr += "1";
		}		
	}
	// str2double:
	if (!realStr.empty()) {
		if (!convert(realStr, realVal)) {
			throw NError(RES_INVALID_MATRIX_VAL);
		}
	}
	if (!imagStr.empty()) {
		if (!convert(imagStr, imagVal)) {
			throw NError(RES_INVALID_MATRIX_VAL);
		}
	}
	val.real(realVal);
	val.imag(imagVal);
}
