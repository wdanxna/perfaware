using file_process_func = void (repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch);
#define MIN_MEMORY_PAGE_SIZE 16384 /*16K*/

//control group, simply allocate and write a constant into the destination buffer
void AllocateAndTouch(
    repetition_tester *Tester, 
    char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch) {

    (void)FileName;
    (void)TotalFileSize;
    (void)Scratch;

    buffer Buffer = AllocateBuffer(BufferSize);
    if (IsValid(Buffer)) {
        //how many page the buffer takes
        u64 TouchCount = (Buffer.Count + MIN_MEMORY_PAGE_SIZE - 1) / MIN_MEMORY_PAGE_SIZE;

        for (u32 PageIndex = 0; PageIndex < TouchCount; ++PageIndex) {
            Buffer.Data[PageIndex*MIN_MEMORY_PAGE_SIZE] = 0; //write
        }
        CountBytes(Tester, TotalFileSize);//pretend we write the whole file?
    }
    else {
        Error(Tester, "Couldn't aquire resources");
    }
    FreeBuffer(&Buffer);
}

//assuming the file's content is already present in the Scratch buffer
void AllocateAndCopy(
    repetition_tester *Tester, 
    char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch) {
    
    (void)FileName;
    buffer Buffer = AllocateBuffer(BufferSize);
    if (IsValid(Buffer)) {
        u8 *Source = Scratch.Data;
        u64 SizeRemaining = TotalFileSize;
        while (SizeRemaining) {
            u64 CopySize = Buffer.Count;
            if (SizeRemaining < CopySize) {
                CopySize = SizeRemaining;
            }
            memcpy(Buffer.Data, Source, CopySize);
            SizeRemaining -= CopySize;
            Source += CopySize;
            CountBytes(Tester, CopySize);
        }
    }
    else {
        Error(Tester, "Couldn't aquire resources");
    }
    FreeBuffer(&Buffer);
}

//This is the complete file read version
void OopenAllocateAndFRead(
    repetition_tester *Tester, 
    char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch) {
    
    (void)Scratch;

    FILE *fd = fopen(FileName, "rb");
    if (fd == NULL) {
        Error(Tester, "Couldn't open file");
    }
    else {
        buffer Buffer = AllocateBuffer(BufferSize);
        if (IsValid(Buffer)) {
            u64 RemainingSize = TotalFileSize;
            while (RemainingSize) {
                u64 ReadSize = Buffer.Count;
                if (ReadSize > RemainingSize) {
                    ReadSize = RemainingSize;
                }
                if (fread(Buffer.Data, ReadSize, 1, fd) == 1) {
                    CountBytes(Tester, ReadSize);
                }
                else {
                    Error(Tester, "fread Failed");
                }

                RemainingSize -= ReadSize;
            }
        }
        else {
            Error(Tester, "Couldn't aquire resource");
        }

        fclose(fd);
        FreeBuffer(&Buffer);
    }
}