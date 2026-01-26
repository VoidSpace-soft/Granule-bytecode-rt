#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>
#include <cstring>

struct IntVars {
    char addr[32];
    int value[32];
    int lastaddr;
};

struct StrVars {
    char addr[32];
    std::string value[32];
    int lastaddr;
};

int find_index(const char* list, int size, char value) {
    for (int i = 0; i < size; i++) {
        if (list[i] == value) {
            return i;
        }
    }
    return -1; // not found
}

int run(const std::vector<uint8_t>& program, bool dotrace=false) {
    size_t ip = 0;
    IntVars intvars = {0};
    intvars.lastaddr = 0;
    StrVars strvars = {0};
    strvars.lastaddr = 0;
    int cstack[32] = {0};
    int lastret = 0;
    bool headed = false;

    while (ip < program.size()) {
        uint8_t opcode = program[ip++];
        if (dotrace) {
            std::cout << "BYTENUM=" << ip << std::endl;
            std::cout << "BYTEDEC=" << (int)opcode << std::endl;
            std::cout << "BYTEEXACT=" << (char)opcode << std::endl;
            std::cout << "NUMINTVARS=" << intvars.lastaddr << std::endl;
            std::cout << "NUMSTRVARS=" << strvars.lastaddr << std::endl;
            std::cout << "CALLS ::" << std::endl;
            for (int i = 0; i < lastret; i++) {
                std::cout << cstack[i] << std::endl;
            }
            std::cout << "stdout/stdin ::" << std::endl;
        }

        switch (opcode) {
            case 0x10: {
                // Output bytes until 0x00
                while (ip < program.size() && program[ip] != 0x00) {
                    std::cout << static_cast<char>(program[ip]);
                    ip++;
                }

                // Skip the 0x00 terminator
                if (ip < program.size() && program[ip] == 0x00) {
                    ip++;
                }
                break;
            }

            case 0x14: {
                uint8_t addr = program[ip++];
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    std::cout << "RTError at byte " << ip << ": No intvar at addr " << (int)addr << std::endl;
                    return -1;
                }
                int value = intvars.value[index];
                if (value == 0) {
                    while (ip < program.size() && program[ip] != 0xFE) {
                        if (ip < program.size()) ip++;
                    }
                    ip++;
                    break;
                }
                break;
            }

            case 0x15: {
                uint8_t addr = program[ip++];
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    std::cout << "RTError at byte " << ip << ": No intvar at addr " << (int)addr << std::endl;
                    return -1;
                }
                int value = intvars.value[index];
                if (value != 0) {
                    while (ip < program.size() && program[ip] != 0xFE) {
                        if (ip < program.size()) ip++;
                    }
                    ip++;
                    break;
                }
                break;
            }

            case 0x16: {
                while (ip < program.size() && program[ip] != 0xFD) {
                    if (ip < program.size()) ip++;
                }
                ip++;
                break;
            }
            
            case 0x17: {
                int vala = program[ip++];
                int valb = program[ip++];
                int jaddr = (vala << 8) | valb;
                cstack[lastret++] = ip;
                ip = jaddr;
                break;
            }

            case 0x18: {
                if (lastret != 0) {
                    ip = cstack[--lastret];
                }
                break;
            }

            case 0x19: { // read string
                uint8_t var_id = program[ip++];
                std::string input;
                std::getline(std::cin, input);
                if (var_id >= strvars.lastaddr) {
                    std::cerr << "RTError at byte " << ip << ": input into unknown strvar " << (int)var_id << "\n";
                    return -1;
                }
                strvars.value[var_id] = input;
                break;
            }

            case 0x1A: { // read int
                uint8_t var_id = program[ip++];
                int value;
                std::cin >> value;
                if (var_id >= intvars.lastaddr) {
                    std::cerr << "RTError at byte " << ip << ": input into unknown intvar " << (int)var_id << "\n";
                    return -1;
                }
                intvars.value[var_id] = value;
                break;
            }

            case 0x1B: { // integer equality
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                int vala = intvars.value[indexa];
                int valb = intvars.value[indexb];
                intvars.value[indexc] = (vala == valb) ? 1 : 0;
                break;
            }

            case 0x1C: { // integer inequality
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                int vala = intvars.value[indexa];
                int valb = intvars.value[indexb];
                intvars.value[indexc] = (vala != valb) ? 1 : 0;
                break;
            }

            case 0x1D: { // integer more
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                int vala = intvars.value[indexa];
                int valb = intvars.value[indexb];
                intvars.value[indexc] = (vala > valb) ? 1 : 0;
                break;
            }

            case 0x1E: { // integer less
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                int vala = intvars.value[indexa];
                int valb = intvars.value[indexb];
                intvars.value[indexc] = (vala < valb) ? 1 : 0;
                break;
            }

            case 0x1F: { // string equality
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(strvars.addr, strvars.lastaddr, addra);
                int indexb = find_index(strvars.addr, strvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no strvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no strvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                std::string vala = strvars.value[indexa];
                std::string valb = strvars.value[indexb];
                intvars.value[indexc] = (vala == valb) ? 1 : 0;
                break;
            }

            case 0x20: { // string equality
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                uint8_t addrc = program[ip++];
                int indexa = find_index(strvars.addr, strvars.lastaddr, addra);
                int indexb = find_index(strvars.addr, strvars.lastaddr, addrb);
                int indexc = find_index(intvars.addr, intvars.lastaddr, addrc);
                if (indexa == -1) {
                    std::cout << "RTError at byteop 0x1B: no strvar at addr " << (int)addra;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byteop 0x1B: no strvar at addr " << (int)addrb;
                    return -1;
                }
                if (indexc == -1) {
                    std::cout << "RTError at byteop 0x1B: no intvar at addr " << (int)addrc;
                    return -1;
                }
                std::string vala = strvars.value[indexa];
                std::string valb = strvars.value[indexb];
                intvars.value[indexc] = (vala != valb) ? 1 : 0;
                break;
            }

            case 0x21: { // integer incrememnt
                long unsigned int oip = ip;
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                if (indexa == -1) {
                    std::cout << "RTError at byte " << oip << ": No intvar at addr " << (int)addra << std::endl;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byte " << oip << ": No intvar at addr " << (int)addrb << std::endl;
                    return -1;
                }
                intvars.value[indexa] += intvars.value[indexb];
                break;
            }

            case 0x22: { // integer decrement
                long unsigned int oip = ip;
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                int indexa = find_index(intvars.addr, intvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                if (indexa == -1) {
                    std::cout << "RTError at byte " << oip << ": No intvar at addr " << (int)addra << std::endl;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byte " << oip << ": No intvar at addr " << (int)addrb << std::endl;
                    return -1;
                }
                intvars.value[indexa] -= intvars.value[indexb];
                break;
            }

            case 0x23: { // string concate
                long unsigned int oip = ip;
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                int indexa = find_index(strvars.addr, strvars.lastaddr, addra);
                int indexb = find_index(strvars.addr, strvars.lastaddr, addrb);
                if (indexa == -1) {
                    std::cout << "RTError at byte " << oip << ": No strvar at addr " << (int)addra << std::endl;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byte " << oip << ": No strvar at addr " << (int)addrb << std::endl;
                    return -1;
                }
                strvars.value[indexa] += strvars.value[indexb];
                break;
            }

            case 0x24: { // string len op
                long unsigned int oip = ip;
                uint8_t addra = program[ip++];
                uint8_t addrb = program[ip++];
                int indexa = find_index(strvars.addr, strvars.lastaddr, addra);
                int indexb = find_index(intvars.addr, intvars.lastaddr, addrb);
                if (indexa == -1) {
                    std::cout << "RTError at byte " << oip << ": No strvar at addr " << (int)addra << std::endl;
                    return -1;
                }
                if (indexb == -1) {
                    std::cout << "RTError at byte " << oip << ": No strvar at addr " << (int)addrb << std::endl;
                    return -1;
                }
                intvars.value[indexb] = strvars.value[indexa].length();
                break;
            }

            case 0x25: {
                strvars.lastaddr--;
                break;
            }

            case 0x26: {
                intvars.lastaddr--;
                break;
            }

            case 0x27: {
                uint8_t addr = program[ip++];
                int vala = program[ip++];
                int valb = program[ip++];
                int nval = (vala << 8) | valb;
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    std::cout << "RTError at byte " << ip << ": No intvar at addr " << (int)addr << std::endl;
                    return -1;
                }
                intvars.value[index] = nval;
                break;
            }

            case 0x28: {
                uint8_t addr = program[ip++];
                std::string value = "";
                while (ip < program.size() && program[ip] != 0x00) {
                    value += static_cast<char>(program[ip++]);
                }
                ip++;
                if (ip >= program.size()) {
                    std::cout << "RTError at byte " << ip << ": Unexpected end of bytecode while reading string.\n";
                    return -1;
                }
                int index = find_index(strvars.addr, strvars.lastaddr, addr);
                if (index == -1) {
                    std::cout << "RTError at byte " << ip << ": No strvar at addr " << (int)addr << std::endl;
                    return -1;
                }
                strvars.value[index] = value;
                break;
            }

            case 0xFC: {
                uint8_t a = program[ip++];
                if ((int)a == 0xFF)
                    headed = true;
                break;
            }

            case 0xFE:
                break;

            case 0x12: {
                if (ip >= program.size()) {
                    std::cout << "RTError: Unexpected end of bytecode reading variable ID\n";
                    return -1;
                }
                uint8_t addr = program[ip++];
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    index = find_index(strvars.addr, strvars.lastaddr, addr);
                    if (index == -1) {
                        std::cout << "RTError at byte " << ip << ": No var at addr " << (int)addr << "\n" << std::endl;
                        return -1;
                    } else {
                        std::string value = strvars.value[index];
                        std::cout << value;
                    }
                } else {
                    int value = intvars.value[index];
                    std::cout << value;
                }
                break;
            }

            case 0x11: { // integer definition
                uint8_t addr = program[ip++];
                int vala = program[ip++];
                int valb = program[ip++];
                if (intvars.lastaddr >= 32) {
                    printf("RTError at byte %ld: ", ip); printf("Variable limit 32 max.");
                    return -1;
                }
                intvars.addr[intvars.lastaddr] = addr;
                intvars.value[intvars.lastaddr] = (vala << 8) | valb;
                intvars.lastaddr++;
                break;
            }

            case 0x13: { // string definition
                uint8_t addr = program[ip++];
                std::string value = "";
                while (ip < program.size() && program[ip] != 0x00) {
                    value += static_cast<char>(program[ip++]);
                }
                ip++;
                if (ip >= program.size()) {
                    std::cout << "RTError: Unexpected end of bytecode while reading string.\n";
                    return -1;
                }
                if (strvars.lastaddr >= 32) {
                    std::cout << "RTError at byte " << ip << ": Too many strvariables." << std::endl;
                    return -1;
                } else {
                    strvars.addr[strvars.lastaddr] = addr;
                    strvars.value[strvars.lastaddr] = value;
                    strvars.lastaddr++;
                    break;
                }
            }

            case 0xFF:
                if (dotrace) {
                    std::cout << "stdend;" << std::endl;
                }
                return 0;

            default:
                std::cerr << "Unknown opcode: 0x"
                          << std::hex << static_cast<int>(opcode)
                          << std::dec << "\n";
                return -1;
        }
        if (! headed) {
            std::cout << "RTError: No head" << std::endl;
            return -1;
        }
        if (dotrace) {
            std::cout << "stdend;" << std::endl;
        }
    }
    return 0;
}

/*int main(int argc, char* argv[]) {
    bool dotrace = false;
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode.bin>\n";
        return 1;
    }

    if (strcmp(argv[1], "--version") == 0) {
        std::cout << "Granule version 2.0 (C++ rewrite)" << std::endl;
        return 0;
    }
    if (argc >= 3) {
        if (strcmp(argv[2], "--trace") == 0) {
            dotrace = true;
        }
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file: " << argv[1] << "\n";
        return 1;
    }

    // Read entire file into memory
    std::vector<uint8_t> program(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    return run(program, dotrace);
}*/
