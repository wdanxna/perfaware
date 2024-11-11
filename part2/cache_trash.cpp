#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <CoreFoundation/CoreFoundation.h>

#include "metrics.cpp"
#include "repetition_tester.cpp"
#include "buffer.cpp"

extern "C" void Jump_Read(u64 count, u64 dist, u8* data);

int main() {
    u64 CPUTimerFreq = EstimateCPUFreq();
    constexpr u64 KiByte = 1024;
    constexpr u64 MiByte = 1024*1024;
    constexpr u64 GiByte = 1024*1024*1024;

    buffer mem = AllocateBuffer(1*MiByte);

    u64 distance[] = {2, 65, 128*KiByte, 128*KiByte+64};
    repetition_tester Testers[ArrayCount(distance)] = {};
    for (int i = 0; i < ArrayCount(distance); i++) {
        auto tester = &Testers[i];
        u64 dis = distance[i];
        NewTestWave(tester, mem.Count, CPUTimerFreq);
        while (IsTesting(tester)) {
            BeginTime(tester);
            Jump_Read(mem.Count, dis, mem.Data);
            EndTime(tester);
            CountBytes(tester, mem.Count);
        }

        auto minTime = tester->Results.MinTime;
        auto seconds = SecondsFromCPUTime(minTime, CPUTimerFreq);
        auto bytes = tester->TargetProcessedByteCount;
        auto throughput = bytes / (GiByte * seconds);

        printf("distance: %llu, BW: %.2f\n", dis, throughput);
    }

    return 0;
}