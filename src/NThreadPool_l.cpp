#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include "NThreadPool.h"


// TODO: ideally these would be members of the class, but for performance reasons I didn't want to use inheritance.
//		 so this is somewhat of a compromise.
pthread_mutex_t _workLock;
pthread_cond_t _workAvailable;
sem_t* _threadsReady = NULL;
volatile int _numOfSleepingThreads = 0; // TODO: not sure this is necessary


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
// OS-specific wrapper for the threadpool's secondary threads' main function
inline void* startLinuxSecondaryThread(void* threadIndexPtr);


/*****************************************************************************
 *                    OS-SPECIFIC PRIVATE MEMBER FUNCTIONS                   *
 *****************************************************************************/
void NThreadPool::initThreadPool() {
	_threadsReady = new sem_t[_numOfThreads];
	for (int i = 0; i < _numOfThreads; ++i) {
		if (sem_init(&_threadsReady[i], 0, 0) < 0) {
			throw NError("NThreadPool: Failed to initialize a semaphore.");
		}
	}
	if (pthread_mutex_init(&_workLock, NULL) < 0) {
		throw NError("NThreadPool: Failed to initialize the _workLock mutex.");
	}
	if (pthread_cond_init(&_workAvailable, NULL) < 0) {
		throw NError("NThreadPool: Failed to initialize the _workAvailable condition variable.");
	}
}

void NThreadPool::notifyThreadCreated(int threadIndex) {
	if (sem_post(&_threadsReady[threadIndex]) < 0) {
		throw NError("NThreadPool: Failed to post a semaphore.");
	}
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
	for (int i = 1; i < _numOfThreads; ++i) {
		if (sem_wait(&_threadsReady[i]) < 0) {
			throw NError("NThreadPool: Failed to wait on a semaphore.");
		}
	}
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
	if (sem_post(&_threadsReady[threadIndex]) < 0) {
		throw NError("NThreadPool: Failed to post a semaphore.");
	}
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
void* startLinuxSecondaryThread(void* threadIndexPtr) {
	int threadIndex = *static_cast<int*>(threadIndexPtr);
	startSecondaryThread(threadIndex); // never returns
	return NULL;
}
