/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 68
   ======================================================================== */

struct buffer
{
    size_t Count;
    u8 *Data;
};

#define CONSTANT_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

static b32 IsInBounds(buffer Source, u64 At)
{
    b32 Result = (At < Source.Count);
    return Result;
}

static b32 AreEqual(buffer A, buffer B)
{
    if(A.Count != B.Count)
    {
        return false;
    }
    
    for(u64 Index = 0; Index < A.Count; ++Index)
    {
        if(A.Data[Index] != B.Data[Index])
        {
            return false;
        }
    }
    
    return true;
}

static buffer AllocateBuffer(size_t Count)
{
    buffer Result = {};
    Result.Data = (u8 *)malloc(Count);
    if(Result.Data)
    {
        Result.Count = Count;
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to allocate %zu bytes.\n", Count);
    }
    
    return Result;
}

static void FreeBuffer(buffer *Buffer)
{
    if(Buffer->Data)
    {
        free(Buffer->Data);
    }
    *Buffer = {};
}

static buffer AlignedAlloc(u64 size, u64 alignment) {
    assert((alignment & 1) == 0);//alignment must be power of 2
    u64 allocSize = size + sizeof(void*) + alignment - 1 ;
    u64* mem = (u64*)malloc(allocSize);
    u64 ptr = ((u64)mem + sizeof(void*) + alignment - 1) & ~(alignment - 1);
    *((u64*)ptr - 1) = (u64)mem;

    buffer ret = {
        .Count = ((allocSize - (ptr - (u64)mem)) / alignment)*alignment,
        .Data = (u8*)ptr
    };

    return ret;
}

static void AlignedFree(buffer* buffer) {
    u64* mem = (u64*)*((u64*)buffer->Data - 1);
    buffer->Data = (u8*)mem;
    FreeBuffer(buffer);
}