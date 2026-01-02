#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include "NThreadPool.h"


// TODO: ideally these would be members of the class, but for performance reasons I didn't want to use inheritance.
//		 so this is somewhat of a compromise.
pthread_mutex_t _workLock;
pthread_cond_t _workAvailable;

// Thread-ready synchronization (replaces deprecated sem_t on macOS)
pthread_mutex_t _readyLock;
pthread_cond_t _readyCond;
int* _threadReady = NULL;  // Array of flags: 0 = busy, 1 = ready

volatile int _numOfSleepingThreads = 0;  // TODO: not sure this is necessary


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
// OS-specific wrapper for the threadpool's secondary threads' main function
inline void* startLinuxSecondaryThread(void* threadIndexPtr);


/*****************************************************************************
 *                    OS-SPECIFIC PRIVATE MEMBER FUNCTIONS                   *
 *****************************************************************************/
void NThreadPool::initThreadPool() {
	// Initialize thread-ready flags (replaces semaphores for macOS compatibility)
	_threadReady = new int[_numOfThreads];
	for (int i = 0; i < _numOfThreads; ++i) {
		_threadReady[i] = 0;  // Not ready initially
	}
	if (pthread_mutex_init(&_readyLock, NULL) != 0) {
		throw NError("NThreadPool: Failed to initialize the _readyLock mutex.");
	}
	if (pthread_cond_init(&_readyCond, NULL) != 0) {
		throw NError("NThreadPool: Failed to initialize the _readyCond condition variable.");
	}
	if (pthread_mutex_init(&_workLock, NULL) != 0) {
		throw NError("NThreadPool: Failed to initialize the _workLock mutex.");
	}
	if (pthread_cond_init(&_workAvailable, NULL) != 0) {
		throw NError("NThreadPool: Failed to initialize the _workAvailable condition variable.");
	}
}

void NThreadPool::notifyThreadCreated(int threadIndex) {
	pthread_mutex_lock(&_readyLock);
	_threadReady[threadIndex] = 1;
	pthread_cond_broadcast(&_readyCond);
	pthread_mutex_unlock(&_readyLock);
}

int NThreadPool::numOfCPUs() const {
	return sysconf(_SC_NPROCESSORS_ONLN);
}

void NThreadPool::createThread(int* threadIndexPtr) {
	pthread_t tid;
	if (pthread_create(&tid, NULL, startLinuxSecondaryThread, threadIndexPtr) < 0) {
		throw NError("NThreadPool: Failed to create secondary thread.");
	}
}

void NThreadPool::wakeupThreads() {
	while (_numOfSleepingThreads != _numOfSecondaryThreads) {
		sched_yield();
	}
	if (pthread_cond_broadcast(&_workAvailable) < 0) {
		throw NError("NThreadPool: Failed to wake up threads.");
	}
}

void NThreadPool::waitForThreads() {
	pthread_mutex_lock(&_readyLock);
	for (int i = 1; i < _numOfThreads; ++i) {
		while (_threadReady[i] == 0) {
			pthread_cond_wait(&_readyCond, &_readyLock);
		}
		_threadReady[i] = 0;  // Reset for next round
	}
	pthread_mutex_unlock(&_readyLock);
}

void NThreadPool::waitForWork() {
	if (pthread_mutex_lock(&_workLock) < 0) {
		throw NError("NThreadPool: Failed to acquire the _workLock mutex.");
	}
	++_numOfSleepingThreads;
	if (pthread_cond_wait(&_workAvailable, &_workLock) < 0) {
		throw NError("NThreadPool: Failed to wait for work.");
	}
	--_numOfSleepingThreads;
	if (pthread_mutex_unlock(&_workLock) < 0) {
		throw NError("NThreadPool: Failed to release the _workLock mutex.");
	}
}

void NThreadPool::notifyWorkFinished(int threadIndex) {
	pthread_mutex_lock(&_readyLock);
	_threadReady[threadIndex] = 1;
	pthread_cond_broadcast(&_readyCond);
	pthread_mutex_unlock(&_readyLock);
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
void* startLinuxSecondaryThread(void* threadIndexPtr) {
	int threadIndex = *static_cast<int*>(threadIndexPtr);
	startSecondaryThread(threadIndex);  // never returns
	return NULL;
}
