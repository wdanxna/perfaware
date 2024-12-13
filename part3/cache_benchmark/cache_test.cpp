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

constexpr static u64 MakeMultipleOf(u64 base, u64 k) {
    return ((base+k-1)/k)*k;
}

#define M(x) MakeMultipleOf(x, 96)

//little struct to record test result
struct TesteData {
    repetition_tester tester;
    u64 region;
    u64 alignment;
    u64 throughput;
};

extern "C" void DoubleLoop_Cache_Test(u64 outer, u64 inner, u8* data);

int main() {
    u64 CPUTimerFreq = EstimateCPUFreq();
    constexpr u64 KiByte = 1024;
    constexpr u64 MiByte = 1024*1024;
    constexpr u64 GiByte = 1024*1024*1024;
    
    buffer memory = AllocateBuffer(M(GiByte));
    //seems like the address returned by malloc is alwalys 64B aligned.
    assert((u64)memory.Data % 64 == 0);

    for (int i = 0; i < memory.Count; i++) {
        memory.Data[i] = i;
    }


    constexpr u64 regions[] = { M(8*KiByte),M(2*MiByte), M(10*MiByte), M(1*GiByte)};
    constexpr u64 alignments[] = {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65};

    TesteData Testers[ArrayCount(regions)][ArrayCount(alignments)] = {};

    for (u32 regionIndex = 0; regionIndex < ArrayCount(regions); regionIndex++) {
            u64 outerCount = memory.Count / regions[regionIndex];
            u64 innerCount = regions[regionIndex] / 96;

        for (u32 alignmentIndex = 0; alignmentIndex < ArrayCount(alignments); alignmentIndex++) {
            TesteData* tester = &Testers[regionIndex][alignmentIndex];
            u64 alignment = alignments[alignmentIndex];
            tester->alignment = alignment;
            tester->region = regions[regionIndex];
            NewTestWave(&tester->tester, outerCount * innerCount * 96, CPUTimerFreq);
            while (IsTesting(&tester->tester)) {
                BeginTime(&tester->tester);
                DoubleLoop_Cache_Test(outerCount, innerCount, memory.Data + alignment);
                EndTime(&tester->tester);
                CountBytes(&tester->tester, outerCount * innerCount * 96);
            }

            auto minTime = tester->tester.Results.MinTime;
            auto seconds = SecondsFromCPUTime(minTime, CPUTimerFreq);
            auto bytes = tester->tester.TargetProcessedByteCount;
            auto throughput = bytes / (GiByte * seconds);
            tester->throughput = throughput;
            printf("region: %llu, alignment: %llu, Bw: %.2f GB/s\n", regions[regionIndex], alignment, throughput);
        }
    }

    printf("Region(Byte), Offset, Throughput\n");
    for (u32 regionIndex = 0; regionIndex < ArrayCount(regions); regionIndex++) {
        for (u32 alignmentIndex = 0; alignmentIndex < ArrayCount(alignments); alignmentIndex++) {
            TesteData* tester = &Testers[regionIndex][alignmentIndex];
            printf("%llu, %llu, %llu\n", tester->region, tester->alignment, tester->throughput);
        }
    }
    return 0;
}