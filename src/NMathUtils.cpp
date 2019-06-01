#include "NMathUtils.h"
#include "NError.h"

const double epsilon = 5E-19;
const double epsilonRelaxed = 5E-15;
const double epsilonSuperRelaxed = 5E-4;

/*****************************************************************************
 *                          FUNCTION IMPLEMENTATIONS                         *
 *****************************************************************************/
bool operator<(const ValType& lhs, const ValType& rhs) {
	//double lAmp = abs(lhs);
	//double rAmp = abs(rhs);
	double lAmp = lhs.real();
	double rAmp = rhs.real();
	if (!isEqual(lAmp, rAmp)) {
		return (lAmp < rAmp);
	}
	double lArg = arg(lhs);
	double rArg = arg(rhs);
	if (!isEqual(lArg, rArg)) {
		return (lArg < rArg);
	}
	return false;
}

VectorXcd tensorProduct(const VectorXcd& lhs, const VectorXcd& rhs) {
	uint lsize = (uint)lhs.size();
	uint rsize = (uint)rhs.size();
	uint totalSize = lsize * rsize;
	VectorXcd res(totalSize);
	uint runningIndex = 0;
	for (uint l = 0; l < lsize; ++l) {
		for (uint r = 0; r < rsize; ++r) {
			res[runningIndex] = lhs[l] * rhs[r];
			++runningIndex;
		}
	}
	return res;
}

ValType trace(const MatrixXcd& mat) {
	ValType tr = mat.trace();
#ifdef __NSTRICT__
	if (!isZero(tr.imag())) {
		stringstream strm;
		strm << "Calculated a non-real trace: " << tr << endl << mat;
		throw NError(strm.str(), false);
	}
#endif // __NSTRICT__
	return tr;
}

ValType trace(const MatrixXcd& lhs, const MatrixXcd& rhs) {
	
	MatrixXcd res = (lhs.adjoint().eval())*rhs;

	// This is an optimization only for hermitian matrices, and since we exapnd 
	// to support none hermitian - no longer relevant :(
	/*
	uint size = (uint)lhs.rows();
	for (uint i = 0; i < size; ++i) {
		for (uint j = 0; j < i; ++j) {
			res += 2*(lhs(i,j)*rhs(j,i)).real();
		}
		res += lhs(i,i).real()*rhs(i,i).real();
	}
	*/
	return res.trace();
}

