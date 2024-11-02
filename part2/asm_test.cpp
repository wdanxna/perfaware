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

    {"Read_4x1_ldur", Read_4x1_ldur},
    {"Read_4x1_ldr", Read_4x1_ldr},
    {"Read_4x2_ldur", Read_4x2_ldur},
    {"Read_4x2_ldr", Read_4x2_ldr},
    {"Read_4x3_ldur", Read_4x3_ldur},
    {"Read_4x3_ldr", Read_4x3_ldr},
    {"Read_4x4_ldur", Read_4x4_ldur},
    {"Read_4x4_ldr", Read_4x4_ldr},
    {"Read_8x3_ld1", Read_8x3_ld1},
    {"Read_8x3_ldr", Read_8x3_ldr},
    {"Read_16x3", Read_16x3},
    {"Read_16x4x1", Read_16x4x1},
    {"Read_16x4x2", Read_16x4x2},
    {"Read_16x4x3", Read_16x4x3}
};


int main(int argc, char** argv) {

    u64 CPUTimerFreq = EstimateCPUFreq();

    if (argc == 2) {
        char *FileName = argv[1];
        struct stat Stat;
        stat(FileName, &Stat);

        buffer Dest = AllocateBuffer(Stat.st_size);

        printf("CPU Freq: %llu, FileSize: %llu\n", CPUTimerFreq, Stat.st_size);

        if (Dest.Count > 0) {
            repetition_tester Testers[ArrayCount(TestFunctions)] = {};

            for (;;) {
                for (u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex) {

                    // for (u32 AllocIndex = 0; AllocIndex < AllocTypeCount-1; ++AllocIndex) {
                    //     auto AllocType = (AllocTypes)AllocIndex;
                    for (u32 PatternIndex = 0; PatternIndex < 1; ++PatternIndex) {
                        
                        // const char* patternDesc;
                        // fillBranchPatterns((BranchPatterns)PatternIndex, &Dest, &patternDesc);
                        
                        repetition_tester *Tester = &Testers[FuncIndex];
                        test_function TestFunc = TestFunctions[FuncIndex];
                        printf("\n--- %s ---\n", TestFunc.Name);
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

    return 0;
}