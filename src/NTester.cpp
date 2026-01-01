/*
 * The following code is for native testing during development.
 * Matrix randomization is done on the server using the NRandomizer executable.
 * See Randomizer project in this solution and the makefile for Linux.
 */
#ifndef __NRUNONSERVER__

#include "NTester.h"
#include "NInputHandler.h"
#include "NOutputHandler.h"
#include "NSeparator.h"
#include "NCollector.h"


/*****************************************************************************
 *                       STATIC FUNCTIONS DECLARATIONS                       *
 *****************************************************************************/
static void runOnce(const MatrixXcd& matrix, const vector<uint>& particleSizes);
static NRandomGenerator* initialize();
MatrixXcd buildMatrix(NRandomGenerator* gen, const char* particleSizesStr, vector<uint>& particleSizes,
                      ExampleMatrices example);
static MatrixXcd randomizeMatrix(NRandomGenerator* gen, const vector<uint>& particleSizes, uint stateSize,
                                 uint numOfStates = 0);
static MatrixXcd buildExample(ExampleMatrices example);
static void printResults(const MatrixXcd& mat, const NResult& res);

/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
void runExample(ExampleMatrices example) {
	NRandomGenerator* gen = initialize();

	vector<uint> particleSizes;
	MatrixXcd matrix = buildMatrix(gen, "2 2", particleSizes, example);

	runOnce(matrix, particleSizes);
}

void testRandom(const char* particleSizesStr /*= "2 2"*/) {
	NRandomGenerator* gen = initialize();

	vector<uint> particleSizes;
	MatrixXcd matrix = buildMatrix(gen, particleSizesStr, particleSizes, NEM_END);

	runOnce(matrix, particleSizes);
}


/*****************************************************************************
 *                      STATIC FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
// runOnce - apply the separator to the given system and collect statistics
void runOnce(const MatrixXcd& matrix, const vector<uint>& particleSizes) {
	// Optional parameters
	double targetDistance = 0;      // use default
	double minProbForState = 0;     // use default
	uint targetNumberOfStates = 0;  // use default

	// Get the separator
	NSeparator* sep = NSeparator::getInstance();

	// Now we are ready to start the actual work - separate the matrix!
	NSTARTCOLLECTION(NCT_TOTAL_TIME);
	NResult res = sep->separate(matrix, particleSizes, targetDistance, minProbForState, targetNumberOfStates);
	NENDCOLLECTION(NCT_TOTAL_TIME);

	// Print
	printResults(matrix, res);
}

// initialize - start the PRNG and set the output precision
NRandomGenerator* initialize() {
	// Set the output precision
	uint outputPrecision = 15;
	NOutputHandler::setPrecision(outputPrecision);

	// Start the pseudo random number generator
#ifdef __NNORANDOM__
	NRandomGenerator::create(1329498937);  // use a constant seed - makes it easier to debug
#endif                                     // __NNORANDOM__
	return NRandomGenerator::getInstance();
}

// buildMatrix - return the requested matrix according to the given particleSizesStr and example if applicable
MatrixXcd buildMatrix(NRandomGenerator* gen, const char* particleSizesStr, vector<uint>& particleSizes,
                      ExampleMatrices example) {
	// Get the particle sizes
	uint stateSize = 0;
	stateSize = NInputHandler::getParticleSizes(particleSizesStr, particleSizes);

	// Build the matrix
	if (example == NEM_END) {
		return randomizeMatrix(gen, particleSizes, stateSize);
	} else {
		return buildExample(example);
	}
}

// randomizeMatrix - randomizes a new matrix
static MatrixXcd randomizeMatrix(NRandomGenerator* gen, const vector<uint>& particleSizes, uint stateSize,
                                 uint numOfStates /*= 0*/) {
	uint numOfParticles = (uint)particleSizes.size();
	if (numOfStates == 0) {
		numOfStates = gen->getUint(stateSize * stateSize - 1);
	}
	MatrixXcd mat(stateSize, stateSize);
	mat.setZero();
	VectorXd probs(numOfStates);
	gen->randomizeProbabilities(probs);
	NPureState pureState(particleSizes);
	NLOG("Generating randomized state:");
	for (uint s = 0; s < numOfStates; ++s) {
		for (uint i = 0; i < numOfParticles; ++i) {
			gen->randomizeVector(pureState[i]);
		}
		pureState.finalize();
#ifdef __NDEBUG__
		pureState.print(probs[s], s);
#endif  // __NDEBUG__
		mat += (pureState * probs[s]);
	}
	return mat;
}

// buildExample - returns one of the example matrices
static MatrixXcd buildExample(ExampleMatrices example) {
	MatrixXcd matrix(4, 4);
	switch (example) {
	case NEM_BARELY_SEPARABLE:
		matrix << 0.33, 0, 0, 0.16, 0, 0.17, 0, 0, 0, 0, 0.17, 0, 0.16, 0, 0, 0.33;
		break;
	case NEM_SLIGHTLY_ENTANGLED:
		matrix << 0.335, 0, 0, 0.17, 0, 0.165, 0, 0, 0, 0, 0.165, 0, 0.17, 0, 0, 0.335;
		break;
	case NEM_BELL_1:
		matrix << 0.50, 0, 0, 0.50, 0, 0, 0, 0, 0, 0, 0, 0, 0.50, 0, 0, 0.50;
		break;
	case NEM_BELL_2:
		matrix << 0, 0, 0, 0, 0, 0.50, -0.50, 0, 0, -0.50, 0.50, 0, 0, 0, 0, 0;
		break;
	case NEM_SIMPLE_SEP:
		matrix << 0.3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.7;
		break;
	default:
		throw NError("Bad example requested from tester.");
	}
	return matrix;
}

// printResults - print the given matrix and the separation results
void printResults(const MatrixXcd& matrix, const NResult& res) {
	NDATA("The given matrix was:");
	NOutputHandler::print(matrix);
	stringstream reasonstrm;
	reasonstrm << "Calculation terminated due to " << ReasonString[res._reason] << "." << endl;

#ifdef __NCOLLECTSTATS__
	reasonstrm << "Total runtime: " << collectedTime(NCT_TOTAL_TIME) << "." << endl
	           << "Total time spent in the main iteration: " << collectedTime(NCT_MAIN_ITERATION) << "." << endl
	           << "    the FNPS algorithm took: " << collectedTime(NCT_FNPS) << "." << endl
	           << "        eigenvalue calculation took: " << collectedTime(NCT_CALC_EV) << "." << endl
	           << "        partial traceouts took: " << collectedTime(NCT_PTO) << "." << endl
	           << "Total time for mixing states: " << collectedTime(NCT_MIXIN) << "." << endl
	           << "    adding states took: " << collectedTime(NCT_ADD_STATE) << "." << endl
	           << "    minimization took: " << collectedTime(NCT_MINIMIZATION) << "." << endl
	           << "        block allocation took: " << collectedTime(NCT_BLOCK_ALLOCATION) << "." << endl
	           << "        linear solving took: " << collectedTime(NCT_LINEAR_SOLVE) << "." << endl
	           << "        QP shifting took: " << collectedTime(NCT_SHIFT) << "." << endl
	           << "    probability updates took: " << collectedTime(NCT_UPDATE_PROBS) << "." << endl
	           << "    matrix rebuilding took: " << collectedTime(NCT_BUILD_MATRIX) << "." << endl;
#endif  // __NCOLLECTSTATS__

	reasonstrm << endl << "The approximated matrix is at distance ";
	NOutputHandler::printStrm(reasonstrm, res._distance);
	reasonstrm << " from the original matrix:";
	NDATA(reasonstrm);
	res._state.print();
}

#endif  // __NRUNONSERVER__
