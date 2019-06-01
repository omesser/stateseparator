#ifndef N_MIXED_STATE_H
#define N_MIXED_STATE_H

#include <list>
#include "NPureState.h"
#include "NThreadPool.h"

using std::list;


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
inline MatrixXcd operator-(const NMixedState& lhs, const NPureState& rhs);
inline MatrixXcd operator-(const NPureState& lhs, const NMixedState& rhs);
inline MatrixXcd operator-(const NMixedState& lhs, const MatrixXcd& rhs);
inline MatrixXcd operator-(const MatrixXcd& lhs, const NMixedState& rhs);


/*****************************************************************************
 *                             Class NMixedState                             *
 *****************************************************************************/
class NMixedState {
public:
	// Constructors
	NMixedState(const NPureState& src, const double& minProbForState, uint targetNumberOfStates,
			    const MatrixXcd& targetState);
	inline NMixedState(const NMixedState& src);

	// Assignment operators
	NMixedState& operator=(const NMixedState& src);

	// Modifiers
	// Insert the new state and rearrange all the probabilities in order to minimize the distance between
	// the new mixed state and the target state.
	// Returns the distance between the new mixed state and the target state.
	double mixIn(const NPureState& state);

	void sortStates();

	// Queries
	inline const MatrixXcd& matrix() const;
	double distance() const; // returns the current distance from the target state

	// Output functions
	void print() const;

private:
	// Type definitions
	typedef list<NPureState> StateList;
	typedef StateList::iterator StateIterator;
	typedef StateList::const_iterator CStateIterator;

	uint _size;					// Overall size of the state's density matrix.
	StateList _states;			// List of all the pure states in this mixed state.
	uint _numOfStates;			// Number of pure states in the list.
	MatrixXcd _matrixForm;		// This is the matrix form of the mixed state.
	double _minProbForState;	// Disacrd states with respective probabilites below this threshold.
    uint _targetNumberOfStates;	// Confine the solution to contain at most this many states.
	MatrixXcd _targetState;		// The mixed state will eventually be an approximation of this density matrix.
	MatrixXd _Q;				// Q_ij = 2*trace(rho_p_i * rho_p_j)
	VectorXd _p;				// p_i = 2*trace(rho_p_i * rho)
	VectorXd _probs;			// Probabilities vector - this is the result vector of the QP optimization.
	uint _startIndex;			// Index of the first state in the Q matrix.
								// It also represents the number of "missing" entries at the beginning of the state list.
	uint _lastIndex;			// Index of the last state in the Q matrix.
    vector<StateIterator> _stateIterators;
                                // An array of StateList iterators, pointed to by their respective index in the Q matrix.
	vector<MatrixXcd> _intermediateMatrices;
                                // An array of matrices used by the multi-threaded buildMatrix function.
	NThreadPool* _tp;			// Pointer to the thread pool.
	int _addQColAction;			// The action number within the thread pool for adding a new column to Q.
	int _addQRowAction;			// The action number within the thread pool for adding a new row to Q.
	int _buildMatrixAction;		// The action number within the thread pool for building an intermediate matrix.

	// Bookkeeping functions
	inline void addState(const NPureState& newState);
	void addFront(const NPureState& newState);
	void addBack(const NPureState& newState);
	inline void eraseState(StateIterator& it);
	inline void compress(uint violatingIndex);
	void shiftUp(uint violatingIndex);
	void shiftDown(uint violatingIndex);
    inline void buildIteratorsArray();
	void quickSortStates(uint left, uint right, StateIterator it_start, StateIterator it_end);

	// Probabilities update functions
	void removeMinState();

	// QP optimization functions
	void minimize();
	uint findSolution();

	// Utility functions
    void buildMatrix();

	// ThreadPool functions
	friend void addQColWrapper(int, int, void*, const void*);
	friend void addQRowWrapper(int, int, void*, const void*);
	friend void threadBuildMatrixWrapper(int, int, int, void*);
	void addQCol(int start, int targets, const NPureState& newState);
	void addQRow(int start, int targets, const NPureState& newState);
	void buildIntermediateMatrix(int threadIndex, int start, int targets);
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NMixedState::NMixedState(const NMixedState& src)
: _size(src._size), _states(src._states), _numOfStates(src._numOfStates), _matrixForm(src._matrixForm),
  _minProbForState(src._minProbForState), _targetNumberOfStates(src._targetNumberOfStates),
  _targetState(src._targetState), _Q(src._Q), _p(src._p),
  _probs(src._probs), // Copying the array is not really necessary (allocation would have sufficed), simply for completeness
  _startIndex(src._startIndex), _lastIndex(src._lastIndex), _stateIterators(src._stateIterators.size()),
  // Copying the intermediate matrices array is not really necessary (allocation would have sufficed), simply for completeness
  _intermediateMatrices(src._intermediateMatrices),
  _tp(src._tp), _addQColAction(src._addQColAction), _addQRowAction(src._addQRowAction),
  _buildMatrixAction(src._buildMatrixAction)
{
    buildIteratorsArray();
}

const MatrixXcd& NMixedState::matrix() const {
	return _matrixForm;
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
void NMixedState::addState(const NPureState& newState) {
	if (_startIndex > 0) {
		addFront(newState);
	}
	else {
		addBack(newState);
	}
}

void NMixedState::eraseState(StateIterator& it) {
	StateIterator tempIt = it;
	++it;
	_states.erase(tempIt);
	--_numOfStates;
}

/* void NMixedState::compress(uint violatingIndex);
 *
 * This function removes one state and its corresponding entry from Q and p. It then shifts the Q and p entries to
 * eliminate the gap that was formed (see shift explanations below).
 *
 * Shift up means taking all the indices below the violating index and moving them up +1.
 * Shift down means taking all the indices above the violating index and moving them down -1.
 * To determine which shift to perform, we take a look at the number of elements below the diagonal which do not
 * move below and above the violating index and shift towards that which is larger.
 *
 * Notation:
 * N = _numOfStates
 * v = violatingIndex
 * m = v + 1
 *
 *     below v           above v
 * (m-1)^2 - (m-1)   (N-m)^2 - (N-m)             N-1
 * --------------- < ---------------   ==>   v < ---
 *        2                 2                     2
 * 
 * If the above condition holds, we should do a shift up because the number of stationary elements above the violating
 * index is larger than the number of stationary elements below the violating index.
 */
void NMixedState::compress(uint violatingIndex) {
	if ((violatingIndex - _startIndex) < ((_numOfStates - 1)/2)) {
		shiftUp(violatingIndex);
	}
	else {
		shiftDown(violatingIndex);
	}
}

void NMixedState::buildIteratorsArray() {
    StateIterator it = _states.begin();
    for (uint i = _startIndex; i <= _lastIndex; ++i, ++it) {
        _stateIterators[i] = it;
    }
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
MatrixXcd operator-(const NMixedState& lhs, const NPureState& rhs) {
	return lhs.matrix() - rhs.matrix();
}

MatrixXcd operator-(const NPureState& lhs, const NMixedState& rhs) {
	return lhs.matrix() - rhs.matrix();
}

MatrixXcd operator-(const NMixedState& lhs, const MatrixXcd& rhs) {
	return lhs.matrix() - rhs;
}

MatrixXcd operator-(const MatrixXcd& lhs, const NMixedState& rhs) {
	return lhs - rhs.matrix();
}

#endif // N_MIXED_STATE_H
