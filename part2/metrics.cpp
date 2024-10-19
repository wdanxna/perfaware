#include "common.h"
// #include <x86intrin.h>
#include <sys/time.h>

//for apple sillicon
//https://lemire.me/blog/2023/03/21/counting-cycles-and-instructions-on-arm-based-apple-systems/
#if __APPLE__ && __arm64__
#include "apple_arm_events.h"
#endif

static u64 GetOSTimerFreq(void)
{
	return 1000000;
}

static u64 ReadOSTimer(void)
{
	// NOTE(casey): The "struct" keyword is not necessary here when compiling in C++,
	// but just in case anyone is using this file from C, I include it.
	struct timeval Value;
	gettimeofday(&Value, 0);
	
	u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
	return Result;
}


#if __APPLE__ && __arm64__
    AppleEvents apple;
#endif
/* NOTE(casey): This does not need to be "inline", it could just be "static"
   because compilers will inline it anyway. But compilers will warn about 
   static functions that aren't used. So "inline" is just the simplest way 
   to tell them to stop complaining about that. */
inline u64 ReadCPUTimer(void)
{
	// NOTE(casey): If you were on ARM, you would need to replace __rdtsc
	// with one of their performance counter read instructions, depending
	// on which ones are available on your platform.
#if __APPLE__ && __arm64__
    bool setup = apple.setup_performance_counters();
    // assert(setup);
    if (!setup) {
        return mach_absolute_time();
    }
    auto counter = apple.get_counters();
    return counter.cycles;
#elif
	return __rdtsc();
#endif
}

u64 EstimateCPUFreq() {
	ReadCPUTimer();//init
    //first calibrate with os time
    u64 MillisecondsToWait = 100;
    u64 OSFreq = GetOSTimerFreq();
    u64 CPUStart = ReadCPUTimer();
    u64 OSStart = ReadOSTimer();
    u64 OSEnd = 0;
    u64 OSElapsed = 0;
    u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
    while (OSElapsed < OSWaitTime) {
        OSEnd = ReadOSTimer();
        OSElapsed = OSEnd - OSStart;
    }
    u64 CPUEnd = ReadCPUTimer();
    u64 CPUElapsed = CPUEnd - CPUStart;
    u64 CPUFreq = 0; //how many 'clocks' per second
    if (OSElapsed) {
        CPUFreq = OSFreq * CPUElapsed / OSElapsed;
    }

	// printf("    OS Timer: %llu -> %llu = %llu elapsed\n", OSStart, OSEnd, OSElapsed);
    // printf(" OS Seconds: %.4f\n", (f64)OSElapsed/(f64)OSFreq);

    // printf("    CPU Timer: %llu -> %llu = %llu elapsed\n", CPUStart, CPUEnd, CPUElapsed);
    // printf(" CPU Freq: %llu (guessed)\n", CPUFreq);
	return CPUFreq;
}
