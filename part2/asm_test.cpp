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

//this file is used to setup the repetion test for assembly functions, unlike the repetition_test.cpp
//To make things simpler, this file tends to elimiate the wrapper, and provides the ability to test
//assembly routines directly, thus we need to change the test framework a little bit, 
//especially the test function prototype.

enum AllocTypes :u32 {
    AllocTypeNone = 0,
    AllocTypeMalloc,//malloc & free every file read
    AllocTypeCount
};

using test_func = void (u64 , u8*);

struct test_function {
    const char *Name;
    test_func *Func;
};

static const char *AllocationDescription(AllocTypes Type) {
    switch (Type)
    {
    case AllocTypeNone:
        return "None";
    case AllocTypeMalloc:
        return "Malloc";
    default:
        return "Unknown";
    }
}

static void HandleAllocation(AllocTypes Alloc, buffer* Buffer) {
    switch (Alloc)
    {
    case AllocTypeNone:
        break;
    case AllocTypeMalloc:
        *Buffer = AllocateBuffer(Buffer->Count);
        break;
    default:
        printf("Allocation: Unrecognized allocation type");
        break;
    }
}

static void HandleDeallocation(AllocTypes Alloc, buffer* Buffer) {
    switch (Alloc)
    {
    case AllocTypeNone:
        break;
    case AllocTypeMalloc:
        FreeBuffer(Buffer);
        break;
    default:
        printf("Deallocation: Unrecognized allocation type");
        break;
    }
}

//For pattern provisioning
enum BranchPatterns : u32 {
    BranchPatternNever,
    BranchPatternAwalys,
    BranchPatternEvery2,
    BranchPatternEvery3,
    BranchPatternEvery4,
    BranchPatternRandom,

    BranchPatternCount
};

#include <random>
void fillRandomPattern(buffer* data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);

    for (int i = 0; i < data->Count; i++) {
        data->Data[i] = dis(gen);
    }
}

void fillBranchPatterns(BranchPatterns pattern, buffer* data, const char** desc) {
    memset(data->Data, 0, data->Count);
    switch (pattern)
    {
    case BranchPatternAwalys: {
        memset(data->Data, 1, data->Count);
        *desc = "Awalys";
    } break;
    case BranchPatternNever: {
        memset(data->Data, 0, data->Count);
        *desc = "Never";
    } break;
    case BranchPatternEvery2: {
        *desc = "Every 2";
        for (int i = 0; i < data->Count; i++) {
            if (i % 2 == 0) {
                data->Data[i] = 1;
            }
        }
    } break;
    case BranchPatternEvery3: {
        *desc = "Every 3";
        for (int i = 0; i < data->Count; i++) {
            if (i % 3 == 0) {
                data->Data[i] = 1;
            }
        }
    } break;
    case BranchPatternEvery4: {
        *desc = "Every 4";
        for (int i = 0; i < data->Count; i++) {
            if (i % 4 == 0) {
                data->Data[i] = 1;
            }
        }
    } break;
    case BranchPatternRandom: {
        *desc = "RandomOS";
        fillRandomPattern(data);
    } break;
    default:
        break;
    }
}

enum MaskPatterns : u32 {
    MaskPatternL1,
    MaskPatternL2,
    MaskPatternL3,
    MaskPatternMemory,

    MaskPatternCount
};

void fillMaskPatterns(MaskPatterns pattern, buffer* data, const char** desc) {
    switch (pattern)
    {
    case MaskPatternL1: {
        //128KiB L1D cache
        *desc = "L1";
        ((u64*)data->Data)[0] = 131071;
    }
    break;

    case MaskPatternL2: {
        //32MB
        *desc = "L2";
        ((u64*)data->Data)[0] = 33554431;
    } break;

    case MaskPatternL3: {
        //48MB
        *desc = "L3";
        ((u64*)data->Data)[0] = 67108863;
    } break;

    case MaskPatternMemory: {
        *desc = "memory";
        ((u64*)data->Data)[0] = 0xFFFFFFFFFFFFFFFF;
    }break;
    
    default:
        break;
    }
}

extern "C" {
    void NOP1AllBytes(u64 count, u8* data);
    void NOP3AllBytes(u64 count, u8* data);
    void NOP9AllBytes(u64 count, u8* data);
    void ConditionalNOP(u64 count, u8* data);

    //code alignment test
    void NOPAlign64(u64 count, u8* data);
    void NOPAlign64_Pad4(u64 count, u8* data);
    void NOPAlign64_Pad20(u64 count, u8* data);
    void NOPAlign64_Pad40(u64 count, u8* data);
    void NOPAlign64_Pad60(u64 count, u8* data);

    //read ports
    void Read_x1(u64 count, u8* data);
    void Read_x2(u64 count, u8* data);
    void Read_x3(u64 count, u8* data);
    void Read_x4(u64 count, u8* data);

    //write ports
    void Write_x1(u64 count, u8* data);
    void Write_x2(u64 count, u8* data);
    void Write_x3(u64 count, u8* data);
    void Write_x4(u64 count, u8* data);

    //SIMD read
    void Read_4x1_ldur(u64 count, u8* data);
    void Read_4x2_ldur(u64 count, u8* data);
    void Read_4x3_ldur(u64 count, u8* data);
    void Read_4x4_ldur(u64 count, u8* data);
    void Read_4x1_ldr(u64 count, u8* data);
    void Read_4x2_ldr(u64 count, u8* data);
    void Read_4x3_ldr(u64 count, u8* data);
    void Read_4x4_ldr(u64 count, u8* data);
    void Read_8x3_ldr(u64 count, u8* data);
    void Read_8x3_ld1(u64 count, u8* data);
    void Read_16x3(u64 count, u8* data);
    void Read_16x4x1(u64 count, u8* data);
    void Read_16x4x2(u64 count, u8* data);
    void Read_16x4x3(u64 count, u8* data);

    void Read_mask(u64 count, u8* data);
    void Read_mask2(u64 count, u8* data);

    void DoubleLoop_Cache_Test(u64 outerCount, u64 innerCount, u8* data);
};

test_function TestFunctions[] = {
    // {"NOP1AllBytes", NOP1AllBytes},
    // {"NOP3AllBytes", NOP3AllBytes},
    // {"NOP9AllBytes", NOP9AllBytes},
    // {"ConditionalNOP", ConditionalNOP}

    // {"NOPAlign64", NOPAlign64},
    // {"NOPAlign64_Pad4",NOPAlign64_Pad4},
    // {"NOPAlign64_Pad20",NOPAlign64_Pad20},
    // {"NOPAlign64_Pad40",NOPAlign64_Pad40},
    // {"NOPAlign64_Pad60",NOPAlign64_Pad60}

    // {"Read_x1", Read_x1},
    // {"Read_x2", Read_x2},
    // {"Read_x3", Read_x3},
    // {"Read_x4", Read_x4},
    // {"Write_x1",Write_x1},
    // {"Write_x2",Write_x2},
    // {"Write_x3",Write_x3},
    // {"Write_x4",Write_x4},

    // {"Read_4x1_ldur", Read_4x1_ldur},
    // {"Read_4x1_ldr", Read_4x1_ldr},
    // {"Read_4x2_ldur", Read_4x2_ldur},
    // {"Read_4x2_ldr", Read_4x2_ldr},
    // {"Read_4x3_ldur", Read_4x3_ldur},
    // {"Read_4x3_ldr", Read_4x3_ldr},
    // {"Read_4x4_ldur", Read_4x4_ldur},
    // {"Read_4x4_ldr", Read_4x4_ldr},
    // {"Read_8x3_ld1", Read_8x3_ld1},
    // {"Read_8x3_ldr", Read_8x3_ldr},
    // {"Read_16x3", Read_16x3},
    // {"Read_16x4x1", Read_16x4x1},
    // {"Read_16x4x2", Read_16x4x2},
    // {"Read_16x4x3", Read_16x4x3}

    {"Read_mask", Read_mask}
};

static constexpr u64 MakeMultipleOfK(u64 n, u64 k) {
    return ((n+k)/k)*k;
}

//initial test written to test on specific cache size based on cpu specification
#define Version_1_Cache_Test 0
//reproduce what video had done, by gradually increasing the access range by power of 2
#define Version_2_Cache_Test 0
//double loop version that subdivides a memory region into bespoke pieces, achieving finer granularity compares to the power of 2 version
#define Version_3_Cache_Test 1

int main(int argc, char** argv) {

    u64 CPUTimerFreq = EstimateCPUFreq();
#if Version_1_Cache_Test
    if (argc == 2) {
        char *FileName = argv[1];
        struct stat Stat;
        stat(FileName, &Stat);

        buffer Dest = AllocateBuffer(MakeMultipleOfK(1024*1024*1024, 192));
        const char* hello;
        //To make sure that those pages are actually been mapped.
        fillBranchPatterns(BranchPatternEvery2, &Dest, &hello);

        printf("CPU Freq: %llu, FileSize: %zu\n", CPUTimerFreq, Dest.Count);

        if (Dest.Count > 0) {
            repetition_tester Testers[ArrayCount(TestFunctions)][MaskPatternCount] = {};

            for (;;) {
                for (u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex) {

                    // for (u32 AllocIndex = 0; AllocIndex < AllocTypeCount-1; ++AllocIndex) {
                    //     auto AllocType = (AllocTypes)AllocIndex;
                    for (u32 PatternIndex = 0; PatternIndex < MaskPatternCount; ++PatternIndex) {
                        
                        const char* patternDesc;
                        fillMaskPatterns((MaskPatterns)PatternIndex, &Dest, &patternDesc);
                        
                        repetition_tester *Tester = &Testers[FuncIndex][PatternIndex];
                        test_function TestFunc = TestFunctions[FuncIndex];
                        printf("\n--- %s %s---\n", TestFunc.Name, patternDesc);
                        NewTestWave(Tester, Dest.Count, CPUTimerFreq);
                        while (IsTesting(Tester)) {
                            buffer Buffer = Dest;//copy
                            // HandleAllocation(AllocType, &Buffer);
                            BeginTime(Tester);
                            TestFunc.Func(Buffer.Count, Buffer.Data);
                            EndTime(Tester);
                            CountBytes(Tester, Buffer.Count);
                            // HandleDeallocation(AllocType, &Buffer);
                        }
                    }
                }
            }
        }
        else {
            printf("Error: Test data size must be non-zero\n");
        }
    }
    else {
        printf("Usage: %s [exiting filename]\n", argv[0]);
    }

#elif Version_2_Cache_Test


    repetition_tester testers[30] = {};
    u64 allocAmount = MakeMultipleOfK(1024*1024*1024, 192);//read 192 bytes per iteration
    auto buffer = AllocateBuffer(allocAmount);
    //make sure those pages are actually been mapped
    for (int j = 0; j < buffer.Count; j++) {
        buffer.Data[j] = j;
    }

    for (int i = 10; i <= ArrayCount(testers); i++) {
        u64 span = (1llu << i);
        u64 mask = span - 1;
        ((u64*)buffer.Data)[0] = mask;

        repetition_tester* tester = testers + i;
        NewTestWave(tester, buffer.Count, CPUTimerFreq);
        while (IsTesting(tester)) {
            BeginTime(tester);
            Read_mask(buffer.Count, buffer.Data);
            EndTime(tester);
            CountBytes(tester, buffer.Count);
        }

        f64 seconds = SecondsFromCPUTime(tester->Results.MinTime, CPUTimerFreq);
        f64 gigabytes = 1024.0*1024.0*1024.0;
        f64 bandwidth = tester->TargetProcessedByteCount / (gigabytes*seconds);
        printf("%llu,%f\n", span, bandwidth);
    }
    FreeBuffer(&buffer);

#elif Version_3_Cache_Test
    //allocate at least 1GB of memory that is multiple of 192
    constexpr u64 allocAmount = MakeMultipleOfK(1024*1024*1024, 192);
    buffer mem = AllocateBuffer(allocAmount);
    //make sure that the memory allocated is actually mapped
    for (u32 i = 0; i < allocAmount; i++) {
        mem.Data[i] = i;
    }



    constexpr u64 l1 = MakeMultipleOfK(128*1024, 192);
    constexpr u64 l2 = MakeMultipleOfK(32*1024*1024, 192);
    constexpr u64 l3 = MakeMultipleOfK(48*1024*1024, 192);
    constexpr u64 lm = MakeMultipleOfK(100*1024*1024, 192);
    constexpr u64 footporints[] = {
        l1 - 192*32,
        l1 - 192*16,
        l1 - 192*8,
        l1 - 192*4,
        l1 - 192*2,
        l1 - 192,
        l1,
        l1 + 192*1024*2,
        l1 + 192*1024*2*2,
        l1 + 192*1024*2*2*2,
        l1 + 192*1024*2*2*2*2,
        l1 + 192*1024*2*2*2*2*2,


        (l1+l2)/2 + 192*1024*2,
        (l1+l2)/2 + 192*1024*2*2,
        (l1+l2)/2 + 192*1024*2*2*2,
        (l1+l2)/2 + 192*1024*2*2*2*2,
        (l1+l2)/2 + 192*1024*2*2*2*2*2,

        l2 + 192*1024*2,
        l2 + 192*1024*2*2,
        l2 + 192*1024*2*2*2,
        l2 + 192*1024*2*2*2*2,
        l2 + 192*1024*2*2*2*2*2,

        (l2+l3)/2 + 192*1024*2,
        (l2+l3)/2 + 192*1024*2*2,
        (l2+l3)/2 + 192*1024*2*2*2,
        (l2+l3)/2 + 192*1024*2*2*2*2,
        (l2+l3)/2 + 192*1024*2*2*2*2*2,
        l3,
        l3 + 192*1024*2,
        l3 + 192*1024*2*2,
        l3 + 192*1024*2*2*2,
        l3 + 192*1024*2*2*2*2,
        l3 + 192*1024*2*2*2*2*2,
        
        lm,
        lm + 192*1024*2,
        lm + 192*1024*2*2,
        lm + 192*1024*2*2*2,
        lm + 192*1024*2*2*2*2,
        lm + 192*1024*2*2*2*2*2,
    };


    repetition_tester testers[ArrayCount(footporints)] = {};
    for (u32 chunkIndex = 0; chunkIndex < ArrayCount(footporints); chunkIndex++) {
        repetition_tester *tester = testers + chunkIndex;

        u64 innerCount = footporints[chunkIndex] / 192;
        u64 outerCount = allocAmount / footporints[chunkIndex];

        NewTestWave(tester, allocAmount, CPUTimerFreq);
        while (IsTesting(tester)) {
            BeginTime(tester);
            DoubleLoop_Cache_Test(outerCount, innerCount, mem.Data);
            EndTime(tester);
            CountBytes(tester, allocAmount);
        }
        auto totalByte = tester->TargetProcessedByteCount;
        f64 minTimeSeconds = SecondsFromCPUTime(tester->Results.MinTime, CPUTimerFreq);
        f64 gigaByte = 1024*1024*1024;
        f64 bandwidth = totalByte / (gigaByte * minTimeSeconds);

        printf("%llu, %.2f\n", footporints[chunkIndex], bandwidth);
    }

#endif
    return 0;
}