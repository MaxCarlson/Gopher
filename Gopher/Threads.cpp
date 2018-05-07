#include "Threads.h"


std::ostream& operator<<(std::ostream& os, SyncOut sc)
{
	static Mutex m;

	if (sc == OUT_LOCK)
		m.lock();

	if (sc == OUT_UNLOCK)
		m.unlock();

	return os;
}



