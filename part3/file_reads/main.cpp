#include "common/common.cpp"
#include "file_reads.cpp"

//The purpose of this test, is to find out what's the sweetspot of the read buffer
//that can achieve the maximum read throughput from file reading syscall.

//The whole idea is, if we call read syscall to the OS for asking a buffer of file content
//If the content of the file is already loaded (by the OS) into the file cache, which is in
//kernel space memory, since these memory have to be copied from kernel space into the process
//virtual address space, the OS needs to first, reading the buffer from its own memory, then writing
//them into the process address space. There is nothing we can do for the reading, but, there maybe something
//we can do with the writing, which if we choose a relatively small writing buffer size, by the time
//our process want to read from the buffer, the buffer is in the L2 cache, so we can achieve better
//throughput of consuming the file content, furthermore, since we repeatedly writing to the same small buffer
//instead of for example a large 1GB of buffer, we pay page fault once and never again regardless of the
//file size. But there is a tradeoff, if we choose a too small of the buffer size, then we need to issue
//a lot of syscall in order to complete the file read, which is also a very large overhead.
//This experiment is meant to hunt for a sweetspot of the writing buffer size that can achieve the best
//file reading throughput.

struct test_function {
    char const *Name;
    file_process_func *Func;
};

static test_function TestFunctions[] = {
    {"AllocateAndTouch", AllocateAndTouch},
    {"AllocateAndCopy", AllocateAndCopy},
    {"OpenAllocateAndFRead", OopenAllocateAndFRead}
    // {"OpenAllocateAndRead", OpenAllocateAndRead},
};


int main(int argc, char** argv) {
    u64 CPUTimerFreq = EstimateCPUFreq();

    if (argc == 2) {
        char *FileName = argv[1];
        printf("%s\n", FileName);
        buffer Buffer = ReadEntireFile(FileName);

        //consider the test series is a excel table, where each column is a test for certain function
        //each row is the certain input size.
        repetition_test_series TestSeries = AllocateTestSeries(ArrayCount(TestFunctions), 1024);

        if (IsValid(Buffer) && IsValid(TestSeries)) {
            
            u64 FileSize = Buffer.Count;
            //build the test series as we go.
            SetRowLabelLabel(&TestSeries, "ReadBufferSize");
            //for each read buffer size
            for (u64 ReadBufferSize = 256*1024; ReadBufferSize <= FileSize; ReadBufferSize*=2) {
                SetRowLabel(&TestSeries, "%lluk", ReadBufferSize/1024);
                //and each function
                for (u64 TestFunctionIndex = 0; TestFunctionIndex < ArrayCount(TestFunctions); ++TestFunctionIndex) {
                    test_function Func = TestFunctions[TestFunctionIndex];
                    SetColumnLabel(&TestSeries, "%s", Func.Name);

                    repetition_tester Tester = {};
                    NewTestWave(&TestSeries, &Tester, FileSize, CPUTimerFreq);
                    while (IsTesting(&TestSeries, &Tester)) {
                        BeginTime(&Tester);
                        Func.Func(&Tester, FileName, FileSize, ReadBufferSize, Buffer);
                        EndTime(&Tester);
                    }
                }
            } 

            PrintCSVForValue(&TestSeries, StatValue_GBPerSecond, stdout);
        }
        else {
            fprintf(stderr, "Error: Test data size must be non-zero\n");
        }
        FreeBuffer(&Buffer);
        FreeTestSeries(&TestSeries);
    }
    else {
        fprintf(stderr, "Usage: %s [existing filename]\n", argv[0]);
    }
    return 0;
}