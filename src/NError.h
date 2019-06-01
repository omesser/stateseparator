#ifndef N_ERROR_H
#define N_ERROR_H

#include <complex>
#include "logger.h"

using std::complex;


/*****************************************************************************
 *                                 Error Codes                               *
 *****************************************************************************/
enum SEP_RESULTS {
	RES_SUCCESS = 0,			// 0
	RES_UNKNOWN_ERROR,			// 1
	RES_INTERNAL_ERROR,			// 2  - An error occured inside the separator. Details are not exported.
	RES_INVALID_ARGS_NUMBER,	// 3  - Number of argument passed to main does not match expected.
	RES_INVALID_SIZES,			// 4  - Invalid particle sizes (should be postive integers only).
	RES_STATE_TOO_BIG,			// 5  - State size exceeds maxMatrixSize. Limit is for security and performance.
	RES_INVALID_MATRIX_VAL,		// 6  - Invalid matrix values (should be complex numbers).
	RES_MATRIX_TOO_SMALL,		// 7  - Not enough matrix rows were passed.
	RES_MATRIX_TOO_BIG,			// 8  - Too many matrix rows were passed.
	RES_ROW_SIZE_MISMATCH,		// 9  - Matrix row size does not match the particle sizes.
	RES_MATRIX_TRACE_NOT_ONE,	// 10 - The trace of the given matrix does not equal 1.
	RES_MATRIX_NOT_HERMITIAN,	// 11 - The given matrix is not hermitian.
	RES_MATRIX_NOT_POSITIVE,	// 12 - The given matrix is not semi-definite positive.
	RES_INVALID_DISTANCE,		// 13 - Invalid distance (should be a positive real number).
	RES_INVALID_PROB,			// 14 - Invalid minimum probability (should be a positive real, smaller than 1).
	RES_INVALID_NUM_OF_STATES,	// 15 - Invalid number of states (should be a positive integer).
	RES_STRING_ERROR,			// 16 - In case getline() failed.
	RES_INVALID_PRECISION		// 17 - Invalid output precision (should be 3, 6 or 9).
};


/*****************************************************************************
 *                                Class NError                               *
 *****************************************************************************/
class NError {
public:
	inline NError(const string& msg, bool critical = true);
	inline NError(SEP_RESULTS errorCode);
	inline void addPreMsg(string msg);
	inline bool criticalFailure() const;
	inline void print() const;
	inline void print_warn() const;
	inline SEP_RESULTS errorCode() const;

private:
	string _msg;
	bool _critical;
	SEP_RESULTS _errorCode;

	static string errorStrings[];

	void handleAdditionalInfo();
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NError::NError(const string& msg, bool critical /*= true*/)
: _msg(msg), _critical(critical), _errorCode(RES_INTERNAL_ERROR)
{
}

NError::NError(SEP_RESULTS errorCode) : _msg(errorStrings[errorCode]), _critical(true), _errorCode(errorCode)
{
}

void NError::addPreMsg(string msg) {
	msg += ": ";
	_msg.insert(0, msg);
}

bool NError::criticalFailure() const {
	return _critical;
}

void NError::print() const {
	NERROR(_msg);
}

void NError::print_warn() const {
	NWARNING(_msg);
}


SEP_RESULTS NError::errorCode() const {
	return _errorCode;
}

#endif // N_ERROR_H
