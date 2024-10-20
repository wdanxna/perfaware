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

struct read_parameters {
    buffer Dest;
    const char *Filename;
};

using read_overhead_test_func = void (repetition_tester*, read_parameters*);

struct test_function {
    const char *Name;
    read_overhead_test_func *Func;
};

static void ReadViaFRead(repetition_tester *Tester, read_parameters *Params) {
    while (IsTesting(Tester)) {
        FILE *File = fopen(Params->Filename, "rb");
        if (File) {
            buffer DestBuffer = Params->Dest;

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

                CFReadStreamClose(ReadStream);
                CFRelease(ReadStream);
                CFRelease(FileURL);

                SizeRemaining -= ReadSize;
            }
        }
        else {
            Error(Tester, "CFReadStreamOpen failed");
        }
    }
}

test_function TestFunctions[] = {
    {"ReadFile", ReadViaCFReadStream},
    {"fread", ReadViaFRead},
    {"read", ReadViaRead}
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

        if (Params.Dest.Count > 0) {
            repetition_tester Testers[ArrayCount(TestFunctions)] = {};

            for (;;) {
                for (u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex) {
                    repetition_tester *Tester = Testers + FuncIndex;
                    test_function TestFunc = TestFunctions[FuncIndex];
                    printf("\n--- %s ---\n", TestFunc.Name);
                    NewTestWave(Tester, Params.Dest.Count, CPUTimerFreq);
                    TestFunc.Func(Tester, &Params);
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