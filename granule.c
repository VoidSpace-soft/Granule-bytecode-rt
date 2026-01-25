#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct intvar {
    char addr[32];
    int value[32];
    int lastaddr;
};

struct strvar {
    char addr[32];
    char value[32][255];
};

int find_index(char arr[], int size, char value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value)
            return i;
    }
    return -1; // not found
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s program.bin\n", argv[0]);
        return 1;
    }
    if (argc > 1 && strcmp(argv[1], "--version") == 0) {
        printf("Granule Bytecode VM version 1.0ler-bb.\n");
        printf("usable by any language-dev in any way.\n");
        printf("fully opensource.\n");
        return 0;
    }
    struct intvar intvars = {0};
    struct strvar strvars = {0};

    intvars.lastaddr = 0;
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Failed to open bytecode file");
        return 1;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    unsigned char *code = malloc(size);
    if (!code) {
        perror("Memory allocation failed");
        fclose(file);
        return 1;
    }

    fread(code, 1, size, file);
    fclose(file);

    // Instruction pointer
    long ip = 0;
    long cstack[32];
    int lastret = 0;

    while (ip < size) {
        unsigned char instr = code[ip++];
        //printf("IP=%ld INSTR=0x%02X\n", ip-1, instr);

        switch (instr) {
            case 0x10:
                if (code[ip] == 0x12) {
                    int size = sizeof(intvars.addr)/sizeof(intvars.addr[0]);
                    char addr = code[ip+1];
                    int index = find_index(intvars.addr, intvars.lastaddr, addr);
                    if (index != -1) {
                        printf("%d", intvars.value[index]);
                        ip++;
                        ip++;
                        break;
                    } else {
                        printf("Unknown IntVar %c.\n", code[ip+1]);
                        return 1;
                    }
                }
                while (ip < size && code[ip] != 0x00) {
                    putchar(code[ip]);
                    ip++;
                }
                ip++; // skip terminator
                break;
            case 0x14:
                printf("RTError at byte %ld: ", ip);
                if (code[ip++] == 0x12) {
                    char addr = code[ip++];
                    int index = find_index(intvars.addr, 32, addr);
                    if (index != -1) {
                        printf("%d", intvars.value[index]);
                        ip++;
                        ip++;
                        printf("\n");
                    } else {
                        int index2 = find_index(strvars.addr, 32, addr);
                        if (index2 == -1) {
                            printf("Unknown IntVar %c.\n", addr);
                            return 1;
                        } else {
                            printf("%254s", strvars.value[index]);
                            return 1;
                        }
                    }
                }
                while (ip < size && code[ip] != 0x00) {
                    putchar(code[ip]);
                    ip++;
                }
                printf("\n");
                ip++; // skip terminator
                return 1;
            case 0x11: {
                if (intvars.lastaddr >= 32) {
                    printf("RTError at byte %ld: ", ip); printf("Too many intvars!\n");
                    return 1;
                }

                char addr = code[ip++];
                int value = 0;
                for (int j = 0; j < 4; j++) {
                    value = (value << 8) | code[ip++];
                }

                intvars.addr[intvars.lastaddr] = addr;
                intvars.value[intvars.lastaddr] = value;
                intvars.lastaddr++;
                break;
            }
            case 0x20: {
                char addr = code[ip++];
                unsigned char length = code[ip++];

                int index = find_index(strvars.addr, 32, addr);
                if (index == -1) {
                    // find first empty slot
                    for (index = 0; index < 32; index++) {
                        if (strvars.addr[index] == 0) {
                            strvars.addr[index] = addr;
                            break;
                        }
                    }
                    if (index == 32) {
                        printf("RTError at byte %ld: ", ip); printf("Too many string vars!\n");
                        return 1;
                    }
                }

                // limit copy to avoid overflow
                int copy_len = (length < 254) ? length : 254;
                if (ip + copy_len > size) {
                    printf("RTError at byte %ld: ", ip); printf("Bytecode truncated while reading string var '%c'\n", addr);
                    return 1;
                }

                for (int j = 0; j < copy_len; j++) {
                    strvars.value[index][j] = code[ip++];
                }
                strvars.value[index][copy_len] = '\0'; // null terminate
                break;
            }
            case 0x21: {
                char addr = code[ip++];
                int index = find_index(strvars.addr, 32, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: ", ip); printf("Unknown string var %c\n", addr);
                    return 1;
                }
                printf("%s", strvars.value[index]);
                break;
            }
            case 0x25: {
                printf("\033[2J\033[H");
                break;
            }
            case 0x26: {
                // read the intvar address
                char addr = code[ip++];

                // find the index
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: Unknown IntVar '%c' for if check!\n", ip, addr);
                    return 1;
                }

                // check the value
                if (intvars.value[index] == 0) {
                    // skip until 0x00 (terminator)
                    while (ip < size && code[ip] != 0xEF) {
                        ip++;
                    }
                    ip++; // skip the 0x00 itself
                }
                // else, do nothing, continue normally
                break;
            }
            case 0x30: {
                // read the intvar address
                char addr = code[ip++];

                // find the index
                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: Unknown IntVar '%c' for if check!\n", ip, addr);
                    return 1;
                }

                // check the value
                if (intvars.value[index] != 0) {
                    // skip until 0x00 (terminator)
                    while (ip < size && code[ip] != 0xEF) {
                        ip++;
                    }
                    ip++; // skip the 0x00 itself
                }
                // else, do nothing, continue normally
                break;
            }
            case 0x22: {
                long addr = code[ip++];
                cstack[lastret++] = ip;
                ip = addr;
                break;
            }
            case 0x31: {
                long addr = code[ip++];
                ip = addr;
                break;
            }
            case 0x32: {
                lastret = 0;
                break;
            }
            case 0xEF: {
                break;
            }
            case 0x27: {
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(intvars.addr, 32, a);
                int bvi = find_index(intvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);
                int av;
                int bv;
                if (avi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    av = intvars.value[avi];
                }
                if (bvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    bv = intvars.value[bv];
                }
                if (cvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c to write to\n", a);
                    return 1;
                } else {
                    if (av == bv) {
                        intvars.value[cvi] = 1;
                    } else intvars.value[cvi] = 0;
                    break;
                }
            }
            case 0x28: {
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(intvars.addr, 32, a);
                int bvi = find_index(intvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);
                int av;
                int bv;
                if (avi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    av = intvars.value[avi];
                }
                if (bvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    bv = intvars.value[bv];
                }
                if (cvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c to write to\n", a);
                    return 1;
                } else {
                    if (av > bv) {
                        intvars.value[cvi] = 1;
                    } else intvars.value[cvi] = 0;
                    break;
                }
            }
            case 0x29: {
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(intvars.addr, 32, a);
                int bvi = find_index(intvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);
                int av;
                int bv;
                if (avi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    av = intvars.value[avi];
                }
                if (bvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    bv = intvars.value[bv];
                }
                if (cvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c to write to\n", a);
                    return 1;
                } else {
                    if (av < bv) {
                        intvars.value[cvi] = 1;
                    } else intvars.value[cvi] = 0;
                    break;
                }
            }
            case 0x2A: {
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(intvars.addr, 32, a);
                int bvi = find_index(intvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);
                int av;
                int bv;
                if (avi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    av = intvars.value[avi];
                }
                if (bvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c\n", a);
                    return 1;
                } else {
                    bv = intvars.value[bv];
                }
                if (cvi == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No intvar at addr %c to write to\n", a);
                    return 1;
                } else {
                    if (av != bv) {
                        intvars.value[cvi] = 1;
                    } else intvars.value[cvi] = 0;
                    break;
                }
            }

            // String comparisons
            case 0x2B: { // seq: a == b -> c
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(strvars.addr, 32, a);
                int bvi = find_index(strvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);

                if (avi == -1 || bvi == -1 || cvi == -1) {
                    printf("RTError at byte %ld: ", ip);
                    if (avi == -1) printf("No strvar at addr %c\n", a);
                    else if (bvi == -1) printf("No strvar at addr %c\n", b);
                    else printf("No intvar at addr %c to write to\n", c);
                    return 1;
                }

                intvars.value[cvi] = (strcmp(strvars.value[avi], strvars.value[bvi]) == 0) ? 1 : 0;
                break;
            }
            case 0x2E: { // sneq: a != b -> c
                char a = code[ip++];
                char b = code[ip++];
                char c = code[ip++];
                int avi = find_index(strvars.addr, 32, a);
                int bvi = find_index(strvars.addr, 32, b);
                int cvi = find_index(intvars.addr, 32, c);

                if (avi == -1 || bvi == -1 || cvi == -1) {
                    printf("RTError at byte %ld: ", ip);
                    if (avi == -1) printf("No strvar at addr %c\n", a);
                    else if (bvi == -1) printf("No strvar at addr %c\n", b);
                    else printf("No intvar at addr %c to write to\n", c);
                    return 1;
                }

                intvars.value[cvi] = (strcmp(strvars.value[avi], strvars.value[bvi]) != 0) ? 1 : 0;
                break;
            }
            case 0x24: {
                char addr = code[ip++];
                int index = find_index(strvars.addr, 32, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: ", ip); printf("No strvar at addr %c to write to.\n", addr);
                    return 1;
                } else {
                    scanf("%254s", strvars.value[index]);
                    break;
                }
            }
            case 0x23: {
                if (lastret != 0) {
                    ip = cstack[--lastret];
                }
                break;
            }

            case 0x13: {
                char addr = code[ip++];

                int value = 0;
                for (int j = 0; j < 4; j++) {
                   value = (value << 8) | code[ip++];
                }

                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: ", ip); printf("Unknown IntVar '%c' for increment!\n", addr);
                    return 1;
                }

                intvars.value[index] += value;
                break;
            }
            
            case 0x33: {
                char addr = code[ip++];

                int value = 0;
                for (int j = 0; j < 4; j++) {
                   value = (value << 8) | code[ip++];
                }

                int index = find_index(intvars.addr, intvars.lastaddr, addr);
                if (index == -1) {
                    printf("RTError at byte %ld: ", ip); printf("Unknown IntVar '%c' for decrement!\n", addr);
                    return 1;
                }

                intvars.value[index] -= value;
                break;
            }

            case 0xFF:  // END
                free(code);
                return 0;
            
            default:
                printf("RTError at byte %ld: ", ip); printf("Unknown instruction: 0x%02X\n", instr);
                free(code);
                return 1;
        }
    }

    free(code);
    return 0;
}