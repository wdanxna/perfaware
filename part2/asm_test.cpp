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

extern "C" {
    void NOP1AllBytes(u64 count, u8* data);
    void NOP3AllBytes(u64 count, u8* data);
    void NOP9AllBytes(u64 count, u8* data);
};

test_function TestFunctions[] = {
    {"NOP1AllBytes", NOP1AllBytes},
    {"NOP3AllBytes", NOP3AllBytes},
    {"NOP9AllBytes", NOP9AllBytes}
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
            repetition_tester Testers[ArrayCount(TestFunctions)][AllocTypeCount] = {};

            for (;;) {
                for (u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex) {

                    for (u32 AllocIndex = 0; AllocIndex < AllocTypeCount-1; ++AllocIndex) {
                        auto AllocType = (AllocTypes)AllocIndex;

                        repetition_tester *Tester = &Testers[FuncIndex][AllocIndex];
                        test_function TestFunc = TestFunctions[FuncIndex];
                        printf("\n--- %s %s---\n", TestFunc.Name, AllocationDescription(AllocType));
                        NewTestWave(Tester, Dest.Count, CPUTimerFreq);
                        while (IsTesting(Tester)) {
                            buffer Buffer = Dest;//copy
                            HandleAllocation(AllocType, &Buffer);
                            BeginTime(Tester);
                            TestFunc.Func(Buffer.Count, Buffer.Data);
                            EndTime(Tester);
                            CountBytes(Tester, Buffer.Count);
                            HandleDeallocation(AllocType, &Buffer);
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