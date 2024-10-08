#include "sim86/sim86_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

u32 load_memory_from_file(char *FileName, u8 *memory)
{
    u32 read = 0;
    FILE *File = fopen(FileName, "rb");
    if (File)
    {
        read = fread(memory, 1, 1024 * 1024, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }
    return read;
}

void print_operand(const instruction_operand& op, char* text) {
    switch (op.Type)
    {
    case Operand_Register: {
        const char* reg = Sim86_RegisterNameFromOperand(const_cast<register_access*>(&op.Register));
        sprintf(text, "%s", reg);
        break;
    }
    case Operand_Immediate: {
        sprintf(text, "0x%X", op.Immediate.Value);
        break;
    }
    case Operand_Memory: {
        //TODO: print all terms?
        sprintf(text, "0x%X", op.Address.Displacement);
        break;
    }
    default:
        assert(false);
        break;
    }
}

void print_instruction(const instruction& inst) {
    const char* op = Sim86_MnemonicFromOperationType(inst.Op);
    char dest[16]{}, src[16]{};
    u16 operand_cnt = 0;
    if (inst.Operands[0].Type != Operand_None) {
        operand_cnt++;
        print_operand(inst.Operands[0], dest);
    }

    if (inst.Operands[1].Type != Operand_None) {
        operand_cnt++;
        print_operand(inst.Operands[1], src);
    }
    if (operand_cnt == 0) {
        printf("%s", op);
    }
    else if (operand_cnt == 1) {
        printf("%s %s", op, dest);
    }
    else {
        printf("%s %s, %s", op, dest, src);
    }
}

struct sim_context {
    u8* memory;
    u8* reg;
};

template<typename T>
T read_register(const register_access& regtype, const u8* reg) {
    return *((T*)(&reg[(regtype.Index-1)*2 + regtype.Offset]));
}
template<typename T>
void write_register(const register_access& regtype, u8* reg, T val) {
    *((T*)&reg[(regtype.Index-1)*2 + regtype.Offset]) =  val;
}

void execute(sim_context& ctx, instruction& inst) {
    switch (inst.Op)
    {
    case Op_mov: {
        //only consider immediate to register mov for LISTING 43
        if (inst.Operands[0].Type == Operand_Register &&
            inst.Operands[1].Type == Operand_Immediate) {
                auto& reg = inst.Operands[0].Register;
                auto& imm = inst.Operands[1].Immediate;
                //just for 16bits registers
                auto old_val = read_register<uint16_t>(reg, ctx.reg);
                write_register<uint16_t>(reg, ctx.reg, imm.Value);
                print_instruction(inst);
                printf("; %s: 0x%X -> 0x%X\n", 
                    Sim86_RegisterNameFromOperand(&reg), 
                    old_val, 
                    read_register<uint16_t>(reg, ctx.reg));
            }
        else if (inst.Operands[0].Type == Operand_Register &&
                inst.Operands[1].Type == Operand_Register) {
                    auto& dest = inst.Operands[0].Register;
                    auto& src = inst.Operands[1].Register;
                    auto old_val = read_register<uint16_t>(dest, ctx.reg);
                    write_register<uint16_t>(dest, ctx.reg, read_register<uint16_t>(src, ctx.reg));
                    print_instruction(inst);
                    printf("; %s: 0x%X -> 0x%X\n", 
                        Sim86_RegisterNameFromOperand(&dest), 
                        old_val, 
                        read_register<uint16_t>(dest, ctx.reg));
                }
        break;
    }
    default:
        break;
    }
}

int main(int argc, char *argv[]) {

    //feed a file
    assert(argc > 1);

    sim_context context;
    context.memory = new u8[1024*1024];//1 MB
    context.reg = new u8[8 * 2]{}; //8 2bytes registers

    char* FileName = argv[1];
    u32 bytes = load_memory_from_file(FileName, context.memory);
    assert(bytes > 0);

    instruction inst{};
    u8* head = context.memory;
    do {
        head += inst.Size;
        Sim86_Decode8086Instruction(bytes, head, &inst);

        execute(context, inst);

        bytes -= inst.Size;
    } while (inst.Op != Op_None && bytes > 0);
    

    delete[] context.memory;
    delete[] context.reg;
    return 0;
}

