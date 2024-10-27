    .section    __TEXT,__text,regular,pure_instructions
    .global     _writeAllBytesASM        // Make the function globally accessible
    .global     _NopAllBytesASM
    .global     _CMPAllBytesASM
    .global     _DecAllBytesASM
    .align      2                     // Align the function to a 4-byte boundary

//this function replicate what C++ code does
_writeAllBytesASM:
    //x0 first parameter, the count
    //x1 second parameter, the data pointer
    eor x8, x8, x8 //set x8 to 0
    1:
    strb w8, [x1, x8] //actually write a byte into memory
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

//this function replace the strb with nop
_NopAllBytesASM:
    eor x8, x8, x8 //set x8 to 0
    1:
    nop //arm64 ISA is a fixed-length instruction set, all instructions take 4 bytes long
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

//this function replace strb or nop with nothing
_CMPAllBytesASM:
    eor x8, x8, x8 //set x8 to 0
    1:
    //purely increment and compare, no write nor nop
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

_DecAllBytesASM:
    //instead of having a dedicated incrementor
    //use x0 directly
    1:
    sub x0, x0, #0x1
    cbnz x0, 1b
    ret

