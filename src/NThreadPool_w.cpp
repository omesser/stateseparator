#include <Windows.h>
#include "NThreadPool.h"

// TODO: ideally these would be members of the class, but for performance reasons I didn't want to use inheritance.
//		 so this is somewhat of a compromise.
HANDLE _workAvailable;
HANDLE* _threadsReady = NULL;


/*****************************************************************************
 *                              GLOBAL FUNCTIONS                             *
 *****************************************************************************/
// OS-specific wrapper for the threadpool's secondary threads' main function
inline static DWORD WINAPI startWindowsSecondaryThread(LPVOID threadIndexPtr);


/*****************************************************************************
 *                    OS-SPECIFIC PRIVATE MEMBER FUNCTIONS                   *
 *****************************************************************************/
void NThreadPool::initThreadPool() {
	_threadsReady = new HANDLE[_numOfThreads];
	_threadsReady[0] = CreateEvent(NULL, true, false, NULL);
	for (int i = 1; i < _numOfThreads; ++i) {
		_threadsReady[i] = CreateEvent(NULL, false, false, NULL);
	}
	_workAvailable = CreateEvent(NULL, true, false, NULL);
}

int NThreadPool::numOfCPUs() const {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
}

void NThreadPool::createThread(int* threadIndexPtr) {
	HANDLE hthread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startWindowsSecondaryThread, threadIndexPtr,
								  0, NULL);
}

void NThreadPool::wakeupThreads() {
	// Wait until all threads are ready for work.
	WaitForMultipleObjects(_numOfSecondaryThreads, &_threadsReady[1], true, INFINITE);		

	
	// The secondary threads will wait on this event after they are done to allow the main thread
	// to reset the _workAvailable event. Failing to do so may cause a secondary thread to begin
	// working again on the same data.
	ResetEvent(_threadsReady[0]);
	
	// Notify the other threads that work is available.
	SetEvent(_workAvailable);
}

void NThreadPool::waitForThreads() {
	// Wait until all the secondary threads are done. If we reset the _workAvailable event before that,
	// we might reset it before a secondary thread was even released from its wait, thus causing a deadlock.
	WaitForMultipleObjects(_numOfSecondaryThreads, &_threadsReady[1], true, INFINITE);

	// Now, reset the _workAvailable event when all the secondary threads are waiting on _threadsReady[0].
	ResetEvent(_workAvailable);

	// Release the secondary threads.
	SetEvent(_threadsReady[0]);
}

void NThreadPool::waitForWork() {
	WaitForSingleObject(_workAvailable, INFINITE);
}

void NThreadPool::notifyWorkFinished(int threadIndex) {
	// Notify the main thread that the work is done.
	SetEvent(_threadsReady[threadIndex]);

	// Allow the main thread to reset the _workAvailable event.
	WaitForSingleObject(_threadsReady[0], INFINITE);

	// Notify the main thread that this thread is ready for work again. Skipping this step could
	// result in a deadlock if the main thread resets _threadsReady[0] before this thread has been
	// released from the wait above.
	SetEvent(_threadsReady[threadIndex]);
}

void NThreadPool::notifyThreadCreated(int threadIndex) {
	// Notify the main thread that the work is done.
	SetEvent(_threadsReady[threadIndex]);

	// Allow the main thread to reset the _workAvailable event.
	WaitForSingleObject(_threadsReady[0], INFINITE);

	// Notify the main thread that this thread is ready for work again. Skipping this step could
	// result in a deadlock if the main thread resets _threadsReady[0] before this thread has been
	// released from the wait above.
	SetEvent(_threadsReady[threadIndex]);
}


/*****************************************************************************
 *                      GLOBAL FUNCTIONS IMPLEMENTATION                      *
 *****************************************************************************/
DWORD WINAPI startWindowsSecondaryThread(LPVOID threadIndexPtr) {
	int threadIndex = *reinterpret_cast<int*>(threadIndexPtr);
	startSecondaryThread(threadIndex); // never returns
	return 0;
}
