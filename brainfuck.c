#include <stdio.h>

#define OP_END          0
#define INC_POS      1
#define DEC_POS      2
#define INC_VAL      3
#define DEC_VAL      4
#define OP_OUT          5
#define OP_IN           6
#define JMP_FWD      7
#define JMP_BCK      8

#define SUCCESS         0
#define FAILURE         1

#define PROGRAM_SIZE    4096
#define STACK_SIZE      512
#define DATA_SIZE       65535

#define STACK_PUSH(A)   (STACK[SP++] = A)
#define STACK_POP()     (STACK[--SP])
#define STACK_EMPTY()   (SP == 0)
#define STACK_FULL()    (SP == STACK_SIZE)

struct instruction_t {
    unsigned short operator;
    unsigned short operand;
};

static struct instruction_t PROGRAM[PROGRAM_SIZE];
static unsigned short STACK[STACK_SIZE];
static unsigned int SP = 0;

int compile_bf(FILE* fp) {
    unsigned short pc = 0, jmp_pc;
    int c;
    while ((c = getc(fp)) != EOF && pc < PROGRAM_SIZE) {
        switch (c) {
            case '>': PROGRAM[pc].operator = INC_POS; break;
            case '<': PROGRAM[pc].operator = DEC_POS; break;
            case '+': PROGRAM[pc].operator = INC_VAL; break;
            case '-': PROGRAM[pc].operator = DEC_VAL; break;
            case '.': PROGRAM[pc].operator = OP_OUT; break;
            case ',': PROGRAM[pc].operator = OP_IN; break;
            case '[':
                PROGRAM[pc].operator = JMP_FWD;
                if (STACK_FULL()) {
                    return FAILURE;
                }
                STACK_PUSH(pc);
                break;
            case ']':
                if (STACK_EMPTY()) {
                    return FAILURE;
                }
                jmp_pc = STACK_POP();
                PROGRAM[pc].operator =  JMP_BCK;
                PROGRAM[pc].operand = jmp_pc;
                PROGRAM[jmp_pc].operand = pc;
                break;
            default: pc--; break;
        }
        pc++;
    }
    if (!STACK_EMPTY() || pc == PROGRAM_SIZE) {
        return FAILURE;
    }
    PROGRAM[pc].operator = OP_END;
    return SUCCESS;
}

int execute_bf() {
    unsigned short data[DATA_SIZE], pc = 0;
    unsigned int ptr = DATA_SIZE;
    while (--ptr) { data[ptr] = 0; }
    while (PROGRAM[pc].operator != OP_END && ptr < DATA_SIZE) {
        switch (PROGRAM[pc].operator) {
            case INC_POS: ptr++; break;
            case DEC_POS: ptr--; break;
            case INC_VAL: data[ptr]++; break;
            case DEC_VAL: data[ptr]--; break;
            case OP_OUT: putchar(data[ptr]); break;
            case OP_IN: data[ptr] = (unsigned int)getchar(); break;
            case JMP_FWD: if(!data[ptr]) { pc = PROGRAM[pc].operand; } break;
            case JMP_BCK: if(data[ptr]) { pc = PROGRAM[pc].operand; } break;
            default: return FAILURE;
        }
        pc++;
    }
    return ptr != DATA_SIZE ? SUCCESS : FAILURE;
}

int main(int argc, const char * argv[])
{
    int status;
    FILE *fp;
    if (argc != 2 || (fp = fopen(argv[1], "r")) == NULL) {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        return FAILURE;
    }
    status = compile_bf(fp);
    fclose(fp);
    if (status == SUCCESS) {
        status = execute_bf();
    }
    if (status == FAILURE) {
        fprintf(stderr, "Error!\n");
    }
    return status;
}
