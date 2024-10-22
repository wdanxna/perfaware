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

//add this to test between different allocation type
enum AllocTypes :u32 {
    AllocTypeNone = 0,
    AllocTypeMalloc,//malloc & free every file read
    AllocTypeCount
};

struct read_parameters {
    buffer Dest;
    const char *Filename;
    AllocTypes Alloc;
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

static void HandleAllocation(read_parameters* Param, buffer* Buffer) {
    switch (Param->Alloc)
    {
    case AllocTypeNone:
        break;
    case AllocTypeMalloc:
        *Buffer = AllocateBuffer(Param->Dest.Count);
        break;
    default:
        printf("Allocation: Unrecognized allocation type");
        break;
    }
}

static void HandleDeallocation(read_parameters* Params, buffer* Buffer) {
    switch (Params->Alloc)
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

using read_overhead_test_func = void (repetition_tester*, read_parameters*);

struct test_function {
    const char *Name;
    read_overhead_test_func *Func;
};

static void ReadViaFRead(repetition_tester *Tester, read_parameters *Params) {
    while (IsTesting(Tester)) {
        FILE *File = fopen(Params->Filename, "rb");
        if (File) {
            buffer DestBuffer = Params->Dest;//copied
            HandleAllocation(Params, &DestBuffer);

            BeginTime(Tester);
            size_t Result = fread(DestBuffer.Data, DestBuffer.Count, 1, File);
            EndTime(Tester);
            if (Result == 1) {
                CountBytes(Tester, DestBuffer.Count);
            }
            else {
                Error(Tester, "fread failed");
            }

            fclose(File);
            HandleDeallocation(Params, &DestBuffer);
        }
        else {
            Error(Tester, "fopen failed");
        }
    }
}

static void ReadViaRead(repetition_tester *Tester, read_parameters *Params) {
    while (IsTesting(Tester)) { 
        int File = open(Params->Filename, O_RDONLY);
        if (File != -1) {
            buffer DestBuffer = Params->Dest;
            HandleAllocation(Params, &DestBuffer);

            u8 *Dest = DestBuffer.Data;
            u64 SizeRemaining = DestBuffer.Count;
            while (SizeRemaining) {
                u32 ReadSize = INT_MAX;
                if ((u64)ReadSize > SizeRemaining) {
                    ReadSize = (u32)SizeRemaining;
                }
                BeginTime(Tester);
                int Result = read(File, Dest, ReadSize);
                EndTime(Tester);

                if (Result == (int)ReadSize) {
                    CountBytes(Tester, ReadSize);
                }
                else {
                    Error(Tester, "_read failed");
                    break;
                }

                SizeRemaining -= ReadSize;
                Dest += ReadSize;
            }

            close(File);
            HandleDeallocation(Params, &DestBuffer);
        }
        else {
            Error(Tester, "_open failed");
        }
    }
}

static void ReadViaCFReadStream(repetition_tester *Tester, read_parameters *Params) {
    while (IsTesting(Tester)) {
        CFStringRef FilePath = CFStringCreateWithCString(kCFAllocatorDefault, Params->Filename, kCFStringEncodingASCII);
        CFURLRef FileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, FilePath, kCFURLPOSIXPathStyle, false);

        CFReadStreamRef ReadStream = CFReadStreamCreateWithFile(kCFAllocatorDefault, FileURL);
        if (CFReadStreamOpen(ReadStream)) {
            buffer DestBuffer = Params->Dest;
            HandleAllocation(Params, &DestBuffer);

            u64 SizeRemaining = DestBuffer.Count;
            while (SizeRemaining) {
                int ReadSize = INT_MAX;
                if ((u64)ReadSize > SizeRemaining) {
                    ReadSize = (u32)SizeRemaining;
                }
                BeginTime(Tester);
                int Result = CFReadStreamRead(ReadStream, DestBuffer.Data, ReadSize);
                EndTime(Tester);
                if (Result == ReadSize) {
                    CountBytes(Tester, ReadSize);
                }
                else {
                    Error(Tester, "CFReadStreamRead failed");
                    break;
                }
                SizeRemaining -= ReadSize;
            }

            CFReadStreamClose(ReadStream);
            HandleDeallocation(Params, &DestBuffer);
        }
        else {
            Error(Tester, "CFReadStreamOpen failed");
        }
        CFRelease(ReadStream);
        CFRelease(FileURL);
        CFRelease(FilePath);
    }
}

static void WriteToAllBytes(repetition_tester *Tester, read_parameters *Params) {
    while (IsTesting(Tester)) {
        buffer Dest = Params->Dest;
        HandleAllocation(Params, &Dest);
        BeginTime(Tester);
        for (int i = 0; i < Dest.Count; i++) {
            Dest.Data[i] = (u8)i;
        }
        EndTime(Tester);

        CountBytes(Tester, Dest.Count);
        HandleDeallocation(Params, &Dest);
    }
}

test_function TestFunctions[] = {
    // {"ReadFile", ReadViaCFReadStream},
    // {"fread", ReadViaFRead},
    // {"read", ReadViaRead}
    {"WriteToAllBytes", WriteToAllBytes}
};

int main(int argc, char** argv) {

    u64 CPUTimerFreq = EstimateCPUFreq();

    if (argc == 2) {
        char *FileName = argv[1];
        struct stat Stat;
        stat(FileName, &Stat);

        read_parameters Params{};
        Params.Dest = AllocateBuffer(Stat.st_size);
        Params.Filename = FileName;

        printf("CPU Freq: %llu, FileSize: %llu\n", CPUTimerFreq, Stat.st_size);

        if (Params.Dest.Count > 0) {
            repetition_tester Testers[ArrayCount(TestFunctions)][AllocTypeCount] = {};

            for (;;) {
                for (u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex) {

                    for (u32 AllocIndex = 0; AllocIndex < AllocTypeCount; ++AllocIndex) {
                        Params.Alloc = (AllocTypes)AllocIndex;

                        repetition_tester *Tester = &Testers[FuncIndex][AllocIndex];
                        test_function TestFunc = TestFunctions[FuncIndex];
                        printf("\n--- %s %s---\n", TestFunc.Name, AllocationDescription(Params.Alloc));
                        NewTestWave(Tester, Params.Dest.Count, CPUTimerFreq);
                        TestFunc.Func(Tester, &Params);
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