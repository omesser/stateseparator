#include <Eigen/QR>
#include "NMixedState.h"
#include "NOutputHandler.h"
#include "NCollector.h"

#ifdef __NFULLMT__

#ifndef __N_MT_ADDSTATE__
#define __N_MT_ADDSTATE__
#endif // __N_MT_ADDSTATE__

#ifndef __N_MT_BUILDMATRIX__
#define __N_MT_BUILDMATRIX__
#endif // __N_MT_BUILDMATRIX__

#endif // __NFULLMT__


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
inline void addQColWrapper(int start, int targets, void* mixedStatePtr, const void* newState);
inline void addQRowWrapper(int start, int targets, void* mixedStatePtr, const void* newState);
inline void threadBuildMatrixWrapper(int threadIndex, int start, int targets, void* mixedStatePtr);


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NMixedState::NMixedState(const NPureState& src, const double& minProbForState, uint targetNumberOfStates,
						 const MatrixXcd& targetState)
: _size(src.size()), _numOfStates(1), _matrixForm(src.matrix()), _minProbForState(minProbForState),
  _targetNumberOfStates(targetNumberOfStates), _targetState(targetState),
  // The +1 below is for preallocation of an extra state which may be entered (and removed) in every iteration
  // after _targetNumberOfStates was reached.
  _Q(VectorXd::Constant(_targetNumberOfStates+1,1.0).asDiagonal()), _p(_targetNumberOfStates+1),
  _probs(_targetNumberOfStates+1), _startIndex(0), _lastIndex(0), _stateIterators(_targetNumberOfStates+1),
  _tp(NThreadPool::getInstance()),
  _addQColAction(_tp->registerAction(addQColWrapper)),
  _addQRowAction(_tp->registerAction(addQRowWrapper)),
  _buildMatrixAction(_tp->registerAction(threadBuildMatrixWrapper))
{
	_states.push_back(src);
	_p[0] = trace(src,_targetState).real();
	_probs[0] = 1; //TODO: is this good enough?
#ifdef __N_MT_BUILDMATRIX__
	_intermediateMatrices.resize(_tp->numOfThreads());
	for (uint i = 0; i < _intermediateMatrices.size(); ++i) {
		_intermediateMatrices[i].setZero(_size,_size);
	}
#else // single threaded execution
	_intermediateMatrices.resize(1);
	_intermediateMatrices[0].setZero(_size,_size);
#endif // __N_MT_BUILDMATRIX__
    _stateIterators[0] = _states.begin();
}

NMixedState& NMixedState::operator=(const NMixedState& src) {
#ifdef __NSTRICT__
	if (src._size != _size) throw NError("NMixedState: Assignment should not change state size.");
#endif // __NSTRICT__
	if (this != &src) {
		_states = src._states;
		_numOfStates = src._numOfStates;
		_matrixForm = src._matrixForm;
		_minProbForState = src._minProbForState;
		_targetState  = src._targetState;
		_Q = src._Q.triangularView<Eigen::Lower>();
		_p = src._p;
		_probs = src._probs; // copying the array is not really necessary (it is already allocated), simply for completeness
		_startIndex = src._startIndex;
		_lastIndex = src._lastIndex;
        buildIteratorsArray();
	}
	return *this;
}

double NMixedState::mixIn(const NPureState& state) {
	// Add the new state
	NSTARTCOLLECTION(NCT_ADD_STATE);
	addState(state);
	NENDCOLLECTION(NCT_ADD_STATE);

	// Find the new probabilities
	NSTARTCOLLECTION(NCT_MINIMIZATION);
	minimize();
	NENDCOLLECTION(NCT_MINIMIZATION);

	// Remove the minimal state if the target number of states was surpassed.
	if (_numOfStates > _targetNumberOfStates) {
		removeMinState();
	}

	// Finalize
	NSTARTCOLLECTION(NCT_BUILD_MATRIX);
	buildMatrix();
	NENDCOLLECTION(NCT_BUILD_MATRIX);
	return distance();
}

double NMixedState::distance() const {
	MatrixXcd sqrdist = _targetState - _matrixForm;
	return sqrt(trace(sqrdist, sqrdist).real());
}

void NMixedState::print() const {
	NOutputHandler::print(_matrixForm);
	stringstream printstrm;
	printstrm << "The separable-matrix is a convex sum of " << _numOfStates << " pure separable states:";
	NDATA(printstrm);
	CStateIterator it = _states.begin();
	for (uint i = 1, p = _startIndex; i <= _numOfStates; ++i, ++p, ++it) {
		it->print(_probs[p], i);
	}
}


// State comparison, by descending probability
void NMixedState::sortStates() {
	NLOG("NMixedState: Sorting states.");
	uint endIndex = _startIndex + _numOfStates - 1;
	StateIterator it_start = _states.begin();
	StateIterator it_end = it_start;
	
	for (uint i = 1; i < _numOfStates; ++i) {
		++it_end;
	}
	quickSortStates(_startIndex, endIndex, it_start, it_end);
}
// QuickSort O(NlogN)
void NMixedState::quickSortStates(uint left, uint right, StateIterator it_start, StateIterator it_end) {
	
	// Stop
	if (right - left < 2) {
		return;
	}
	uint i = left;
	uint j = right;
	StateIterator it_i = it_start;
	StateIterator it_j = it_end;
	double pivot = _probs[ (uint)((left + right) / 2)];

	/* partition */
	while (i <= j) {
		while (_probs[i] > pivot) {
			++i; ++it_i;
		}
		while (_probs[j] < pivot) {
			--j; --it_j;
		}
        if (i <= j) {
			// Swapping:
			std::swap(_probs[i], _probs[j]);
			std::swap(*it_i, *it_j);
	        ++i; ++it_i;
            --j; --it_j;
		}
	}
	/* Recursion */
	if (left < j) quickSortStates(left, j, it_start, it_j);
	if (i < right) quickSortStates(i, right, it_i, it_end);
}


// Alternative (slower) selection sort
/*
void NMixedState::sortStates() {
	NLOG("NMixedState: Sorting states.");
	uint lastIndex = _startIndex + _numOfStates - 1;
	// Index of highest probability state
	uint maxIndex;
	
	StateIterator it_i = _states.begin();
	StateIterator it_j;
	StateIterator it_max;
	
	// Selection sort
	for (uint i = _startIndex; i < lastIndex; ++i, ++it_i) {
		maxIndex = i;
		it_j = it_i;
		it_max = it_i;
		++it_j;
		for (uint j = i + 1; j <= lastIndex; ++j, ++it_j) {
			if (_probs[j] > _probs[maxIndex]) { 
				maxIndex = j;
				it_max = it_j;
			}
		}
		if (maxIndex != i) {	
			// Swapping:
			std::swap(_probs[i], _probs[maxIndex]);
			std::swap(*it_i, *it_max);
		}
	}
}
*/

/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
void NMixedState::addFront(const NPureState& newState) {
	// Add the new state to the list
	_states.push_front(newState);
	--_startIndex;
	++_numOfStates;
    _stateIterators[_startIndex] = _states.begin();

	// Update Q
#ifdef __N_MT_ADDSTATE__
	_tp->execute(_addQColAction, _numOfStates, this, &newState);
#else // single threaded execution
	addQCol(0, _numOfStates, newState);
#endif // __N_MT_ADDSTATE__

	// Update p
	_p[_startIndex] = trace(newState, _targetState).real();
}

void NMixedState::addBack(const NPureState& newState) {
	// Add the new state to the list
	_states.push_back(newState);
	++_lastIndex;
	++_numOfStates;
    _stateIterators[_lastIndex] = --_states.end();

	// Update Q
#ifdef __N_MT_ADDSTATE__
	_tp->execute(_addQRowAction, _numOfStates, this, &newState);
#else // single threaded execution
	addQRow(0, _numOfStates, newState);
#endif // __N_MT_ADDSTATE__

	// Update p
	_p[_lastIndex] = trace(newState, _targetState).real();
}

void NMixedState::shiftUp(uint violatingIndex) {
	NLOG("NMixedState: Performing shift up.");
    uint oldJ = violatingIndex-1;
    StateIterator it = _stateIterators[violatingIndex];
	eraseState(it); // remove the stale state

	// shift up
	for (uint newJ = violatingIndex; newJ > _startIndex; --newJ, --oldJ) {
        _stateIterators[newJ] = --it;
		uint newI = _lastIndex;
		for (; newI > violatingIndex; --newI) {
			_Q(newI, newJ) = _Q(newI,oldJ);
		}
		for (uint oldI = newI-1; newI > newJ; --newI, --oldI) {
			_Q(newI,newJ) = _Q(oldI,oldJ);
		}
		_p[newJ] = _p[oldJ];
	}
	++_startIndex;
#ifdef __NSTRICT__
	if (it != _states.begin()) throw NError("NMixedState: The state list is inconsistent.");
#endif // __NSTRICT__
}

void NMixedState::shiftDown(uint violatingIndex) {
	NLOG("NMixedState: Performing shift down.");
    uint oldI = violatingIndex+1;
    StateIterator it = _stateIterators[violatingIndex];
	eraseState(it); // remove the stale state

	// shift down
	for (uint newI = violatingIndex; newI < _lastIndex; ++newI, ++oldI, ++it) {
        _stateIterators[newI] = it;
		uint newJ = _startIndex;
		for (; newJ < violatingIndex; ++newJ) {
			_Q(newI,newJ) = _Q(oldI,newJ);
		}
		for (uint oldJ = newJ+1; newJ < newI; ++newJ, ++oldJ) {
			_Q(newI,newJ) = _Q(oldI,oldJ);
		}
		_p[newI] = _p[oldI];
	}
	--_lastIndex;
#ifdef __NSTRICT__
	if (it != _states.end()) throw NError("NMixedState: The state list is inconsistent.");
#endif // __NSTRICT__
}

void NMixedState::removeMinState() {
	NLOG("NMixedState: Removing extra state.");
	uint i = _startIndex;
	double minProb = _probs[i];
	uint minIndex = i;
	for (++i; i <= _lastIndex; ++i) {
		if (_probs[i] < minProb) {
			minProb = _probs[i];
			minIndex = i;
		}
	}
	NSTARTCOLLECTION(NCT_SHIFT);
	compress(minIndex);
	NENDCOLLECTION(NCT_SHIFT);
}

/* void NMixedState::minimize();
 *
 * This function calculates the probabilities which give the minimal distance from the target state.
 * We want to solve the following Quadratic Programming optimization problem:
 *
 *				min f(alpha) = 1/2 (alpha' * Q * alpha) + p' * alpha
 *				s.t. _minProbForState <= alpha[i] <= 1	(1)
 *				s.t. sum(alpha[i]) = 1					(2)
 *				where Q is a real symmetric positive semi-definite matrix
 *				and the ' notatation signifies the transposed vector
 *
 * Notations (corresponding variables appear in parenthesis)
 *   Q		(_Q)		- The given NxN matrix for the QP optimization problem.
 *   p		(_p)		- The given size-N vector for the QP optimization problem.
 *   alpha	(_probs)	- The unknowns size-N vector (in our case - the probabilites).
 *
 * Ideally, we would use a QP algorithm however no such good open source algorithm was found. So we
 * reduce the problem to solving a set of linear equations. We note that f recieves a minimum when
 * the gradient, defined by F(alpha) = grad(f) = (Q * alpha) + p, is zero. This defines a set of
 * linear equations which can be easily solved.
 *
 * Algorithm:
 * 1) Find min f(alpha) which satisfies (2) - this is done in findSolution() below.
 * 2) If an entry in alpha is found which violates (1) (denoted by violatingIndex)
 *    a) Remove the corresponding state from the states list.
 *    b) Shift the Q and p entries to eliminate the gap that was formed.
 *    c) Return to step 1.
 * 
 * The algorithm terminates when no such violating probability is found.
 */
void NMixedState::minimize() {
	NLOGNUM("NMixedState: Minimizing", _numOfStates);
	uint violatingIndex = findSolution();
	while (violatingIndex <= _lastIndex) {
		NLOGNUM("NMixedState: Removing stale state at index: ", violatingIndex);
		NSTARTCOLLECTION(NCT_SHIFT);
		compress(violatingIndex);
		NENDCOLLECTION(NCT_SHIFT);
		violatingIndex = findSolution();
	}
}

/* uint NMixedState::findSolution();
 *
 * Solve the set of linear equations:
 *
 *				(Q * alpha) + p = 0
 *				s.t. sum(alpha[i]) = 1
 *				where Q is a real symmetric positive semi-definite NxN matrix
 *				and p is a size-N real vector
 *
 * To satisfy the constraint we would like to add a Lagrange multiplier. This can be done by assigning
 * "1" to the p vector at entry N+1, adding a row of "1"s to Q at N+1 and nullifying the value on the
 * diagonal at N+1 like so:
 *				_Q.row(_lastIndex+1).setOnes();
 *				_Q(_lastIndex+1,_lastIndex+1) = 0;
 *				_p[_lastIndex+1] = 1;
 * This however prevents the Q matrix from being a positive semi-definite matrix which will require us
 * to use a much slower linear solving algorithm (we use the ldlt decomposition which is the fastest
 * Eigen provides). So we trade off some accuracy for speed and use Q and p as they are.
 * We can normalize the resulting vector - disabled, to allow approximations of input matrices with none-unitary trace.
 */
uint NMixedState::findSolution() {
	// Prepare the system to be solved
	NSTARTCOLLECTION(NCT_BLOCK_ALLOCATION);
	const MatrixXd& A = _Q.block(_startIndex, _startIndex, _numOfStates, _numOfStates).triangularView<Eigen::Lower>();
	const VectorXd& b = _p.segment(_startIndex, _numOfStates);
	NENDCOLLECTION(NCT_BLOCK_ALLOCATION);

	// Solve the set of linear equations
	NSTARTCOLLECTION(NCT_LINEAR_SOLVE);
	_probs.segment(_startIndex, _numOfStates) = A.selfadjointView<Eigen::Lower>().ldlt().solve(b);
	NENDCOLLECTION(NCT_LINEAR_SOLVE);

	// Find the most violating index
	double minVal = _minProbForState;
	uint violatingIndex = _lastIndex + 1;
	double sum = 0;
	for (uint i = _startIndex; i <= _lastIndex; ++i) {
		sum += _probs[i];
		if (_probs[i] < minVal && !isZero(_probs[i])) {
			minVal = _probs[i];
			violatingIndex = i;
		}
	}
	if (isZero(sum)) throw NError("NMixedState: Calculated 0 sum."); // avoid division by zero
	
	// Without forcing sum==1 we allow none-unitary matrices to be approximated
	/*
	if (!isOne(sum)) {
		_probs /= sum;
		// After normalizing, the violating entry might not be violating anymore.
		if (violatingIndex <= _lastIndex) {
			if (_probs[violatingIndex] >= _minProbForState) {
				violatingIndex = _lastIndex + 1;
			}
		}
	}
	*/
	return violatingIndex;
}

void NMixedState::buildMatrix() {
	// Build intermediate matrices
#ifdef __N_MT_BUILDMATRIX__
	_tp->execute(_buildMatrixAction, _numOfStates, this);
#else // Single threaded execution
	buildIntermediateMatrix(0, 0, _numOfStates);
#endif // __N_MT_BUILDMATRIX__
	// Sum up to the actual matrix
	_matrixForm.noalias() = _intermediateMatrices[0];
#ifdef __N_MT_BUILDMATRIX__
	for (uint i = 1; i < _intermediateMatrices.size(); ++i) {
		_matrixForm.noalias() += _intermediateMatrices[i];
	}
#endif // __N_MT_BUILDMATRIX__

#ifdef __NSTRICT__
	// Disabled to allow more generic matrices to be approximated
	//if (!isOnePermissive(_matrixForm.trace())) throw NError("NMixedState: Matrix trace does not equal 1.");
	//if (!isOne(_matrixForm.trace())) throw NError("NMixedState: Matrix trace does not equal 1.");
#endif // __NSTRICT__
}

void NMixedState::addQCol(int start, int targets, const NPureState& newState) {
	// set the loop boundary indices
	start += _startIndex;
	uint end = start + targets;
	if ((uint)start == _startIndex) { // adjust for the diagonal element
		++start;
	}
    StateIterator it = _stateIterators[start];

	// calculate the traces
	for (uint i = start, j = _startIndex; i < end; ++i, ++it) {
		_Q(i,j) = trace(*it, newState).real();
	}
}

void NMixedState::addQRow(int start, int targets, const NPureState& newState) {
	// set the loop boundary indices
	start += _startIndex;
	uint end = start + targets;
	if (end == _lastIndex+1) { // adjust for the diagonal element
		--end;
	}
    StateIterator it = _stateIterators[start];

	// calculate the traces
	for (uint i = _lastIndex, j = start; j < end; ++j, ++it) {
		_Q(i,j) = trace(*it, newState).real();
	}
}

void NMixedState::buildIntermediateMatrix(int threadIndex, int start, int targets) {
	int i = _startIndex + start;
	int end = i + targets;
    StateIterator it = _stateIterators[i];
	_intermediateMatrices[threadIndex].noalias() = (it->matrix() * _probs[i]);
	for (++i, ++it; i < end; ++i, ++it) {
		_intermediateMatrices[threadIndex].noalias() += (it->matrix() * _probs[i]);
	}
}

/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
void addQColWrapper(int start, int targets, void* mixedStatePtr, const void* newState) {
	static_cast<NMixedState*>(mixedStatePtr)->addQCol(start, targets, *static_cast<const NPureState*>(newState));
}

void addQRowWrapper(int start, int targets, void* mixedStatePtr, const void* newState) {
	static_cast<NMixedState*>(mixedStatePtr)->addQRow(start, targets, *static_cast<const NPureState*>(newState));
}

void threadBuildMatrixWrapper(int threadIndex, int start, int targets, void* mixedStatePtr) {
	static_cast<NMixedState*>(mixedStatePtr)->buildIntermediateMatrix(threadIndex, start, targets);
}
