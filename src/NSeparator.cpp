#include <map>
#include <algorithm>
#include "NSeparator.h"
#include "NCollector.h"

using std::map;
using std::sort;
using std::reverse;
using std::log;

NSeparator* NSeparator::_theInstance = NULL;
const uint FNPSITERNUM_LOW  = 100; 
const uint FNPSITERNUM_HIGH = 1000;



/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
/* NResult separate(const MatrixXcd& src, const vector<uint>& particleSizes,
 *				    const double& targetDistance, const double& minProbForState, uint targetNumberOfStates)
 *
 * Stage I: Auxiliary test - performing Peres test on the given matrix.
 * Stage II: main algorithm:
 * Given a test state, which could be a separable state or an entangled state. We want to find
 * the "nearest" separable state if it is entangled, or the decomposition to pure states if it
 * is separable. Both of the above are approximations which could change due to the algorithm's
 * reliance on randomization.
 * 
 * Notations:
 *   "rho"		- The test state (the given density matrix).
 *   "rho_s"	- The approximated state (the iterative built state). Stands for "rho separable".
 *   "rho_p"	- A pure state which is mixed-in to rho_s each iteration. Stands for "rho pure".
 *   "sigma"	- 'sigma = (rho - rho_s)'
 *   "delta"	- 'delta = (rho_p - rho_s)'
 *   "S"		- 'S = trace(delta * sigma)'
 *
 * Algorithm:
 * In every iteration do:
 *   1) Find rho_p which maximizes S.
 *   2) If 'S > 0', mix rho_p with rho_s.
 *   3) If 'S <= 0', return rho_s (without rho_p) - this is the approximation.
 *
 * Input:
 *   src					- The test state. Should be a legal density matrix.
 *   particleSizes			- A list of sizes of the basic particles that construct the src state.
 *							  The sum of these sizes must equal the size of src.
 *   targetDistance			- End the algorithm when the calculated distance is lower than this.
 *   minProbForState		- Disacrd states with respective probabilites below this.
 *   targetNumberOfStates	- Confine the solution to contain at most this many states.
 *   boostAccuracy			- boosting accuracy by uping the iteration ceiling and inner fnpsIterationNum 
 *							 (number of trace outs steps). Defaults to false
 *
 * Return value:
 *   A Result struct containing the reason for for termination, and upon successful seapration,
 *   the approximated separable mixed state and the distance to the given state.
 *
 */
NResult NSeparator::separate(const MatrixXcd& src, const vector<uint>& particleSizes,
							 double targetDistance, double minProbForState, uint targetNumberOfStates, bool boostAccuracy)
{
	// For debug printing, when wanting to print more than string+int
	std::ostringstream debugstrm;
	// Trace out iteration number
	uint fnpsIterationNum = 0;
	uint N = (uint)src.rows();
	// TIMEOUT - outermost iteration ceiling
	int OuterIterationCounter = 0;

	// 2 modes of rigorousness: 
	if (boostAccuracy){
		NLOG("NSeparator: Accuracy boost - rigorous computation mode");
		fnpsIterationNum = FNPSITERNUM_HIGH;
		OuterIterationCounter = N*N*N;
	} else {
		NLOG("NSeparator: No accuracy boost - fast mode");
		fnpsIterationNum = FNPSITERNUM_LOW;
		OuterIterationCounter = N*N + N;
		//OuterIterationCounter = targetNumberOfStates + (int)log((double)targetNumberOfStates); // original TIMEOUT
	}

	// Initialization
	reset(src, particleSizes, minProbForState, targetNumberOfStates, fnpsIterationNum);
	if (isZero(targetDistance)) {
		targetDistance = targetDistanceDefault;
	}
	NMixedState rho_s(rho_p, minProbForState, targetNumberOfStates, rho); // rho_p is finalized in reset()
	NResult result(rho_s, rho_s.distance());

	// Stage I - Peres test:
	result._peresEVs = _peresTester.peresTest(rho, particleSizes);
	result._minPeresEV = result._peresEVs[0];
	for (uint i = 0; i < (uint)result._peresEVs.size(); ++i) {
		if (result._peresEVs[i] < result._minPeresEV) {
			result._minPeresEV = result._peresEVs[i];
		}
	}

	// Stage II - Main algorithm:

	// The first iteration is out of the main loop in order to calculate the first value of S.
	NSTARTCOLLECTION(NCT_MAIN_ITERATION);
	double S = mainIterationStep(rho_s, fnpsIterationNum);
	NENDCOLLECTION(NCT_MAIN_ITERATION);


	
	// If the first iteration produced S < epsilon - we expect the default reason to be this:
	if (S <= epsilon) {
		result._reason = NREASON_DISTANCE;
	}
	
	// The main loop:
	double minDist = result._distance;
	try {
		while (S > epsilon) {
			NSTARTCOLLECTION(NCT_MIXIN);
			double distance = rho_s.mixIn(rho_p);
			NENDCOLLECTION(NCT_MIXIN);
			if (distance > minDist) {
				// TODO: this exception should be thrown after the numerical bugs are fixed.
				// throw NError("NSeparator: Calculated larger distance than the previous iteration.");
				//NLOGNUM("NSeparator: Calculated larger distance than the previous minimum", distance);
				debugstrm << "NSeparator: Calculated larger distance: " << distance << " than the previous minimum: " << minDist;
				NLOG(debugstrm.str());
				debugstrm.str(std::string()); // efficiently cleaning the debugstrm
				// TODO: find a way to revert this result
			}
			else {
				minDist = distance;
			}
			if (distance < targetDistance) { // target distance reached
				result._reason = NREASON_DISTANCE;
				break;
			}
			if (--OuterIterationCounter < 0) { // timeout reached - return the last best mixed state
				result._reason = NREASON_TIMEOUT;
				break;
			}
			NSTARTCOLLECTION(NCT_MAIN_ITERATION);
			S = mainIterationStep(rho_s, fnpsIterationNum);   // rho_p is calculated here
			NENDCOLLECTION(NCT_MAIN_ITERATION);
		}
		if (result._reason == NREASON_NONE) {
			result._reason = NREASON_S;
		}
	} catch (NError err) {
		// If the failure is critical (e.g. memory allocation failure) we should escalate the error.
		// Otherwise, just set the termination reason to fail which means numerical error.
		if (err.criticalFailure()) throw;
		result._reason = NREASON_FAIL;
		result._distance = rho_s.distance();
		result._state = rho_s;
#ifdef __NDEBUG__
		err.print();
#endif // __NDEBUG__
	}
	
	result._distance = rho_s.distance();
	rho_s.sortStates();
	result._state = rho_s;
	return result;
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
uint NSeparator::ParticleRecord::completeRecord(uint jumpFactor, uint systemSize) {
	_jumpFactor = jumpFactor;
	uint reducedSize = systemSize / _size;
	_indices.resize(reducedSize);
	_subMatrix.resize(reducedSize,reducedSize);
	buildIndices(systemSize);
	return reducedSize;
}

void NSeparator::ParticleRecord::buildIndices(uint systemSize) {
	uint blockSize = _jumpFactor * _size;
	uint numOfBlocks = systemSize / blockSize;

	// set the indices array
	uint runningIndex = 0;
	uint jumpingIndex = 0;
	for (uint b = 0; b < numOfBlocks; ++b) {
		jumpingIndex = b*blockSize;
		for (uint j = 0; j < _jumpFactor; ++j) {
			_indices[runningIndex] = jumpingIndex;
			++jumpingIndex;
			++runningIndex;
		}
	}
}

void NSeparator::ParticleRecord::partialTraceout(const MatrixXcd& matrix, const VectorXcd& vec) {
	uint reducedSize = (uint)_subMatrix.rows();

	// Calculate the (i,k) elements of the reduced matrix
	for (uint i = 0; i < reducedSize; ++i) {
		uint iStart = _indices[i];
		for (uint k = 0; k < i; ++k) {
			uint kStart = _indices[k];
			_subMatrix(i,k) = 0;
			for (uint r = 0; r < _size; ++r) {
				uint rActual = r*_jumpFactor;
				for (uint s = 0; s < _size; ++s) {
					uint sActual = s*_jumpFactor;
					_subMatrix(i,k) += conj(vec[r])
									* matrix(iStart + rActual , kStart + sActual)
									* vec[s];
				}
			}
			_subMatrix(k,i) = conj(_subMatrix(i,k));
		}
		_subMatrix(i,i) = 0;
		// The diagonal calculation is optimized here. We can do this because the block for calculating the diagonal
		// values is self-adjoint. This means that we can consider only the lower triangular part of the block.
		for (uint r = 0; r < _size; ++r) {
			uint rActual = r*_jumpFactor;
			for (uint s = 0; s < r; ++s) {
				uint sActual = s*_jumpFactor;
				ValType val = conj(vec[r])
							* matrix(iStart + rActual , iStart + sActual)
							* vec[s];
				_subMatrix(i,i) += 2 * val.real();
			}
			_subMatrix(i,i) += conj(vec[r])
							* matrix(iStart + rActual , iStart + rActual)
							* vec[r];
		}
	}
}

void NSeparator::reset(const MatrixXcd& src, const vector<uint>& particleSizes, double& minProbForState,
					   uint& targetNumberOfStates, uint fnpsIterationNum)
{
	// Initialize all the member data fields.
	_systemSize = (uint)src.rows();
	rho = src;
	rho_p = particleSizes;
	_numOfParticles = (uint)particleSizes.size();
	_solvers.resize(_numOfParticles);
	for (uint i = 0; i < _numOfParticles; ++i) {
		uint particleSize = particleSizes[i];
		_solvers[i] = EVSolver(particleSize);
	}

	buildSystemData(particleSizes);

	// Set the target number of states if not supplied by the user. We don't need more than N^2.
	if (targetNumberOfStates == 0) {
		int N = (int)rho.rows();
		targetNumberOfStates = N*N;
	}

	// Set the minimum probability threshold for keeping a pure state if not supplied by the user.
	if (isZero(minProbForState)) {
		minProbForState = 0;
	}

	// Start with the completely mixed state.
	MatrixXcd begin(VectorXcd::Constant(rho.rows(),1./rho.rows()).asDiagonal());

	// Now find the nearest pure state.
	findNearestPureState(rho - begin, fnpsIterationNum);
}

double NSeparator::mainIterationStep(const NMixedState& rho_s, uint fnpsIterationNum) {
	MatrixXcd sigma = rho - rho_s;
	NSTARTCOLLECTION(NCT_FNPS);
	findNearestPureState(sigma, fnpsIterationNum); //rho_p is calculated here
	NENDCOLLECTION(NCT_FNPS);
	return trace((rho_p - rho_s), sigma).real();
}

void NSeparator::findNearestPureState(const MatrixXcd& sigma, uint maxIterationNumber) {
	for (uint i = 1; i < _numOfParticles; ++i) {
		// Randomize all the particles except the first one (which will be calculated).
		_gen->randomizeVector(rho_p[i], false);
	}
	uint count = 0;
	while (!calcFNPSIteration(sigma)) {
		if (++count >= maxIterationNumber) break; // This heuristic can be changed. Originally 100
	}
	rho_p.finalize(); // This builds the state's matrix form
}

bool NSeparator::calcFNPSIteration(const MatrixXcd& sigma) {
	vector<double> eigenValues(_numOfParticles);
	bool done = true; // true if the max eigenvalue of all submatrices is the same
	for (uint i = 0; i < _numOfParticles; ++i) {
		// Perform the traceout
		NSTARTCOLLECTION(NCT_PTO);
		traceOut(sigma, i);
		NENDCOLLECTION(NCT_PTO);

		// Find the maximal eigenvalue
		NSTARTCOLLECTION(NCT_CALC_EV);
		eigenValues[i] = maxEigenVector(_solvers[i], _traceoutData[i][_numOfParticles-2]._subMatrix, rho_p[i]);
		NENDCOLLECTION(NCT_CALC_EV);

		if (!isEqual(eigenValues[i], eigenValues[0])) {
			done = false;
		}
	}
	return done;
}

void NSeparator::traceOut(const MatrixXcd& sigma, uint particleNum) {
	SystemStructure& theSystem = _traceoutData[particleNum];
	uint numOfTraceouts = _numOfParticles - 1;
	theSystem[0].partialTraceout(sigma, rho_p[theSystem[0]._index]);
	for (uint i = 1; i < numOfTraceouts; ++i) {
		theSystem[i].partialTraceout(theSystem[i-1]._subMatrix, rho_p[theSystem[i]._index]);
	}
}

double NSeparator::maxEigenVector(EVSolver& solver, const MatrixXcd& mat, VectorXcd& eigenVector) {
	// The solver references only the lower triangular part of the matrix.
	solver.compute(mat);
	if (solver.info() != Eigen::Success) {
		throw NError("NSeparator: Failed to calculate eigenvectors.");
	}
	// The eigenvalues were stored in increasing order by the compute method.
	uint maxIndex = (uint)(mat.rows() - 1);
	eigenVector = solver.eigenvectors().col(maxIndex);
	return solver.eigenvalues()[maxIndex];
}

void NSeparator::buildSystemData(const vector<uint>& particleSizes) {
	// Build the sorted system
	vector<uint> rawSystem(_numOfParticles);
	SystemStructure sortedSystem(_numOfParticles);
	for (uint p = 0; p < _numOfParticles; ++p) {
		rawSystem[p] = particleSizes[p];
		sortedSystem[p]._size = particleSizes[p];
		sortedSystem[p]._index = p;
	}
	sort(sortedSystem.begin(), sortedSystem.end());
	reverse(sortedSystem.begin(), sortedSystem.end());
	for (uint p = 0; p < _numOfParticles; ++p) {
		uint index = sortedSystem[p]._index;
		uint jumpFactor = 1;
		rawSystem[index] = 1;
		for (uint i = index+1; i < _numOfParticles; ++i) {
			jumpFactor *= rawSystem[i];
		}
		sortedSystem[p]._jumpFactor = jumpFactor;
	}

	// Build the traceout database
	uint numOfTraceouts = _numOfParticles - 1;
	_traceoutData.resize(_numOfParticles);
	for (uint p = 0; p < _numOfParticles; ++p) {
		uint systemSize = _systemSize;
		uint currentSize = particleSizes[p];
		SystemStructure& particleData = _traceoutData[p];
		particleData.resize(numOfTraceouts);
		for (uint i = 0, d = 0; i < _numOfParticles; ++i) {
			if (sortedSystem[i]._index == p) continue;
			particleData[d] = sortedSystem[i];
			uint jumpFactor = sortedSystem[i]._jumpFactor;
			if (sortedSystem[i]._size <= currentSize && sortedSystem[i]._index < p) {
				jumpFactor *= currentSize;
			}
			systemSize = particleData[d].completeRecord(jumpFactor, systemSize);
			++d;
		}
	}
}
