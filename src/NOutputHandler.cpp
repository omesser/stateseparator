#include <iomanip>
#include <ios>
#include "NOutputHandler.h"

using std::fixed;
using std::left;
using std::right;
using std::setiosflags;
using std::setprecision;
using std::setw;

#if defined(linux)
typedef std::_Ios_Fmtflags flagsType;
#else   // not linux
typedef int flagsType;
#endif  // not linux

uint NOutputHandler::_outputPrecision = 3;  // default value - this will be overriden by user input
uint NOutputHandler::_componentWidth = 5;   // -0.RRR
uint NOutputHandler::_outputWidth = 12;     // 0.RRR+i0.III  does not include the leading '-' of the real component
uint NOutputHandler::_pureImaginary = 11;


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
void NOutputHandler::print(const ValType& num) {
	stringstream strm;
	printStrm(strm, num);
	NDATA(strm);
}

void NOutputHandler::print(const MatrixXcd& matrix) {
	stringstream strm;
	uint size = (uint)matrix.rows();
	for (uint i = 0; i < size; ++i) {
		printStrm(strm, matrix.row(i), true);
		strm << endl;
	}
	NDATA(strm);
}

double NOutputHandler::truncate(double num, char* sign) {
	stringstream tempStrm;
	tempStrm << setprecision(_outputPrecision) << num;
	if (tempStrm.peek() == '-') {
		tempStrm.ignore(1);
		*sign = '-';
	}
	tempStrm >> num;
	return num;
}

void NOutputHandler::printStrm(stringstream& strm, const ValType& num) {
	char realSign = ' ';
	double realVal = truncate(num.real(), &realSign);
	char imagSign = (realVal == 0) ? ' ' : '+';
	double imagVal = truncate(num.imag(), &imagSign);

	if (realVal != 0) {
		if (imagVal != 0) {
			strm << realSign << setw(_componentWidth) << realVal << imagSign << "i" << setw(_componentWidth) << imagVal;
		} else {
			strm << realSign << setw(_outputWidth) << realVal;
		}
	} else {
		if (imagVal != 0) {
			strm << imagSign << "i" << setw(_pureImaginary) << imagVal;
		} else {
			strm << ' ' << setw(_outputWidth) << 0;
		}
	}
}

void NOutputHandler::printStrm(stringstream& strm, const double& num) {
	strm << fixed << setprecision(_outputPrecision) << num;
}

void NOutputHandler::printStrm(stringstream& strm, const VectorXcd& vec, bool isMAtrixRow /*= false*/) {
	strm << fixed << left << setprecision(_outputPrecision);
	uint size = (uint)vec.size();
	string delim = (isMAtrixRow) ? "  " : ", ";
	if (!isMAtrixRow) {
		strm << "(";
	}
	printStrm(strm, vec[0]);
	for (uint i = 1; i < size; ++i) {
		strm << delim;
		printStrm(strm, vec[i]);
	}
	if (!isMAtrixRow) {
		strm << ")";
	}
}
