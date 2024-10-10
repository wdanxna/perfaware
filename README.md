My homework assignment for performance awareness programming

## 10/10/2024

simulating add, sub, cmp. which all of three share the almost same code path.
again, runtime dispatch seems neccessary to distinguish between different register sizes

---

Redesigned register storage, make it more explicit.
implement ip manipulation, simulating jne instruction, completing LISTING 48, 49.

---

Unify the operand access throuh rm_access object for `MOV`. completing LISTING 51, 52.
simulating memory access without considering segment registers and 8bits registers, so only 64K is addressable.

---

Successfully running LISTING 54 a "real" program, creating a 64x64x4 image, dumping the raw image into a file and check with GIMP

![alt text](dump.jpg "Title")

## 9/10/2024
I decided to use Casys's Sim86 as basis for future assignments since its data structure is better designed and I want to study its code and design style. I imported its Sim86 as library into my homework workspace, write a main.cpp for assignment 4 which is simulating `MOV reg, imm` and `MOV reg, reg` instructions.

Implementation: use Sim86 to decode (deseriallize) the binary into data structure one instruction at a time, setup a context data structure to represent a 8086 machine, which now is just a 1MB buffer for memory and 8*2 bytes buffer for registers. write an `execute` function which receives a instruction and a context, which will update the context based on the instruction.

the current `execute` is implemented with just 2 ugly if-elses for simulating the 2 MOVs that LISTING 43 required.

----
Do challenge LISTING 45, be able to read/write into 8bit registers. write a utility tempalte function `read_register` and `wirte_register` that can operate on buffers in accordance with type been specified. A dynamic dispatch (with if-elses) seems needed in order to discern which register to access

## 06/10/2024
Complete assignment 1, 2 and challenge assignment:
An 8086 disassembler, can decode all MOV instructions now

assignment 3: implement ADD, SUB, CMP, JNZ decoding



