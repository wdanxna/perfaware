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

#define MYDESIGN 1
#define REPRODUCE 0


extern "C" void Normal_Write(u64, u64, u8*, u8*);
extern "C" void Nontemporal_Write(u64, u64, u8*, u8*);

int main() {
    u64 CPUTimerFreq = EstimateCPUFreq();
    constexpr u64 KiByte = 1024;
    constexpr u64 MiByte = 1024*1024;
    constexpr u64 GiByte = 1024*1024*1024;

    //experiment for nontemporal write instruction
    //write a loop to load 128 bytes from one pointer and write to another
    //write the code in 2 different versions, one for normal write, the other for nontemporal write
    //compare their throughtput differences.

    //we still confrom to the double loop design, in which the outer loop controls the repetition count
    //the inner conducts the read/write behaviour.
    using test_function = void (*)(u64 outer, u64 inner, u8* read, u8* write);

    struct Test {
        const char* name;
        test_function func;
    };

    Test tests[] = {
        // {"Normal_Write", Normal_Write},
        {"Nontemporal_Write", Nontemporal_Write}
    };

    buffer SourceBuffer = AllocateBuffer(MiByte);
    buffer DestBuffer = AllocateBuffer(GiByte);

    for (u32 Index = 0; Index < SourceBuffer.Count; ++Index) {
        SourceBuffer.Data[Index] = (u8)Index;
    }

    for (u32 Index = 0; Index < DestBuffer.Count; ++Index) {
        DestBuffer.Data[Index] = 255 - (u8)Index;
    }

    u64 InnerSize = 128; //how many bytes to read per inner loop
    // test on various read footprint, ranging from 128 to 1MB
    printf("Source size, Normal, Non-temporal\n");
    for (u32 SourceSize = 128; SourceSize <= SourceBuffer.Count; SourceSize *= 2) {
        u64 InnerCount = SourceSize / InnerSize;
        u64 OuterCount = DestBuffer.Count/(InnerCount*InnerSize);
        printf("%u", SourceSize);
        for (u32 TestIndex = 0; TestIndex < ArrayCount(tests); ++TestIndex) {
            auto test = tests + TestIndex;
            repetition_tester tester = {};
            NewTestWave(&tester, OuterCount*InnerCount*InnerSize, CPUTimerFreq);
            while (IsTesting(&tester)) {
                BeginTime(&tester);
                test->func(OuterCount, InnerCount, SourceBuffer.Data, DestBuffer.Data);
                EndTime(&tester);
                CountBytes(&tester, OuterCount*InnerCount*InnerSize);
            }

            auto seconds = SecondsFromCPUTime(tester.Results.MinTime, CPUTimerFreq);
            auto bytes = tester.TargetProcessedByteCount;
            auto throughput = bytes / (seconds * GiByte);
            printf(", %.4f", throughput);
            // printf("%s, src:%u, %.4f\n", test->name, SourceSize, throughput);
        }
        printf("\n");
    }

}