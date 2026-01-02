#include <iomanip>
#include <Eigen/Eigenvalues>
#include "NInputHandler.h"
#include "NOutputHandler.h"
#include "NRandomGenerator.h"
#include "NPureState.h"

using Eigen::SelfAdjointEigenSolver;
using std::fixed;
using std::setprecision;

typedef SelfAdjointEigenSolver<MatrixXcd> EVSolver;

// randomizeMatrix - randomizes a new matrix
static MatrixXcd randomizeMatrix(NRandomGenerator* gen, const vector<uint>& particleSizes, uint stateSize) {
	uint numOfParticles = particleSizes.size();

	uint numOfStates = gen->getUint(stateSize * stateSize - 1);

	MatrixXcd mat(stateSize, stateSize);
	mat.setZero();
	VectorXd probs(numOfStates);
	gen->randomizeProbabilities(probs);
	NPureState pureState(particleSizes);
	for (uint s = 0; s < numOfStates; ++s) {
		for (uint i = 0; i < numOfParticles; ++i) {
			gen->randomizeVector(pureState[i]);
		}
		pureState.finalize();
		mat += (pureState * probs[s]);
	}
	return mat;
}

static void truncateMatrix(MatrixXcd& matrix, uint outputPrecision) {
	uint size = matrix.rows();
	for (uint i = 0; i < size; ++i) {
		for (uint j = 0; j < i; ++j) {
			stringstream realValStrm;
			stringstream imagValStrm;
			realValStrm << fixed << setprecision(outputPrecision) << matrix(i, j).real();
			imagValStrm << fixed << setprecision(outputPrecision) << matrix(i, j).imag();
			double realVal;
			double imagVal;
			realValStrm >> realVal;
			imagValStrm >> imagVal;
			matrix(i, j).real(realVal);
			matrix(i, j).imag(imagVal);
			matrix(j, i).real(realVal);
			matrix(j, i).imag(-imagVal);
		}
		stringstream diagStrm;
		double diagVal;
		diagStrm << fixed << setprecision(outputPrecision) << matrix(i, i).real();
		diagStrm >> diagVal;
		matrix(i, i).real(diagVal);
		matrix(i, i).imag(0);
	}
}

static void correctTrace(MatrixXcd& matrix) {
	double matTrace = trace(matrix).real();
	if (!isEqual(matTrace, 1)) {
		matTrace -= matrix(0, 0).real();
		matrix(0, 0).real(1 - matTrace);
	}
}

static bool verifyMatrix(const MatrixXcd& mat) {
	for (int i = 0; i < mat.rows(); ++i) {
		if (!isZero(mat(i, i).imag())) {
			return false;
		}
		for (int j = 0; j < i; ++j) {
			if (!isEqual(conj(mat(i, j)), mat(j, i))) {
				return false;
			}
		}
	}
	EVSolver evs(mat);
	VectorXd eigenvalues = evs.eigenvalues();
	for (int i = 0; i < eigenvalues.size(); ++i) {
		if (eigenvalues[i] < 0) {
			return false;
		}
	}
	return true;
}


/*****************************************************************************
 *                               MAIN FUNCTION                               *
 *****************************************************************************/
/*
 * argv[1] - particle sizes
 * argv[2] - output precision
 */
int main(int argc, char* argv[]) {
	try {
		if (argc != 3) {
			throw NError(RES_INVALID_ARGS_NUMBER);
		}

		vector<uint> particleSizes;
		uint stateSize = NInputHandler::getParticleSizes(argv[1], particleSizes);

		// Get the output precision
		uint outputPrecision = NInputHandler::getOutputPrecision(argv[2]);
		NOutputHandler::setPrecision(outputPrecision);

		// Initialize the Pseudo-Random Number Generator
		NRandomGenerator* gen = NRandomGenerator::getInstance();

		MatrixXcd matrix;
		do {
			matrix = randomizeMatrix(gen, particleSizes, stateSize);
			truncateMatrix(matrix, outputPrecision);  // set the desired precision
			correctTrace(matrix);                     // compensate for the limited precision
		} while (!verifyMatrix(matrix));
		NOutputHandler::print(matrix);
	} catch (const NError& err) {
		err.print();
		return err.errorCode();
	} catch (...) {
		NERROR("MAIN: Failed with unexpected error!!!");
		return RES_UNKNOWN_ERROR;
	}
	return RES_SUCCESS;
	return 0;
}
