#include "timer.h"

Timer::Timer()
{
	gettimeofday(&m_time, NULL);
}

void Timer::Reset()
{
	gettimeofday(&m_time, NULL);
}

unsigned long Timer::GetTime() const
{
	timeval now;
	gettimeofday(&now, NULL);
	unsigned long micros = (now.tv_sec-m_time.tv_sec)*1000000+(now.tv_usec-m_time.tv_usec);
	return micros;
}

void Timer::Spin(unsigned long micros)
{
	Timer t;
	while (t.GetTime() < micros);
	return;
}
