#include "common.h"
#include <mach/mach_time.h>
#include "metrics.cpp"

int main(int argc, char** agrv) {
    //estimate performance counter's frequency
    u64 OSFreq = GetOSTimerFreq();
    u64 OSTimeStart = ReadOSTimer();
    u64 PerfStart = mach_absolute_time();
    u64 OSTimeEnd = 0;
    u64 OSTimeElapsed = 0;
    u64 MillisecondsToWait = 100;
    while (OSTimeElapsed <  OSFreq * MillisecondsToWait / 1000) {
        OSTimeEnd = ReadOSTimer();
        OSTimeElapsed = OSTimeEnd - OSTimeStart;
    }
    u64 PerfEnd = mach_absolute_time();
    u64 PerfFreq = (OSFreq * (PerfEnd - PerfStart)/OSTimeElapsed);

    printf("PerfFreq: %llu", PerfFreq);//about 24MHz
    return 0;
}