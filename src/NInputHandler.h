#ifndef N_INPUT_HANDLER_H
#define N_INPUT_HANDLER_H

#include "basic_def.h"

using std::ws;

// This maximum size is intended to keep the computation time reasonable as well as placing a limit on the
// input size for security reasons. This is not air-tight, but it's a start.
const uint maxMatrixSize = 100;


/*****************************************************************************
 *                            Class NInputHandler                            *
 *****************************************************************************/
class NInputHandler {
public:
	static uint getParticleSizes(const string& particleSizesStr, vector<uint>& particleSizes);
	static void getMatrix(const char* rows, uint stateSize, MatrixXcd& matrix);
	static double getTargetDistance(const string& targetDistanceStr);
	static double getMinProbForState(const string& minProbForStateStr);
	static uint getTargetNumOfStates(const string& targetNumberOfStatesStr);
	static uint getOutputPrecision(const string& outputPrecisionStr);

private:
	static inline bool convert(const string& str, uint& store);
	static inline bool convert(const string& str, double& store);
	static bool extract(stringstream& strm, uint& store);
	static inline bool extract(stringstream& strm, double& store);
	static void extract(stringstream& strm, ValType& val);
};


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
bool NInputHandler::convert(const string& str, uint& store) {
	stringstream strm(str);
	return extract(strm, store);
}

bool NInputHandler::convert(const string& str, double& store) {
	stringstream strm(str);
	return extract(strm, store);
}

bool NInputHandler::extract(stringstream& strm, double& store) {
	strm >> store;
	if (strm.fail()) return false;
	strm >> ws;  // consume trailing whitespace separately
	return true;
}

#endif // N_INPUT_HANDLER_H
