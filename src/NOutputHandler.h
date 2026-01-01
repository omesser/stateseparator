#ifndef N_OUTPUT_HANDLER_H
#define N_OUTPUT_HANDLER_H

#include "basic_def.h"


/*****************************************************************************
 *                            Class NOutputHandler                           *
 *****************************************************************************/
class NOutputHandler {
public:
	static void print(const MatrixXcd& matrix);
	static void printStrm(stringstream& strm, const double& num);
	static void printStrm(stringstream& strm, const VectorXcd& vec, bool isMAtrixRow = false);
	static inline void setPrecision(uint outputPrecision);

private:
	static uint _outputPrecision;
	static uint _outputWidth;
	static uint _componentWidth;
	static uint _pureImaginary;

	static void print(const ValType& num);
	static void printStrm(stringstream& strm, const ValType& num);

	static double truncate(double num, char* sign);
	static inline void calcWidth();
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
void NOutputHandler::setPrecision(uint outputPrecision) {
	_outputPrecision = outputPrecision;
	calcWidth();
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
void NOutputHandler::calcWidth() {
	_componentWidth = _outputPrecision + 2;
	_outputWidth = (_componentWidth) * 2 + 2;  // this does not include the leading '-' of the real component
	_pureImaginary = _outputWidth - 1;
}

#endif  // N_OUTPUT_HANDLER_H
