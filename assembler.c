#define _CRT_SECURE_NO_WARNINGS

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *globl;
int current = 0;
int instr_count;

#define MAX_STUFF (30000)

struct Symbol {
    char *name;
    int instr_num;
} symbols[MAX_STUFF] = {0};

struct Label {
    char *name;
    int instr_num;
} labels[MAX_STUFF] = { 0 };

int new_var_a = 16;

int num_labels = 0;

int num_syms = 0;

short instrs[MAX_STUFF];

int num_instrs = 0;

int find_sym(char *name) {
    for (int i = 0; i < num_syms; i++) {
        if (strcmp(name, symbols[i].name) == 0)
            return i;
    }
    return -1;
}

int find_label(char *name) {
    for (int i = 0; i < num_labels; i++) {
        if (strcmp(name, labels[i].name) == 0)
            return i;
    }
    return -1;
}

int isws(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int is_alpha(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || (c == '_');
}

int is_num(int c) {
    return ('0' <= c && c <= '9');
}

int is_alnum(int c) {
    return is_alpha(c) || is_num(c);
}

void skip_ws() {
    while (globl[current] && isws(globl[current])) {
        current++;
    }
}

void skip_rest_of_line() {
    while (globl[current] && globl[current] != '\n' && globl[current] != '\r') {
        current++;
    }

    if (globl[current] == '\r') {
        current++;
    }
    while (globl[current] == '\n') {
        current++;
    }
}

void skip_ws_no_nl() {
    while (globl[current] && isws(globl[current]) && globl[current] != '\n') {
        current++;
    }
}

void skip_whitespace_no_nl() {
    skip_ws_no_nl();
    while (globl[current] == '/' && globl[current + 1] == '/') {
        current += 2;
        skip_rest_of_line();
        skip_ws_no_nl();
    }
}

void skip_whitespace() {
    skip_ws();
    while (globl[current] == '/' && globl[current + 1] == '/') {
        current += 2;
        skip_rest_of_line();
        skip_ws();
    }
}

void skip_untilws() {
    while (globl[current] && !isws(globl[current])) {
        current++;
    }
}

void add_sym(char *name, int i) {
    symbols[num_syms].name = name;
    symbols[num_syms].instr_num = i;
    num_syms++;
}

void add_label(char *name, int i) {
    labels[num_labels].name = name;
    labels[num_labels].instr_num = i;
    num_labels++;
}

char *cpy_str(char *orig, int len) {
    
    char *str = malloc(len + 1);
    if (!str) {
        printf("out of mem");
        exit(1);
    }
    memcpy(str, orig, len);
    str[len] = '\0';
    return str;
}

void parse_ainstr() {
    if (globl[current] != '@') {
        return;
    }
    current++;
    if (is_alpha(globl[current])) {
        int start = current;
        skip_untilws();
        int len = current - start;
        char *str = cpy_str(globl + start, len);
        add_sym(str, num_instrs);
        instrs[num_instrs] = 0; // patch up later
        num_instrs++;
    } else {
        
        int val = 0;

        while (is_num(globl[current])) {
            val = val * 10 + globl[current] - '0';
            current++;
        }

        if (val > (1 << 15) - 1) {
            printf("value bigger than max a-instr val\n");
            exit(1);
        }

        instrs[num_instrs] = val;
        num_instrs++;
    }
}

void parse_label() {
    if (globl[current] != '(') {
        printf("fuck you");
        exit(1);
    }
    current++;
    int start = current;
    while (globl[current] && globl[current] != ')') {
        current++;
    }
    if (globl[current] != ')') {
        printf("label does not end with ')'");
        exit(1);
    }
    current++;
    int len = current - start - 1;
    char *str = cpy_str(globl + start, len);
    int idx = find_label(str);
    if (idx != -1) {
        printf("label \"%s\" already defined\n", str);
        exit(1);
    }
    add_label(str, num_instrs);
}

void parse_cinstr() {
    // upper three bits
    unsigned short finished_instr = (1 << 15) | (1 << 14) | (1 << 13);
    int eq = current;
    while (globl[eq] && globl[eq] != '\n' && globl[eq] != '=') {
        eq++;
    }
    if (globl[eq] == '=') {
        // optional assignment
        if (globl[current] == 'A') {
            finished_instr |= (1 << 5);
            current++;
        }
        if (globl[current] == 'M') {
            finished_instr |= (1 << 3);
            current++;
        }
        if (globl[current] == 'D') {
            finished_instr |= (1 << 4);
            current++;
        }
        current = eq + 1;
    }
    skip_whitespace();
    int semic = current;
    while (globl[semic] && globl[semic] != '\n' && globl[semic] != ';') {
        semic++;
    }
    int ba = 1 << 12;
    int bc1 = 1 << 11; 
    int bc2 = 1 << 10; 
    int bc3 = 1 << 9; 
    int bc4 = 1 << 8; 
    int bc5 = 1 << 7; 
    int bc6 = 1 << 6; 
    // non optional expr
    if (globl[current] == '0') {
        finished_instr |= bc1;
        finished_instr |= bc3;
        finished_instr |= bc5;
        current++;
    } else if (globl[current] == '1') {
        finished_instr |= bc1;
        finished_instr |= bc2;
        finished_instr |= bc3;
        finished_instr |= bc4;
        finished_instr |= bc5;
        finished_instr |= bc6;
        current++;
    } else if (globl[current] == '-') {
        current++;
        if (globl[current] == '1') {
            finished_instr |= 1 << 11;
            finished_instr |= 1 << 10;
            finished_instr |= 1 << 9;
            finished_instr |= 1 << 7;
        } else if (globl[current] == 'D') {
            finished_instr |= 1 << 9;
            finished_instr |= 1 << 8;
            finished_instr |= 1 << 7;
            finished_instr |= 1 << 6;
        } else if (globl[current] == 'A') {
            finished_instr |= 1 << 11;
            finished_instr |= 1 << 10;
            finished_instr |= 1 << 7;
            finished_instr |= 1 << 6;
        } else if (globl[current] == 'M') {
            finished_instr |= 1 << 12;
            finished_instr |= 1 << 11;
            finished_instr |= 1 << 10;
            finished_instr |= 1 << 7;
            finished_instr |= 1 << 6;
        }
        current++;
    } else if (globl[current] == '!') {
        current++;
        if (globl[current] == 'D') {
            finished_instr |= 1 << 9;
            finished_instr |= 1 << 8;
            finished_instr |= 1 << 6;
        } else if (globl[current] == 'A') {
            finished_instr |= 1 << 11;
            finished_instr |= 1 << 10;
            finished_instr |= 1 << 6;
        } else if (globl[current] == 'M') {
            finished_instr |= 1 << 12;
            finished_instr |= 1 << 11;
            finished_instr |= 1 << 10;
            finished_instr |= 1 << 6;
        }
        current++;
    } else if (globl[current] == 'D') {
        current++;
        if (globl[current] == '+') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= 1 << 10;
                finished_instr |= 1 << 9;
                finished_instr |= 1 << 8;
                finished_instr |= 1 << 7;
                finished_instr |= 1 << 6;
            } else if (globl[current] == 'A') {
                finished_instr |= 1 << 7;
            } else if (globl[current] == 'M') {
                finished_instr |= 1 << 12;
                finished_instr |= 1 << 7;
            }
            current++;
        } else if (globl[current] == '-') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= bc3 | bc4 | bc5;
            } else if (globl[current] == 'A') { // D-A
                finished_instr |= bc2 | bc5 | bc6;
            } else if (globl[current] == 'M') { // D-M
                finished_instr |= ba | bc2 | bc5 | bc6;
            }
            current++;
        } else if (globl[current] == '&') {
            current++;
            if (globl[current] == 'A') { // D&A
                finished_instr |= 0;
            } else if (globl[current] == 'M') { // D&M
                finished_instr |= ba;
            }
            current++;
        } else if (globl[current] == '|') {
            current++;
            if (globl[current] == 'A') {
                finished_instr |= bc2 | bc4 | bc6;
            } else if (globl[current] == 'M') {
                finished_instr |= ba | bc2 | bc4 | bc6;
            }
            current++;
        } else {
            finished_instr |= bc3 | bc4;
        }
    } else if (globl[current] == 'A') {
        current++;
        if (globl[current] == '+') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= bc1 | bc2 | bc4 | bc5 | bc6;
            }
            current++;
        } else if (globl[current] == '-') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= bc1 | bc2 | bc5;
            } else if (globl[current] == 'D') {
                finished_instr |= bc4 | bc5 | bc6;
            }
            current++;
        }  else {
            finished_instr |= bc1 | bc2;
        }
    } else if (globl[current] == 'M') {
        current++;
        if (globl[current] == '+') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= ba | bc1 | bc2 | bc4 | bc5 | bc6;
            }
            current++;
        } else if (globl[current] == '-') {
            current++;
            if (globl[current] == '1') {
                finished_instr |= ba | bc1 | bc2 | bc5;
            } else if (globl[current] == 'D') {
                finished_instr |= ba | bc4 | bc5 | bc6;
            }
            current++;
        } else {
            finished_instr |= ba | bc1 | bc2;
        }
    }
    if (globl[current] == ';') {
        current++;
        int bgt = 1 << 0;
        int beq = 1 << 1;
        int blt = 1 << 2;
        if (strncmp(globl + current, "JGT", 3) == 0) {
            finished_instr |= bgt;
        } else if (strncmp(globl + current, "JEQ", 3) == 0) {
            finished_instr |= beq;
        } else if (strncmp(globl + current, "JGE", 3) == 0) {
            finished_instr |= beq | bgt;
        } else if (strncmp(globl + current, "JLT", 3) == 0) {
            finished_instr |= blt;
        } else if (strncmp(globl + current, "JNE", 3) == 0) {
            finished_instr |= blt | bgt;
        } else if (strncmp(globl + current, "JLE", 3) == 0) {
            finished_instr |= blt | beq;
        } else if (strncmp(globl + current, "JMP", 3) == 0) {
            finished_instr |= blt | beq | bgt;
        } else {
            printf("invalid jmp");
            exit(1);
        }
        current += 3;
    }
    if (num_instrs)
    instrs[num_instrs] = finished_instr;
    num_instrs++;
}

void parse() {
    skip_whitespace();
    while (globl[current]) {
        if (globl[current] == '@') {
            parse_ainstr();
        } else if (globl[current] == '(') {
            parse_label();
        } else if (is_alnum(globl[current])) {
            parse_cinstr();
        } else {
            printf("unrecognized instr char '%c''%d' at '%d'\n", globl[current], globl[current], current);
            exit(1);
        }
        skip_whitespace_no_nl();
        if (globl[current] != '\n') {
            printf("instr must end with a newline\n");
            exit(1);
        }
        current++;
        skip_whitespace();
    }
}

void add_r_syms() {
    add_label("R0", 0);
    add_label("R1", 1);
    add_label("R2", 2);
    add_label("R3", 3);
    add_label("R4", 4);
    add_label("R5", 5);
    add_label("R6", 6);
    add_label("R7", 7);
    add_label("R8", 8);
    add_label("R9", 9);
    add_label("R10", 10);
    add_label("R11", 11);
    add_label("R12", 12);
    add_label("R13", 13);
    add_label("R14", 14);
    add_label("R15", 15);

    add_label("SCREEN", 16384);
    add_label("KBD", 24576);
    add_label("SP", 0);
    add_label("LCL", 1);
    add_label("ARG", 2);
    add_label("THIS", 3);
    add_label("THAT", 4);
}


void print_instrs(FILE *f) {
    for (int i = 0; i < num_instrs; i++) {
        for (int j = 8 * sizeof(short) - 1; j >= 0; j--) {
            fprintf(f, "%d", (instrs[i] >> j) & 1);
        }
        fprintf(f, "\n");
    }
}

char *load_file(char *fname) {
    char *buffer = 0;
    long length;
    FILE *f = fopen(fname, "rb");
    if (!f) {
        printf("err no file \"%s\"\n", fname);
        exit(1);
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer = malloc(length + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }
    fread(buffer, 1, length, f);
    buffer[length] = '\0';
    fclose(f);
    return buffer;
}

void patch_labels() {
    for (int i = 0; i < num_syms; i++) {

        int idx = find_label(symbols[i].name);
        if (idx == -1) {
            add_label(symbols[i].name, new_var_a);
            instrs[symbols[i].instr_num] = new_var_a;
            new_var_a++;
        } else {
            instrs[symbols[i].instr_num] = labels[idx].instr_num;
        }
    }
}

void main(int argc, char **argv) {
    //if (argc != 2) {
    //    printf("Usage: %s <filename>\n", *argv);
    //    exit(1);
    //}
    // char *fn = "C:\\Users\\nick8186\\Desktop\\nand2tetris\\projects\\06\\rect\\Rect.asm";
    char *fn = argv[1];
      globl = load_file(fn);
    add_r_syms();

    int filename_len = strlen(fn);
    char *finished_file = cpy_str(fn, filename_len + 6);
    finished_file[filename_len] = '.';
    finished_file[filename_len+1] = 'h';
    finished_file[filename_len+2] = 'a';
    finished_file[filename_len+3] = 'c';
    finished_file[filename_len+4] = 'k';
    finished_file[filename_len+5] = '\0';
    parse();
    patch_labels();
    FILE *f = fopen(finished_file, "w+");
    print_instrs(f);
}