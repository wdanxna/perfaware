#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <CoreFoundation/CoreFoundation.h>
#include <sys/types.h>
#include <sys/mman.h>

#include "metrics.cpp"
#include "repetition_tester.cpp"
#include "buffer.cpp"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Error: input a page count");
        return 1;
    }

    auto CPUFreq = EstimateCPUFreq();
    u64 PageSize = 16*1024;
    u64 PageCount = atoi(argv[1]);
    u64 TotalSize = PageSize * PageCount;

    //generate CSV
    printf("Page Count, Touch Count, Fault Count, Extra Faults\n");
    // for (u64 TouchCount = 0; TouchCount <= PageCount; TouchCount++) {
    //     //how many pages to map
    //     u64 TouchSize = PageSize * TouchCount;
    //     u8 *Data = (u8*)mmap(NULL, TotalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    //     madvise(Data, TotalSize, MADV_SEQUENTIAL);
    //     if (Data) {
    //         u64 StartFaultCount = ReadOSPageFaultCount();
    //         for (u64 Index = 0; Index < TouchSize; Index++) {
    //             //write to every byte of the mapped pages
    //             Data[Index] = (u8)Index;
    //         }
    //         u64 EndFaultCount = ReadOSPageFaultCount();
    //         u64 FaultCount = EndFaultCount - StartFaultCount;
    //         printf("%llu, %llu, %llu, %lld\n", PageCount, TouchCount, FaultCount, (FaultCount - TouchCount));
    //         munmap(Data, TotalSize);
    //     }
    //     else {
    //         fprintf(stderr, "Error: unable to allocate memory\n");
    //     }
    // }

    for (u64 TouchCount = 0; TouchCount <= PageCount; TouchCount++) {
        //how many pages to map
        u64 TouchSize = PageSize * TouchCount;
        u8 *Data = (u8*)mmap(NULL, TotalSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        madvise(Data, TotalSize, MADV_SEQUENTIAL);
        if (Data) {
            u64 StartFaultCount = ReadOSPageFaultCount();
            for (u64 Index = 0; Index < TouchSize; Index++) {
                //write to every byte of the mapped pages
                Data[TotalSize - 1 - Index] = (u8)Index;
            }
            u64 EndFaultCount = ReadOSPageFaultCount();
            u64 FaultCount = EndFaultCount - StartFaultCount;
            printf("%llu, %llu, %llu, %lld\n", PageCount, TouchCount, FaultCount, (FaultCount - TouchCount));
            munmap(Data, TotalSize);
        }
        else {
            fprintf(stderr, "Error: unable to allocate memory\n");
        }
    }
    
    return 0;
}