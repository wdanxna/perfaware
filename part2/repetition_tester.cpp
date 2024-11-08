#include "common.h"

enum test_mode : u32 {
    TestMode_Uninitialized,
    TestMode_Testing,
    TestMode_Completed,
    TestMode_Error
};

struct repetition_test_results {
    u64 TestCount; //how many times the test has run.
    u64 TotalTime; //how long has the test last.
    u64 MaxTime; //the slowest time the test has recorded.
    u64 MinTime; //the fastest time the test has recorded.

    u64 MaxPageFault;
    u64 MinPageFault;
};
//the finite-state-machine
struct repetition_tester {
    u64 TargetProcessedByteCount;//how many bytes this test has been processed
    u64 CPUTimerFreq;
    u64 TryForTime; //total number of CPU timer ticks to spend for trying to find a new fatest time.
    u64 TestsStartedAt; //current cpu timer

    test_mode Mode;
    b32 PrintNewMinimums; //whether print it when a new minimum is found
    u32 OpenBlockCount; //the amount of block opener
    u32 CloseBlockCount; //the amount of block closer
    u64 TimeAccumulatedOnThisTest; //how much time has this test elapsed.
    u64 BytesAccumulatedOnThisTest; //how many bytes in total has this test processed.
    u64 PageFaultsAccumulatedOnThisTest;

    repetition_test_results Results;
};

static void Error(repetition_tester *Tester, const char* Reason) {
    Tester->Mode = TestMode_Error;
    printf("Error: %s\n", Reason);
}

static void NewTestWave(
    repetition_tester *Tester, 
    u64 TargetProcessedByteCount, 
    u64 CPUTimerFreq, 
    u32 SecondsToTry = 10) {

        //initialize the tester if it's never initialized before
        if (Tester->Mode == TestMode_Uninitialized) {
            Tester->Mode = TestMode_Testing;
            Tester->TargetProcessedByteCount = TargetProcessedByteCount;
            Tester->CPUTimerFreq = CPUTimerFreq;
            Tester->PrintNewMinimums = false;
            Tester->Results.MinTime = (u64)-1;
            Tester->Results.MaxTime = 0;
            Tester->Results.MinPageFault = (u64)-1;
        }

        //if the tester has been used before, reset its mode to testing
        //and check values being passed in don't differ.
        else if(Tester->Mode == TestMode_Completed) {
            Tester->Mode = TestMode_Testing;
            if (Tester->TargetProcessedByteCount != TargetProcessedByteCount) {
                Error(Tester, "TargetProcessedByteCount changed");
            }
            if (Tester->CPUTimerFreq != CPUTimerFreq) {
                Error(Tester, "CPU frequency changed");
            }
        }

        Tester->TryForTime = SecondsToTry * CPUTimerFreq;
        Tester->TestsStartedAt = ReadCPUTimer();
}

static void BeginTime(repetition_tester *Tester) {
    ++Tester->OpenBlockCount;
    //acc += (end - begin)
    //  -> acc += -begin + end 
    //  -> acc -= begin; acc += end
    Tester->TimeAccumulatedOnThisTest -= ReadCPUTimer();
    Tester->PageFaultsAccumulatedOnThisTest -= ReadOSPageFaultCount();
}

static void EndTime(repetition_tester *Tester) {
    ++Tester->CloseBlockCount;
    Tester->TimeAccumulatedOnThisTest += ReadCPUTimer();
    Tester->PageFaultsAccumulatedOnThisTest += ReadOSPageFaultCount();
}

static void CountBytes(repetition_tester *Tester, u64 ByteCount) {
    Tester->BytesAccumulatedOnThisTest += ByteCount;
}

static f64 SecondsFromCPUTime(f64 CPUTime, u64 CPUTimerFreq) {
    return CPUTimerFreq == 0 ? 0.0 : (f64)CPUTime / CPUTimerFreq;
}

static void PrintTime(const char *Label, f64 CPUTime, u64 CPUTimerFreq, u64 ByteCount) {
    printf("%s: %.0f", Label, CPUTime);
    if (CPUTimerFreq) {
        f64 Seconds = SecondsFromCPUTime(CPUTime, CPUTimerFreq);
        printf(" (%fms)", 1000.0f * Seconds);

        if (ByteCount) {
            f64 Gigabyte = 1024.0 * 1024.0 * 1024.0;
            f64 BestBandwidth = ByteCount / (Gigabyte * Seconds);
            printf(" %fGB/s", BestBandwidth);
        }
    }
}

static void PrintPageFault(const char *Label, u64 Faults, f64 Seconds) {
    printf(" %s: %llu (%.2f faults/second)", Label, Faults, (f64)Faults/Seconds);
}

static void PrintResults(repetition_test_results Results, u64 CPUTimerFreq, u64 ByteCount) {
    PrintTime("Min", (f64)Results.MinTime, CPUTimerFreq, ByteCount);
    PrintPageFault("PF", Results.MinPageFault, SecondsFromCPUTime(Results.TotalTime, CPUTimerFreq));
    printf("\n");

    PrintTime("Max", (f64)Results.MaxTime, CPUTimerFreq, ByteCount);
    PrintPageFault("PF", Results.MaxPageFault, SecondsFromCPUTime(Results.TotalTime, CPUTimerFreq));
    printf("\n");

    if (Results.TestCount) {
        PrintTime("Avg", (f64)Results.TotalTime / Results.TestCount, CPUTimerFreq, ByteCount);
        PrintPageFault("PF",(f64)(Results.MinPageFault + Results.MaxPageFault)/2.0, SecondsFromCPUTime(Results.TotalTime, CPUTimerFreq));
        printf("\n");
    }
}

static b32 IsTesting(repetition_tester *Tester) {
    if (Tester->Mode == TestMode_Testing) {
        u64 CurrentTime = ReadCPUTimer();

        if (Tester->OpenBlockCount) {
            //if the test has any timing blocks
            if (Tester->OpenBlockCount != Tester->CloseBlockCount) {
                Error(Tester, "Unbalanced BeginTime/EndTime");
            }

            if (Tester->BytesAccumulatedOnThisTest != Tester->TargetProcessedByteCount) {
                Error(Tester, "Processed byte count mismatch");
            }

            if (Tester->Mode == TestMode_Testing) {
                repetition_test_results *Results = &Tester->Results;
                u64 ElapsedTime = Tester->TimeAccumulatedOnThisTest;
                u64 PageFaultsDiff = Tester->PageFaultsAccumulatedOnThisTest;
                Results->TestCount += 1;
                Results->TotalTime += ElapsedTime;
                //udpate the slowest time
                if (Results->MaxTime < ElapsedTime) {
                    Results->MaxTime = ElapsedTime;
                    Results->MaxPageFault = PageFaultsDiff;
                    // PrintTime("Max", Results->MaxTime, Tester->CPUTimerFreq, Tester->BytesAccumulatedOnThisTest);
                    // printf("            \n");
                }
                //update the fastest time
                if (Results->MinTime > ElapsedTime) {
                    Results->MinTime = ElapsedTime;
                    Results->MinPageFault = PageFaultsDiff;
                    //whenever we get a new minimum, we reset the clock to the full trail time
                    Tester->TestsStartedAt = CurrentTime;
                    if (Tester->PrintNewMinimums) {
                        PrintTime("Min", Results->MinTime, Tester->CPUTimerFreq, Tester->BytesAccumulatedOnThisTest);
                        printf("            \n");
                    }
                }
                // u64 PageFaultsDiff = Tester->PageFaultsAccumulatedOnThisTest;
                // if (Results->MaxPageFault < PageFaultsDiff) {
                //     Results->MaxPageFault = PageFaultsDiff;
                // }
                // if (Results->MinPageFault > PageFaultsDiff) {
                //     Results->MinPageFault = PageFaultsDiff;
                // }

                Tester->OpenBlockCount = 0;
                Tester->CloseBlockCount = 0;
                Tester->TimeAccumulatedOnThisTest = 0;
                Tester->BytesAccumulatedOnThisTest = 0;
                Tester->PageFaultsAccumulatedOnThisTest = 0;
            }
        }
        //check if enough time has elapsed since the last new MinTime
        if ((CurrentTime - Tester->TestsStartedAt) > Tester->TryForTime) {
            Tester->Mode = TestMode_Completed;
            // printf("\n");
            // PrintResults(Tester->Results, Tester->CPUTimerFreq, Tester->TargetProcessedByteCount);
        }
    }

    b32 Result = (Tester->Mode == TestMode_Testing);
    return Result;
}