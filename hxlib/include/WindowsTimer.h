#ifndef __WINDOWSTIMER_H__
#define __WINDOWSTIMER_H__

#include <Windows.h>

class WindowsTimer {
public:
	WindowsTimer() {
		reset();
	}

	inline void reset() {
		m_last_time = m_start_time = GetTickCount();
	}
	inline unsigned getMicroseconds() {
		unsigned current_time = GetTickCount();
		unsigned time_interval = current_time - m_last_time;
		m_last_time = current_time;
		return time_interval;
	}

protected:
	unsigned m_start_time;
	unsigned m_last_time;
};

#endif	/* __WINDOWSTIMER_H__ */