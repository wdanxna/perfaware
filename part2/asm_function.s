    .section    __TEXT,__text,regular,pure_instructions
    .global     _writeAllBytesASM        // Make the function globally accessible
    .global     _NopAllBytesASM
    .global     _CMPAllBytesASM
    .global     _DecAllBytesASM
    .global     _NOP1AllBytes
    .global     _NOP3AllBytes
    .global     _NOP9AllBytes
    .global     _ConditionalNOP
    .global     _NOPAlign64
    .global     _NOPAlign64_Pad4
    .global     _NOPAlign64_Pad20
    .global     _NOPAlign64_Pad40
    .global     _NOPAlign64_Pad60

    .global     _Read_x1
    .global     _Read_x2
    .global     _Read_x3
    .global     _Read_x4

    .global     _Write_x1
    .global     _Write_x2
    .global     _Write_x3
    .global     _Write_x4
    .align      2                     // Align the function to a 4-byte boundary

//probing the write ports
_Write_x1:
1:
    str x3, [x1]
    subs x0, x0, #0x1
    bgt 1b
    ret

_Write_x2:
1:
    str x3, [x1]
    str x3, [x1]
    subs x0, x0, #0x2
    bgt 1b
    ret

_Write_x3:
1:
    str x3, [x1]
    str x3, [x1]
    str x3, [x1]
    subs x0, x0, #0x3
    bgt 1b
    ret

_Write_x4:
1:
    str x3, [x1]
    str x3, [x1]
    str x3, [x1]
    str x3, [x1]
    subs x0, x0, #0x4
    bgt 1b
    ret

//probing the read ports
_Read_x1:
1:
    ldr x3, [x1]
    subs x0, x0, #0x1
    bgt 1b
    ret

_Read_x2:
1:
    ldr x3, [x1]
    ldr x3, [x1]
    subs x0, x0, #0x2
    bgt 1b
    ret

_Read_x3:
1:
    ldr x3, [x1]
    ldr x3, [x1]
    ldr x3, [x1]
    subs x0, x0, #0x3
    bgt 1b
    ret

_Read_x4:
1:
    ldr x3, [x1]
    ldr x3, [x1]
    ldr x3, [x1]
    ldr x3, [x1]
    subs x0, x0, #0x4
    bgt 1b
    ret

//These are for experiment for code alignment
_NOPAlign64:
    mov x8, #0x0
.align 6 //align at next 2^6 byte boundary
1:
    add x8, x8, #0x1
    cmp x8, x0
    b.ne 1b
    ret

_NOPAlign64_Pad4:
    mov x8, #0x0
.align 6 //align at next 2^6 byte boundary
nop //pad 4 byte, can only pad multiple of 4 in ARM64 I believe 
1:
    add x8, x8, #0x1
    cmp x8, x0
    b.ne 1b
    ret

_NOPAlign64_Pad20:
    mov x8, #0x0
.align 6 //align at next 2^6 byte boundary
nop
nop
nop
nop
nop
1:
    add x8, x8, #0x1
    cmp x8, x0
    b.ne 1b
    ret

_NOPAlign64_Pad40:
    mov x8, #0x0
.align 6 //align at next 2^6 byte boundary
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
1:
    add x8, x8, #0x1
    cmp x8, x0
    b.ne 1b
    ret

//try to span the 64B L1 cache line
_NOPAlign64_Pad60:
    mov x8, #0x0
.align 6 //align at next 2^6 byte boundary
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
nop
1:
    add x8, x8, #0x1
    cmp x8, x0
    b.ne 1b
    ret

//experiment that probing the branch selection behaviour
_ConditionalNOP:
    mov x8, #0x0
1:
    ldrb w2, [x1, x8]
    add x8, x8, #0x1 //increment
    cmp x2, #0x1
    b.eq .skip // take the jump if data[i] == 1
    //FallthroughBranch
    nop
.skip:
    //TakenBranch
    cmp x8, x0
    b.ne 1b
    ret
    

//Those following functions are dedicated to experiment the ramification of Nops for throughput
//This has a sigle nop
_NOP1AllBytes:
    eor x8, x8, x8 //set x8 to 0
    1:
    nop
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

//This has 3 nops
_NOP3AllBytes:
    eor x8, x8, x8 //set x8 to 0
    1:
    nop
    nop
    nop
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

//9 nops
_NOP9AllBytes:
    eor x8, x8, x8 //set x8 to 0
    1:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    add x8, x8, #0x1
    cmp x0, x8
    b.ne 1b
    ret

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

