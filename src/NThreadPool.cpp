#include "NThreadPool.h"

NThreadPool* NThreadPool::_theInstance = NULL;
NThreadPool::runfunc NThreadPool::_runFuncs[NAT_END] = {run_iivcv, run_iiiv};


/*****************************************************************************
 *                          PUBLIC MEMBER FUNCTIONS                          *
 *****************************************************************************/
void NThreadPool::execute(int action, int targets, void* object, const void* cvInput) {
	// TODO: this should be handled better, need to build a mechanism for waking up a subset of the threads
	if (targets < _numOfThreads) {
		// don't bother waking up the threads if there is not enough work
		_iivcvFuncs[action](0, targets, object, cvInput);
		return;
	}

	// Calculate the number of targets for each thread.
	_targetsPerThread = targets / _numOfThreads;
	_targetDelta = targets % _numOfThreads;

	// Prepare the input for the threads.
	_actionNum = action;
	_actionType = NAT_IIVCV;
	_currentObject = object;
	_cvInput = cvInput;

	// Wake up the threads to allow them to do the work.
	wakeupThreads();

	// Do the work for the main thread.
	_iivcvFuncs[action](0, _targetsPerThread + _targetDelta, object, cvInput);

	// Wait for all secondary threads to complete their work.
	waitForThreads();
}

void NThreadPool::execute(int action, int targets, void* object) {
	// TODO: this should be handled better, need to build a mechanism for waking up a subset of the threads
	if (targets < _numOfThreads) {
		// don't bother waking up the threads if there is not enough work
		_iiivFuncs[action](0, 0, targets, object);
		return;
	}

	// Calculate the number of targets for each thread.
	_targetsPerThread = targets / _numOfThreads;
	_targetDelta = targets % _numOfThreads;

	// Prepare the input for the threads.
	_actionNum = action;
	_actionType = NAT_IIIV;
	_currentObject = object;

	// Wake up the threads to allow them to do the work.
	wakeupThreads();

	// Do the work for the main thread.
	_iiivFuncs[action](0, 0, _targetsPerThread + _targetDelta, object);

	// Wait for all secondary threads to complete their work.
	waitForThreads();
}


/*****************************************************************************
 *                          PRIVATE MEMBER FUNCTIONS                         *
 *****************************************************************************/
void NThreadPool::secondaryThreadMain(int threadIndex) {
	notifyThreadCreated(threadIndex);
	while (true) {
		// Sleep here until awakened by the main thread.
		waitForWork();

		// Do the assigned work.
		_runFuncs[_actionType](this, threadIndex);

		// Notify the main thread that this thread has completed its work.
		notifyWorkFinished(threadIndex);
	}
}

void NThreadPool::createThreads() {
	vector<int> indices(_numOfThreads);
	for (int i = 1; i < _numOfThreads; ++i) {
		indices[i] = i;
		createThread(&indices[i]);
	}
	waitForThreads();
	NLOGNUM("NThreadPool: Number of secondary threads created: ", _numOfSecondaryThreads);
}
