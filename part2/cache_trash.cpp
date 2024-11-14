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
extern "C" void Stride_Read(u64 outerCount, u64 innerCount, u64 stride, u8* data);

int main() {
    u64 CPUTimerFreq = EstimateCPUFreq();
    constexpr u64 KiByte = 1024;
    constexpr u64 MiByte = 1024*1024;
    constexpr u64 GiByte = 1024*1024*1024;

#if 0
    buffer mem = AllocateBuffer(1*MiByte);

    u64 distance[] = {0, 64, 128, 192, 256, 320, 384, 448, 512, 128*KiByte, 128*KiByte+64};
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
#endif

    //lets reproduce Casey's design
    //128 strides to experiment on
    repetition_tester Testers[128] = {};
    u64 cacheLineSize = 64;
    u64 repCount = 64;//outer loop always repeate 64 times
    u64 readCount = 256;//inner loop acount

    u64 totalBytes = repCount * readCount * cacheLineSize;
    buffer memory = AllocateBuffer(readCount * cacheLineSize * ArrayCount(Testers));

    for (int strideIndex = 0; strideIndex < ArrayCount(Testers); strideIndex++) {
        u64 stride = strideIndex * cacheLineSize;
        auto tester = &Testers[strideIndex];
        NewTestWave(tester, totalBytes, CPUTimerFreq);
        while (IsTesting(tester)) {
            BeginTime(tester);
            Stride_Read(repCount, readCount, stride, memory.Data);
            EndTime(tester);
            CountBytes(tester, totalBytes);
        }

        auto minTime = tester->Results.MinTime;
        auto seconds = SecondsFromCPUTime(minTime, CPUTimerFreq);
        auto bytes = tester->TargetProcessedByteCount;
        auto throughput = bytes / (GiByte * seconds);
        printf("%llu, %.2f\n", stride, throughput);
    }
    return 0;
}