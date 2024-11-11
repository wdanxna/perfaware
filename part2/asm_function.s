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

    .global     _Read_4x1_ldur
    .global     _Read_4x2_ldur
    .global     _Read_4x3_ldur
    .global     _Read_4x4_ldur
    .global     _Read_4x1_ldr
    .global     _Read_4x2_ldr
    .global     _Read_4x3_ldr
    .global     _Read_4x4_ldr
    .global     _Read_8x3_ldr
    .global     _Read_8x3_ld1
    .global     _Read_16x3
    .global     _Read_16x4x1
    .global     _Read_16x4x2
    .global     _Read_16x4x3


    .global     _Read_mask
    .global     _Read_mask2
    .global     _DoubleLoop_Cache_Test

    .global     _Align_Read
    .global     _UnAlign_Read

    .global     _Jump_Read
    .align      2                     // Align the function to a 4-byte boundary

//x0, count
//x1, distance
//x2, base pointer
_Jump_Read:
1:
    ldr x3, [x2]
    ldr x3, [x2, x1]
    subs x0, x0, #16
    bgt 1b
    ret


//x0, count
//x1, base pointer
_Align_Read:
1:
    ldr q0, [x1, #0]
    ldr q0, [x1, #16]
    ldr q0, [x1, #32]
    ldr q0, [x1, #48]

    add x1, x1, #64
    subs x0, x0, #64
    bgt 1b
    ret

_UnAlign_Read:
add x1, x1, #60
1:
    ldr q0, [x1, #0]
    ldr q0, [x1, #16]
    ldr q0, [x1, #32]
    ldr q0, [x1, #48]

    add x1, x1, #64
    subs x0, x0, #64
    bgt 1b
    ret


//x0: outer loop iteration count, this determines how much memory to access in total
//x1: inner loop iteration count, this controls the access footprint
//x2: base pointer
_DoubleLoop_Cache_Test:
1:
    mov x3, x1 //reset inner loop count
    mov x4, x2 //reset base pointer
    2: //inner loop, read 192 byte per iteration
        ldr q0, [x4, #0]
        ldr q0, [x4, #16]
        ldr q0, [x4, #32]

        ldr q0, [x4, #48]
        ldr q0, [x4, #64]
        ldr q0, [x4, #80]

        add x4, x4, #96
        subs x3, x3, #1
        b.ne 2b
    subs x0, x0, #1
    b.ne 1b
    ret


//Cache test
//read mask from x1
//assuming L1D cache is 128kiB = 2^17, mask = 2^17-1
//L2 cache is 32MB = 2^25, mask = 2^25-1
//L3 cache is 48MB = 2^25 + 2^24, mask = 2^26-1
_Read_mask:
mov x3, #0 //offset
mov x4, x1 //base pointer
mov x8, #0 //total
ldr x7, [x1] //mask

1:
    //read x 3, since there are 3 read ports
    //3 load, 1 cycle. To hide 3 cycles offset update, write 4 sets of ldr to make sure it at least takes 4 cycles to run
    ldr q0, [x4, #0]
    ldr q0, [x4, #16]
    ldr q0, [x4, #32]

    ldr q0, [x4, #48]
    ldr q0, [x4, #64]
    ldr q0, [x4, #80]

    ldr q0, [x4, #96]
    ldr q0, [x4, #112]
    ldr q0, [x4, #128]

    ldr q0, [x4, #144]
    ldr q0, [x4, #160]
    ldr q0, [x4, #176]

    //update offset
    //dependency chain, at least 3 cycles
    add x3, x3, #192 //cycle 1
    and x3, x3, x7  //cycle 2

    mov x4, x1
    add x4, x4, x3 //cycle 3



    add x8, x8, #192
    cmp x0, x8
    b.ne 1b
    ret



_Read_mask2:
mov x2, x1 //save base pointer
mov x3, #0 //offset
mov x8, #0 //total
ldr x7, [x1] //mask
1:
    mov x1, x2 //ld1 do post-update, it will modify the base pointer
    //read x 3, since there are 3 read ports
    ld1 {v0.2D, v1.2D, v2.2D}, [x1], x3 //post update
    ld1 {v0.2D, v1.2D, v2.2D}, [x1] //dependent on previous inst, these two ld1 cost at least 2 cycles.

    //update offset
    //dependency chain, at least 2 cycles
    add x3, x3, #96
    and x3, x3, x7

    add x8, x8, #96
    cmp x0, x8
    b.ne 1b
    ret

//SIMD read
_Read_4x1_ldur:
1:
    ldur s0, [x1]
    subs x0, x0, #4
    bgt 1b
    ret

_Read_4x1_ldr:
1:
    ldr w2, [x1]
    subs x0, x0, #4
    bgt 1b
    ret

_Read_4x2_ldur:
1:
    ldur s0, [x1]
    ldur s0, [x1]
    subs x0, x0, #8
    bgt 1b
    ret

_Read_4x2_ldr:
1:
    ldr w2, [x1]
    ldr w2, [x1]
    subs x0, x0, #8
    bgt 1b
    ret

_Read_4x3_ldur:
1:
    ldur s0, [x1]
    ldur s0, [x1]
    ldur s0, [x1]
    subs x0, x0, #12
    bgt 1b
    ret

_Read_4x3_ldr:
1:
    ldr w2, [x1]
    ldr w2, [x1]
    ldr w2, [x1]
    subs x0, x0, #12
    bgt 1b
    ret


_Read_4x4_ldur:
1:
    ldur s0, [x1]
    ldur s0, [x1]
    ldur s0, [x1]
    ldur s0, [x1]
    subs x0, x0, #16
    bgt 1b
    ret

_Read_4x4_ldr:
1:
    ldr w2, [x1]
    ldr w2, [x1]
    ldr w2, [x1]
    ldr w2, [x1]
    subs x0, x0, #16
    bgt 1b
    ret


_Read_8x3_ldr:
1:
    ldr x2, [x1]
    ldr x2, [x1]
    ldr x2, [x1]
    subs x0, x0, #24
    bgt 1b
    ret

_Read_8x3_ld1:
1:
    ld1 {V0.1D}, [x1]
    ld1 {V0.1D}, [x1]
    ld1 {V0.1D}, [x1]
    subs x0, x0, #24
    bgt 1b
    ret

_Read_16x3:
1:
    ld1 {V0.2D}, [x1]
    ld1 {V0.2D}, [x1]
    ld1 {V0.2D}, [x1]
    subs x0, x0, #48
    bgt 1b
    ret

_Read_16x4x1:
1:
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    subs x0, x0, #64
    bgt 1b
    ret

_Read_16x4x2:
1:
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    subs x0, x0, #128
    bgt 1b
    ret

_Read_16x4x3:
1:
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    ld1 {V0.2D, V1.2D, v2.2D, v3.2D}, [x1]
    subs x0, x0, #192
    bgt 1b
    ret



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
    ldrsb x3, [x1]
    subs x0, x0, #0x1
    bgt 1b
    ret

_Read_x2:
1:
    ldrsb x3, [x1]
    ldrsb x3, [x1]
    subs x0, x0, #0x2
    bgt 1b
    ret

_Read_x3:
1:
    ldrsb x3, [x1]
    ldrsb x3, [x1]
    ldrsb x3, [x1]
    subs x0, x0, #0x3
    bgt 1b
    ret

_Read_x4:
1:
    ldrsb x3, [x1]
    ldrsb x3, [x1]
    ldrsb x3, [x1]
    ldrsb x3, [x1]
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

