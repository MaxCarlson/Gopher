#pragma once

#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <iostream>

#ifndef NOMINMAX
#  define NOMINMAX // Disable macros min() and max()
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOMINMAX

// Low level mutex lock/unlock
struct Mutex {
	Mutex() { InitializeCriticalSection(&cs); }
	~Mutex() { DeleteCriticalSection(&cs); }
	void lock() { EnterCriticalSection(&cs); }
	void unlock() { LeaveCriticalSection(&cs); }

private:
	CRITICAL_SECTION cs;
};

// Avoids printing to std::cout at the same time 
// when using multiple threads.

// Avoids printing to std::cout at the same time 
// when using multiple threads.
enum SyncOut { OUT_LOCK, OUT_UNLOCK };
std::ostream& operator<<(std::ostream&, SyncOut);

#define sync_out  std::cout << OUT_LOCK
#define sync_endl std::endl << OUT_UNLOCK; // TODO: Figure out how to do this without endl 
