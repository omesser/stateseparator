#include "NPeresTester.h"
#include "NOutputHandler.h"
#include "NError.h"


typedef Eigen::ComplexEigenSolver<MatrixXcd> GenEVSolver;

NPeresTester::NPeresTester(void) {}

NPeresTester::~NPeresTester(void) {}
/* Applies the peres test to a given matrix by trying all possible partial
 * transposes on each of the particles, and returning the EigenValues vector of the
 * configuration that returned the lowest minimal EigenValue.
 */
VectorXcd NPeresTester::peresTest(const MatrixXcd& src, const vector<uint>& particleSizes) const {
	MatrixXcd mat = src;
	GenEVSolver solver = GenEVSolver();
	// initial value
	VectorXcd minEVVector;
	ValType minEV;
	// For debug messages:
	std::stringstream ss;
	uint matSize = (uint)mat.rows();
	NLOG("NPeresTester: Starting Peres test...");
	uint numOfParticles = (uint)particleSizes.size();
	for (uint p = 0; p < numOfParticles; ++p) {
		ss << "NPeresTester: Transposing particle " << p << " of size: " << particleSizes[p];
		NLOG(ss.str());
		ss.str(std::string());

		uint blockSize = multVec(particleSizes, p, numOfParticles - 1);
		uint stepSize = multVec(particleSizes, p + 1, numOfParticles - 1);
		// p=0 -> stepSize=2
		// p=1 -> stepSize=1
		for (uint factor_c = 0; factor_c < matSize; factor_c += blockSize) {
			for (uint factor_r = 0; factor_r < matSize; factor_r += blockSize) {
				ss << "NPeresTester: block (" << (factor_c) << "," << (factor_r) << ") of size: " << blockSize;
				NLOG(ss.str());
				ss.str(std::string());
				for (uint c = 0; c < blockSize; c += stepSize) {
					for (uint r = c + stepSize; r < blockSize; r += stepSize) {  // Avoid redundent swappings
						                                                         // Inside the block...
						for (uint block_c = 0; block_c < stepSize; ++block_c) {
							for (uint block_r = 0; block_r < stepSize; ++block_r) {
								// Swap original elements:
								if ((std::max(factor_c + c + block_c, factor_r + r + block_r) < matSize) &&
								    (std::max(factor_c + r + block_c, factor_r + c + block_r) < matSize)) {
									// Debug logging
									ss << "NPeresTester: swapping: m(" << (factor_c + c + block_c) << ","
									   << (factor_r + r + block_r) << ") and m(" << (factor_c + r + block_c) << ","
									   << (factor_r + c + block_r) << ")";
									NLOG(ss.str());
									ss.str(std::string());
									// Swapping around block axis:
									mat(factor_c + c + block_c, factor_r + r + block_r) =
									    src(factor_c + r + block_c, factor_r + c + block_r);
									mat(factor_c + r + block_c, factor_r + c + block_r) =
									    src(factor_c + c + block_c, factor_r + r + block_r);
								}
							}
						}
					}
				}
			}
		}
		// The Eigenvalues were stored in increasing order by the compute method.
		solver.compute(mat, false);
		if (solver.info() != Eigen::Success) {
			throw NError("NPeresTest: Failed to calculate EigenValue.");
		}
		// First tieration:
		if (p == 0) {
			minEVVector = solver.eigenvalues();
			// Get min EV:
			minEV = minEVVector[0];
			for (uint i = 0; i < (uint)minEVVector.size(); ++i) {
				if (minEVVector[i] < minEV) {
					minEV = minEVVector[i];
				}
			}
		}

		ss << "NPeresTester: EigenValues are: ";
		NOutputHandler::printStrm(ss, solver.eigenvalues());
		NLOG(ss.str());
		ss.str(std::string());
		// Update min EV:
		for (uint i = 0; i < (uint)solver.eigenvalues().size(); ++i) {
			if (solver.eigenvalues()[i] < minEV) {
				minEVVector = solver.eigenvalues();
				minEV = minEVVector[i];
			}
		}
	}
	return minEVVector;
}


uint NPeresTester::multVec(const vector<uint>& vec, uint start, uint end) const {
	uint res = 1;
	for (uint i = start; i <= end; ++i) {
		res *= vec[i];
	}
	return res;
}
