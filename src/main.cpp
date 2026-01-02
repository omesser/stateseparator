#include "NInputHandler.h"
#include "NOutputHandler.h"
#include "NSeparator.h"

#ifndef __NRUNONSERVER__

#include <iostream>
#include "NTester.h"  // this is used for native runs only - for testing during development

using std::cin;
using std::cout;
using std::endl;

uint getScenario() {
	cout << "1) 2 2 2 2 2" << endl
	     << "2) 2 3 7" << endl
	     << "3) barely separable example" << endl
	     << "4) slightly entangled example" << endl
	     << "5) first bell state" << endl
	     << "6) second bell state" << endl
	     << "7) simple separable state" << endl;
	cout << "Please choose one of the scenarios above: ";
	uint choice = 0;
	cin >> choice;
	if (cin.fail()) {  // TODO: handle errors better
		exit(1);
	}
	if (choice < 1 || choice > 7) {  // TODO: handle errors better
		exit(2);
	}
	return choice;
}

#else  // This code is for the server version of the State Separator

/*
 * This function verifies that the given matrix is a valid density matrix.
 * There is a similar function in NRandomizer.cpp for the NRandomizer executable.
 * See Randomizer project in this solution.
 */
static void verifyMatrix(const MatrixXcd& mat) {
	stringstream ss;
	// TEST 1: trace(A) == 1
	if (!isOnePermissive(mat.trace())) {
		// Only warning
		NError(RES_MATRIX_TRACE_NOT_ONE).print_warn();
		// throw NError(RES_MATRIX_TRACE_NOT_ONE);
	}
	// TEST 2: Hermiticity :  Real vals on diagonal, conjugates on all other entries
	for (int i = 0; i < mat.rows(); ++i) {
		if (!isZero(mat(i, i).imag())) {
			// Only warning
			NError(RES_MATRIX_NOT_HERMITIAN).print_warn();
			break;
			// throw NError(RES_MATRIX_NOT_HERMITIAN);
		}
		for (int j = 0; j < i; ++j) {
			if (!isEqual(conj(mat(i, j)), mat(j, i))) {
				// Only warning
				NError(RES_MATRIX_NOT_HERMITIAN).print_warn();
				break;
				// throw NError(RES_MATRIX_NOT_HERMITIAN);
			}
		}
	}
	EVSolver evs(mat);
	VectorXd eigenvalues = evs.eigenvalues();
	// TEST 3: Positive Semi-definite - all EigenValues >= 0
	for (int i = 0; i < eigenvalues.size(); ++i) {
		if (eigenvalues[i] < 0 && !isZeroPermissive(eigenvalues[i])) {
			NError(RES_MATRIX_NOT_POSITIVE).print_warn();
			ss << "The matrix EigenValues are: ";
			NOutputHandler::printStrm(ss, eigenvalues.cast<ValType>());
			NDATA(ss);
			break;
		}
	}
}


void printResults(const NResult& res) {
	stringstream finalstrm;
	bool peresPassFlag = true;

	if (res._minPeresEV < 0 && !isZeroPermissive(res._minPeresEV)) {
		peresPassFlag = false;
	}

	// Verbal Veridct
	if (!peresPassFlag) {
		finalstrm << "The system is entangled by Peres-Horodecki test." << endl;
	} else {
		if (res._reason == NREASON_DISTANCE) {
			finalstrm << "The system is separable." << endl;
		} else if (res._distance > 0.0005) {  // Originally 0.005
			finalstrm << "The system is most likely entangled." << endl;
		} else {
			finalstrm << "The system might be entangled." << endl;
		}
	}

	finalstrm << "The Eigenvalues of the partially-transposed (Peres test) matrix are: " << endl;
	// Printing the EVs with nice formatting
	NOutputHandler::printStrm(finalstrm, res._peresEVs);
	finalstrm << endl << endl;
	// Main Algorithm output
	finalstrm << "A separable matrix at distance of ";
	NOutputHandler::printStrm(finalstrm, res._distance);
	finalstrm << " from the original matrix:";
	NDATA(finalstrm);
	res._state.print();
}

/*
 * argv[1] - particle sizes				MANDATORY
 * argv[2] - matrix					MANDATORY
 * argv[3] - target distance				OPTIONAL
 * argv[4] - minimum probability per state		OPTIONAL
 * argv[5] - target number of states			OPTIONAL
 * argv[6] - output precision				MANDATORY
 * argv[7] - accuracy boost				OPTIONAL
 */
NResult runOnServer(int argc, char* argv1[]) {
	if (argc < 7 || argc > 8) {
		throw NError(RES_INVALID_ARGS_NUMBER);
	}

	// Get the particle sizes
	vector<uint> particleSizes;
	uint stateSize = NInputHandler::getParticleSizes(argv1[1], particleSizes);
	uint outputPrecision = NInputHandler::getOutputPrecision(argv1[6]);
	NOutputHandler::setPrecision(outputPrecision);

	// Get the Matrix
	MatrixXcd matrix(stateSize, stateSize);
	NInputHandler::getMatrix(argv1[2], stateSize, matrix);
	verifyMatrix(matrix);

	// Get the optional parameters
	double targetDistance = NInputHandler::getTargetDistance(argv1[3]);
	double minProbForState = NInputHandler::getMinProbForState(argv1[4]);
	uint targetNumberOfStates = NInputHandler::getTargetNumOfStates(argv1[5]);

	// boost accuracy?
	bool accuracyBoost = false;
	if (argc == 8 && argv1[7] == "1") {
		accuracyBoost = true;
	}

	// Now we are ready to start the actual work - separate the matrix!
	return NSeparator::getInstance()->separate(matrix, particleSizes, targetDistance, minProbForState,
	                                           targetNumberOfStates, accuracyBoost);
}

#endif  // __NRUNONSERVER__

/*****************************************************************************
 *                               MAIN FUNCTION                               *
 *****************************************************************************/
int main(int argc, char* argv[]) {
	try {
#ifdef __NRUNONSERVER__
		NResult res = runOnServer(argc, argv);
		printResults(res);

#else  // run native tests - development only

		if (argc > 1) {
			throw NError(RES_INVALID_ARGS_NUMBER);
		}

		uint runScenario = getScenario();

		switch (runScenario) {
		case 1:  // Test one random matrix - default size is "2 2"
			testRandom("2 2 2 2 2");
			break;
		case 2:
			testRandom("2 3 7");
			break;
		case 3:
			runExample(NEM_BARELY_SEPARABLE);
			break;
		case 4:
			runExample(NEM_SLIGHTLY_ENTANGLED);
			break;
		case 5:
			runExample(NEM_BELL_1);
			break;
		case 6:
			runExample(NEM_BELL_2);
			break;
		case 7:
			runExample(NEM_SIMPLE_SEP);
			break;
		default:;
		}

#endif  // __NRUNONSERVER__

	} catch (const NError& err) {
		err.print();
		return err.errorCode();
	} catch (...) {
		NERROR("MAIN: Failed with unexpected error!!!");
		return RES_UNKNOWN_ERROR;
	}
	return RES_SUCCESS;
}
