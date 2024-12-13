#include <sys/stat.h>

static void *OSAllocate(size_t ByteCount)
{
    void *Result = mmap(NULL, ByteCount, PROT_READ|PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return Result;
}

static void OSFree(size_t ByteCount, void *BaseAddress)
{
    munmap(BaseAddress, ByteCount);
}

static u64 GetFileSize(char *FileName)
{
    struct stat Stat;
    stat(FileName, &Stat);
    
    return Stat.st_size;
}

static buffer ReadEntireFile(char *FileName)
{
    buffer Result = {};
    
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        Result = AllocateBuffer(GetFileSize(FileName));
        if(Result.Data)
        {
            if(fread(Result.Data, Result.Count, 1, File) != 1)
            {
                int err = ferror(File);
                fprintf(stderr, "ERROR: Unable to read \"%s\" %d.\n", FileName, err);
                FreeBuffer(&Result);
            }
        }
        
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", FileName);
    }
    
    return Result;
}