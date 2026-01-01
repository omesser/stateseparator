#ifndef N_THREAD_POOL_H
#define N_THREAD_POOL_H

#include <vector>
#include <climits>
#include "NError.h"

using std::vector;


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
// OS-independent Wrapper for the threadpool's secondary threads' main function
inline void startSecondaryThread(int threadIndex);


/*****************************************************************************
 *                             Class NThreadPool                             *
 *****************************************************************************/
class NThreadPool {
public:
	// TODO: handle errors in the threadpool module

	// TODO: document this better
	/* typedef void (*actionFunc)(int, int, void*);
	 *
	 * An action function operates on a group of targets typically organized in an array. Each thread
	 * will iterate through its own dedicated partition in the array. Load balancing is achieved by
	 * assigning each thread an equal number of targets.
	 *
	 * param [in]	int		the starting index for this thread in the general array
	 * param [in]	int		the number of targets this thread should handle
	 * param [in]	void*	the object which contains the array
	 * param [in]	void*	input for the function being called in the object
	 */
	typedef void (*iivcv)(int, int, void*, const void*);
	typedef void (*iiiv)(int, int, int, void*);

	inline static NThreadPool* getInstance();

	// Queries
	inline int numOfThreads() const;

	// Actions
	inline int registerAction(iivcv func);
	inline int registerAction(iiiv func);
	void execute(int action, int targets, void* object, const void* cvInput);
	void execute(int action, int targets, void* object);

private:
	// The singleton instance
	static NThreadPool* _theInstance;

	// Type definitions
	typedef void (*runfunc)(NThreadPool*, int);
	enum ActionType { NAT_IIVCV = 0, NAT_IIIV, NAT_END };

	// Data members
	int _numOfThreads;
	int _numOfSecondaryThreads;
	int _targetsPerThread;
	int _targetDelta;
	void* _currentObject;
	const void* _cvInput;
	ActionType _actionType;
	int _actionNum;
	vector<iivcv> _iivcvFuncs;
	vector<iiiv> _iiivFuncs;
	static runfunc _runFuncs[NAT_END];

	// Constructor
	inline NThreadPool();

	// OS-specific initialization function for the thread pool
	void initThreadPool();

	// Secondary threads' main function
	// TODO: not sure this function is necessary
	friend void startSecondaryThread(int threadIndex);
	void secondaryThreadMain(int threadIndex);  // never returns
	void notifyThreadCreated(int threadIndex);

	// Run functions
	inline static void run_iivcv(NThreadPool* tp, int start);
	inline static void run_iiiv(NThreadPool* tp, int start);

	// Utility functions
	void createThreads();

	// OS-specific utility functions
	int numOfCPUs() const;
	void createThread(int* threadIndexPtr);
	void wakeupThreads();
	void waitForThreads();
	void waitForWork();
	void notifyWorkFinished(int threadIndex);

	// Masking copy constructor and assignment operator
	inline NThreadPool(const NThreadPool& src);     // ERROR
	inline void operator=(const NThreadPool& src);  // ERROR
};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
NThreadPool* NThreadPool::getInstance() {
	if (_theInstance == NULL) {
		_theInstance = new NThreadPool;
		_theInstance->createThreads();
	}
	return _theInstance;
}

int NThreadPool::numOfThreads() const {
	return _numOfThreads;
}

int NThreadPool::registerAction(iivcv func) {
	if (_iivcvFuncs.size() > INT_MAX) {
		throw("NThreadPool: size of actionNum exceeded max int! Something went wrong.");
	}
	int actionNum = static_cast<int>(_iivcvFuncs.size());
	_iivcvFuncs.push_back(func);
	return actionNum;
}

int NThreadPool::registerAction(iiiv func) {
	if (_iiivFuncs.size() > INT_MAX) {
		throw("NThreadPool: size of actionNum exceeded max int! Something went wrong.");
	}
	int actionNum = static_cast<int>(_iiivFuncs.size());
	_iiivFuncs.push_back(func);
	return actionNum;
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
NThreadPool::NThreadPool()
    : _numOfThreads(numOfCPUs()),
      _numOfSecondaryThreads(_numOfThreads - 1),
      _targetsPerThread(0),
      _targetDelta(0),
      _currentObject(NULL),
      _cvInput(NULL),
      _actionType(NAT_END),
      _actionNum(0) {
	initThreadPool();
}

void NThreadPool::run_iivcv(NThreadPool* tp, int threadIndex) {
	int start =
	    (threadIndex * tp->_targetsPerThread) + tp->_targetDelta;  // calculate the starting index for this thread
	tp->_iivcvFuncs[tp->_actionNum](start, tp->_targetsPerThread, tp->_currentObject, tp->_cvInput);
}

void NThreadPool::run_iiiv(NThreadPool* tp, int threadIndex) {
	int start =
	    (threadIndex * tp->_targetsPerThread) + tp->_targetDelta;  // calculate the starting index for this thread
	tp->_iiivFuncs[tp->_actionNum](threadIndex, start, tp->_targetsPerThread, tp->_currentObject);
}

NThreadPool::NThreadPool(const NThreadPool& src) {
	throw NError("NThreadPool: Copy constructor was invoked.");
}

void NThreadPool::operator=(const NThreadPool& src) {
	throw NError("NThreadPool: Assignment operator was invoked.");
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
void startSecondaryThread(int threadIndex) {
	NThreadPool::_theInstance->secondaryThreadMain(threadIndex);
}

#endif  // N_THREAD_POOL_H
