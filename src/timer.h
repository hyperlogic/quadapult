#ifndef TIMER_H
#define TIMER_H

#include "abaci.h"
#include <sys/time.h>

#define NUM_TIMER_SAMPLES 30

#define TIMER_DEF(name)										\
	static unsigned long name##micros[NUM_TIMER_SAMPLES];	\
	static Timer name##timer;

#define TIMER_START(name) name##timer.Reset();

#define TIMER_STOP(name, frame_counter)	\
	name##micros[frame_counter % NUM_TIMER_SAMPLES] = name##timer.GetTime();

#define TIMER_REPORT(name, frame_counter)								\
	if (frame_counter > 0 && (frame_counter % NUM_TIMER_SAMPLES) == 0)	\
	{																	\
	    unsigned long sum = 0;											\
		for (int i = 0; i < NUM_TIMER_SAMPLES; i++)						\
            sum += name##micros[i];										\
		printf("%s = %ld\n", #name, sum / NUM_TIMER_SAMPLES);			\
	}

	

class Timer
{
public:

	// busy wait for a period of time in microseconds.
	static void Spin(unsigned long micros);

	Timer();
	void Reset();
	// returns time since construction or Reset in microseconds.
	unsigned long GetTime() const;
protected:
	timeval m_time;
};

#endif
