#include "sim86/sim86_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


enum register_mapping_8086
{
    Register_none,
    
    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,
    Register_es,
    Register_cs,
    Register_ss,
    Register_ds,
    Register_ip,
    Register_flags,
    
    Register_count,
};

#define GReg(name) \
        union {\
            struct {\
                u8 name##l, name##h;\
            };\
            u16 name##x;\
        };

struct Registers {
    union {
        struct {
            GReg(a)
            GReg(b)
            GReg(c)
            GReg(d)
            u16 sp;
            u16 bp;
            u16 si;
            u16 di;
            u16 es;
            u16 cs;
            u16 ss;
            u16 ds;
            u16 ip;
            u16 flags;
        };
        u8 mem[14*2];
    };
};

struct sim_context {
    u8 memory[1024*1024];
    Registers reg;
    bool S_Flag, Z_Flag;
};


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
        sprintf(text, "[%s + %s + 0x%X]", 
            Sim86_RegisterNameFromOperand((register_access*)&op.Address.Terms[0].Register), 
            Sim86_RegisterNameFromOperand((register_access*)&op.Address.Terms[1].Register),
            op.Address.Displacement);
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


template<typename T>
T read_register(const register_access& regtype, const u8* reg) {
    return *((T*)(&reg[(regtype.Index-1)*2 + regtype.Offset]));
}
template<typename T>
void write_register(const register_access& regtype, u8* reg, T val) {
    *((T*)&reg[(regtype.Index-1)*2 + regtype.Offset]) =  val;
}

//register/memory access
struct rm_access {
    u8* p;//pointer to the memory
    size_t s; //how many bytes to read & write
    s32 val; //to store immediate
};

rm_access build_rm_access(sim_context& ctx, instruction_operand& operand) {
    rm_access ret{};
    if (operand.Type == Operand_Register) {
        auto& reg = operand.Register;
        ret.p = &ctx.reg.mem[(reg.Index-1)*2 + reg.Offset];
        ret.s = reg.Count;
    }
    else if (operand.Type == Operand_Memory) {
        auto& access = operand.Address;
        //ignore segment registers, just accessing 64k memory
        auto r1 = access.Terms[0].Register;
        auto r2 = access.Terms[1].Register;
        //only dealing with 16bits registers, ignoring 8bits registers
        assert(r1.Count == 2 && r2.Count == 2);

        auto term1 = (r1.Index == Register_none)
            ? 0 
            : read_register<u16>(r1, ctx.reg.mem);
        auto term2 = (r2.Index == Register_none)
            ? 0
            : read_register<u16>(r2, ctx.reg.mem);
        //absolute address = [term1 + term1 + disp]
        u16 addr = term1 + term2 + access.Displacement;
        ret.p = ctx.memory + addr;
        ret.s = 2;//only read&write 2 bytes
    }
    else if (operand.Type == Operand_Immediate) {
        ret.val = operand.Immediate.Value;
    }
    return ret;
}

s32 rm_access_read(rm_access& access) {
    if (access.s == 1) {
        return (s32)(*access.p);
    }
    else if (access.s == 2) {
        return (s32)*((uint16_t*)access.p);
    }
    else if (access.s == 0) {
        return (s32)access.val;
    }
    assert(false);
    return 0;
}

void rm_access_write(rm_access& o, s32 value) {
    if (o.s == 1) {
        *o.p = (s8)value;
    }
    else if (o.s == 2) {
        *((s16*)o.p) = (s16)value;
    }
    else {
        //immediate can't be written
        assert(false);
    }
}

void execute(sim_context& ctx, instruction& inst) {
    print_instruction(inst);
    switch (inst.Op)
    {
    case Op_mov: {
        rm_access o1 = build_rm_access(ctx, inst.Operands[0]);
        rm_access o2 = build_rm_access(ctx, inst.Operands[1]);

        rm_access_write(o1, rm_access_read(o2));
    } break;
    case Op_add:
    case Op_cmp:
    case Op_sub: {
        auto op1 = inst.Operands[0];
        auto op2 = inst.Operands[1];
        //like Casy's code, I'll try to build a "access" object to unify
        //the read/write operations to both memory and registers
        rm_access o1 = build_rm_access(ctx, op1);
        rm_access o2 = build_rm_access(ctx, op2);
        auto v1 = rm_access_read(o1);
        auto v2 = rm_access_read(o2);
        s32 v = {};
        if (inst.Op == Op_add) {
            v = v1 + v2;
        }
        else if (inst.Op == Op_sub || inst.Op == Op_cmp) {
            v = v1 - v2;
        }

        if (inst.Op != Op_cmp) {
            rm_access_write(o1, v);
        }
        s16 v16;s8 v8;
        if (o1.s == 1) {
            v8 = v;
        }
        else if (o1.s == 2) {
            v16 = v;
        }
        bool old_z = ctx.Z_Flag;
        bool old_s = ctx.S_Flag;
        ctx.Z_Flag = (o1.s == 1 ? (v8 == 0) : (v16 == 0));
        ctx.S_Flag = (o1.s == 1 ? (v8 < (s8)0) : (v16 < (s16)0));

        if (old_z != ctx.Z_Flag || old_s != ctx.S_Flag) {
            printf("Flag: ");
            if (!old_z && ctx.Z_Flag) printf("->Z");
            if (old_z && !ctx.Z_Flag) printf("Z->");
            if (!old_s && ctx.S_Flag) printf("->S");
            if (old_s && !ctx.S_Flag) printf("S->");
        }
    } break;
    case Op_jne: {
        //jump on not equal to zero
        s16 offset = inst.Operands[0].Immediate.Value;
        if (!ctx.Z_Flag) 
            ctx.reg.ip += offset;
    } break;
    default:
        break;
    }

    printf(" ip: 0x%X \n", 
    read_register<uint16_t>({Register_ip, 0, 2}, ctx.reg.mem));
}

int main(int argc, char *argv[]) {
    //feed a file
    assert(argc > 1);

    sim_context context;

    char* FileName = argv[1];
    u32 bytes = load_memory_from_file(FileName, context.memory);
    assert(bytes > 0);

    instruction inst{};
    context.reg.ip = 0;
    do {
        //calcuate absolute address
        auto addr = context.memory + context.reg.ip;
        Sim86_Decode8086Instruction(bytes, addr, &inst);
        context.reg.ip += inst.Size;

        execute(context, inst);

    } while (inst.Op != Op_None && context.reg.ip < bytes);
    
    printf("Final Registers: \n");
    printf("    bx: 0x%X\n", context.reg.bx);
    printf("    cx: 0x%X\n", context.reg.cx);
    printf("    dx: 0x%X\n", context.reg.dx);
    printf("    bp: 0x%X\n", context.reg.bp);
    printf("    si: 0x%X\n", context.reg.si);
    printf("    ip: 0x%X\n", context.reg.ip);
    printf("flags: ");
    if (context.Z_Flag) printf("Z");
    if (context.S_Flag) printf("S");
    return 0; 
}

