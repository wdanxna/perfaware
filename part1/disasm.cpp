#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>

const std::string filepath =
    "../computer_enhance/perfaware/part1/listing_0038_many_register_mov";

std::string get_register(uint8_t reg, uint8_t w) {
  static const std::string tb[2 * 8] = {"al", "cl", "dl", "bl", "ah", "ch",
                                        "dh", "bh", "ax", "cx", "dx", "bx",
                                        "sp", "bp", "si", "di"};
  return tb[w * 8 + reg];
}

std::string get_effective_address(uint8_t rm, uint8_t mod, std::ifstream &s) {
  std::string dest;
  switch (rm) {
  case 0b000:
    dest = "bx + si";
    break;
  case 0b001:
    dest = "bx + di";
    break;
  case 0b010:
    dest = "bp + si";
    break;
  case 0b011:
    dest = "bp + di";
    break;
  case 0b100:
    dest = "si";
    break;
  case 0b101:
    dest = "di";
    break;
  case 0b110:
    dest = "bp";
    break;
  case 0b111:
    dest = "bx";
    break;
  default:
    break;
  }

  switch (mod) {
  case 0b00: {
    // no displacement except when rm=110, then 16bits displacement follows
    if (rm == 0b110) {
      int16_t dis{}; // lo, hi. little endian machine
      if (!s.read((char *)&dis, sizeof(int16_t))) {
        assert(false);
      }
      dest = std::to_string(dis);
    }
    break;
  }
  case 0b01: {
    // 8bits displacement
    int8_t dis{};
    if (!s.read((char *)&dis, sizeof(int8_t))) {
      assert(false);
    }
    if (dis != 0)
      dest += (dis > 0 ? " + " : " - ") + std::to_string(std::abs(dis));
    break;
  }
  case 0b10: {
    // 16bits displacement
    int16_t dis{}; // lo, hi. little endian machine
    if (!s.read((char *)&dis, sizeof(int16_t))) {
      assert(false);
    }
    if (dis != 0)
      dest += (dis > 0 ? " + " : " - ") + std::to_string(std::abs(dis));
    break;
  }
  default:
    break;
  }

  return "[" + dest + "]";
}

std::string next_immediate(uint8_t w, std::ifstream &s, bool prefix = true) {
  if (w == 0) {
    // w = 0, 8bit immediate
    int8_t imm{};
    if (!s.read((char *)&imm, sizeof(int8_t))) {
      assert(false);
    }
    auto ret = std::to_string(imm);
    return prefix ? "byte " + ret : ret;
  } else {
    // w = 1, 16bits immediate
    int16_t imm{};
    if (!s.read((char *)&imm, sizeof(int16_t))) {
      assert(false);
    }
    auto ret = std::to_string(imm);
    return (prefix && imm <= 255 && imm >= -256) ? "word " + ret : ret;
  }
}

std::tuple<uint8_t, uint8_t, uint8_t>
next_mod_reg_rm(std::ifstream &s) {
  char b{};
  if (!s.read(&b, sizeof(char))) {
    assert(false);
  }
  uint8_t mod = (b & 0b11000000) >> 6;
  uint8_t reg = (b & 0b000111000) >> 3;
  uint8_t rm = (b & 0b000000111);
  return {mod, reg, rm};
}

std::tuple<std::string, std::string>
next_reg_mem(char op, std::ifstream &s) {
  auto [mod, reg, rm] = next_mod_reg_rm(s);
  uint8_t dw = (op & 0b00000011);
  std::string src, dest;
  src = get_register(reg, dw & 1);
  switch (mod) {
  // register to register
  case 0b11:
    dest = get_register(rm, dw & 1);
    break;
  // memory mode
  default:
    dest = get_effective_address(rm, mod, s);
    break;
  }
  if (dw & 0b10) {
    // d is 1, use reg as destination
    std::swap(src, dest);
  }
  return {src, dest};
}

std::string register_memory_to_from_register(char op, std::ifstream &s) {
  auto [src, dest] = next_reg_mem(op, s);
  return (std::stringstream() << "mov " << dest << "," << src).str();
}

std::string immediate_to_memory(char op, std::ifstream &s) {
  char b2{};
  if (!s.read(&b2, sizeof(char))) {
    assert(false);
  }
  uint8_t mod = (b2 & 0b11000000) >> 6;
  uint8_t rm = (b2 & 0b000000111);
  // get destination address first, since the displacement located at next 2
  // bytes
  std::string dest = get_effective_address(rm, mod, s);
  std::string src = next_immediate(op & 1, s);
  // based on w get the immediate number
  return "mov " + dest + ", " + src;
}

std::string immediate_to_register(char op, std::ifstream &s) {
  uint8_t w = (op & 0b00001000) >> 3;
  uint8_t reg = (op & 0b00000111);
  std::string dest = get_register(reg, w);
  std::string src = next_immediate((op & 0b1000) >> 3, s);
  return "mov " + dest + ", " + src;
}

std::string memory_to_accumulator(char op, std::ifstream &s) {
  return "mov ax, [" + next_immediate(op & 1, s, false) + "]";
}

std::string accumulator_to_memory(char op, std::ifstream &s) {
  return "mov [" + next_immediate(op & 1, s, false) + "], ax";
}

using SS = std::stringstream;
std::string add_cmp_sub_rm_to_rm(char op, std::ifstream &s) {
    std::string inst;
    switch ((op & 0b111000)>>3)
    {
    //ADD
    case 0b000:
        inst = "add";
        break;
    case 0b111:
        inst = "cmp";
        break;
    //SUB
    case 0b101:
        inst = "sub";
    default:
        break;
    }

    auto [src, dest] = next_reg_mem(op, s);
    return (SS() << inst << " " << dest << ", " << src).str();
}

std::string add_cmp_sub_imm_to_acc(char op, std::ifstream &s) {
    std::string inst;
    switch ((op & 0b111000)>>3)
    {
    //ADD
    case 0b000:
        inst = "add";
        break;
    //CMP
    case 0b111:
        inst = "cmp";
        break;
    //SUB
    case 0b101:
        inst = "sub";
    default:
        break;
    }
    auto imm = next_immediate(op&1, s);
    return (SS() << inst << " ax, " << imm).str();
}

std::string add_cmp_sub_imm_to_rm(char op, std::ifstream &s) {
    auto [mod, reg, rm] = next_mod_reg_rm(s);
    std::string inst;
    switch (reg)
    {
    //ADD
    case 0b000:
        inst = "add";
        break;
    //CMP
    case 0b111:
        inst = "cmp";
        break;
    //SUB
    case 0b101:
        inst = "sub";
        break;
    default:
        break;
    }

    std::string dest;
    if (mod == 0b11) {
        dest = get_register(reg, op&1);
    }
    else {
        dest = get_effective_address(rm, mod, s);
    }

    uint8_t signext = (op&0b10) >> 1;
    uint8_t w = (op&1);
    std::string src = next_immediate((!signext)&w, s);

    return (SS() << inst << " " << dest << ", " << src).str();
}

int8_t next_INC8(std::ifstream& s) {
    //next 8-bit signed increment (IP-INC8)
    int8_t inc{};
    if (!s.read((char *)&inc, sizeof(int8_t))) {
      assert(false);
    }
    return inc;
}

//conditional jump
std::string jxx(const std::string& instruct, std::ifstream &s) {
    return (SS() << instruct << " " << std::to_string(next_INC8(s))).str();
}

std::string decode(uint8_t op, std::ifstream &s) {
  switch (op) {
  case 0b01110101:
    return jxx("jnz", s);
  case 0b01110100:
    return jxx("je", s);
  case 0b01111100:
    return jxx("jl", s);
  case 0b01111110:
    return jxx("jle", s);
  case 0b01110010:
    return jxx("jb", s);
  case 0b01110110:
    return jxx("jbe", s);
  case 0b01111010:
    return jxx("jp", s);
  case 0b01110000:
    return jxx("jo", s);
  case 0b01111000:
    return jxx("js", s);
  case 0b01111101:
    return jxx("jnl", s);
  case 0b01111111:
    return jxx("jg", s);
  case 0b01110011:
    return jxx("jnb", s);
  case 0b01110111:
    return jxx("ja", s);
  case 0b01111011:
    return jxx("jnp", s);
  case 0b01110001:
    return jxx("jno", s);
  case 0b01111001:
    return jxx("jns", s);
  case 0b11100010:
    return jxx("loop", s);
  case 0b11100001:
    return jxx("loopz", s);
  case 0b11100000:
    return jxx("loopnz", s);
  case 0b11100011:
    return jxx("jcxz", s);
  case 0b10001100:
    // segment register to register/memory
    break;
  case 0b10001110:
    // register/memory to segment register
    break;
  default:
    switch (op & 0b11111110) {
    case 0b11000110:
      // immediate to register/memory
      // in the manual, this op is called "immediate to register/memory", but
      // it actually just move data to memory, never to register.
      return immediate_to_memory(op, s);
    case 0b10100000:
      // memory to accumulator
      return memory_to_accumulator(op, s);
    case 0b10100010:
      // accumulator to memory
      return accumulator_to_memory(op, s);
    //ADD & CMP & SUB imm to accumulator
    case 0b00000100:
    case 0b00111100:
    case 0b00101100:
        return add_cmp_sub_imm_to_acc(op, s);
    default:
      switch (op & 0b11111100) {
      //ADD & CMP & SUB rm to rm
      case 0b00000000:
      case 0b00111000:
      case 0b00101000:
        return add_cmp_sub_rm_to_rm(op, s);
      //ADD & CMP & SUB imm to rm
      case 0b10000000:
        return add_cmp_sub_imm_to_rm(op, s);

      // register/memory to/from register
      case 0b10001000:
        return register_memory_to_from_register(op, s);
      default:
        switch (op & 0b11110000) {
        case 0b10110000:
          // immediate to register
          return immediate_to_register(op, s);
        default:
          break;
        }
        break;
      }
      break;
    }
    break;
  }
  return "";
}

std::string decode(std::ifstream &s) {
  uint8_t op{};
  if (!s.read((char*)&op, sizeof(char)))
    return "";
  return decode(op, s);
}

int main(int argc, char *argv[]) {
  assert(argc == 2);
  std::ifstream file(argv[1], std::ios::binary);
  while (!file.eof()) {
    std::cout << decode(file) << "\n";
  }
  file.close();
}