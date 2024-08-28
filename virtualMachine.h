#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

/*Declaration des structures*/
typedef struct {
    int PC;
    short registers[32];
    unsigned int bitC;
    unsigned int bitZ;
    unsigned int bitN;
} CPU;

typedef struct {
    int codeOp;
    int dest;
    int src1;
    int imm;
    int src2;
} Instruction;

// Tries to execute the hexadecimal code containted in the file fileName
int exec(char* fileName);

// Initialization of memory
CPU *initCPU(void);
Instruction *initInstruction(void);
Instruction *customInstruction(int codeOp, int dest, int src1, int imm, int src2);

// load all the file memory into the memory array
void loadMemory(char *file, unsigned char memory[]);

// read the next instruction from the memory
Instruction *readMemory(CPU *cpu, unsigned char memory[]);

// Utility functions
int updateBitC(int value);
int updateBitZ(int value);
int updateBitN(int value);

// Selects the right instruction to execute and execute it
void instruction_selector(CPU *cpu, Instruction *instruc, unsigned char mem[]);

// All instructions possible
void op_add(CPU *cpu, Instruction *instruc);
void op_sub(CPU *cpu, Instruction *instruc);
void op_mult(CPU *cpu, Instruction *instruc);
void op_div(CPU *cpu, Instruction *instruc);

void op_and(CPU *cpu, Instruction *instruc);
void op_or(CPU *cpu, Instruction *instruc);
void op_xor(CPU *cpu, Instruction *instruc);
void op_shl(CPU *cpu, Instruction *instruc);

void ldb(CPU *cpu, Instruction *instruc, unsigned char mem[]);
void ldw(CPU *cpu, Instruction *instruc, unsigned char mem[]);

void stb(CPU *cpu, Instruction *instruc, unsigned char mem[]);
void stw(CPU *cpu, Instruction *instruc, unsigned char mem[]);

void jmp(CPU *cpu, Instruction *instruc);
void jzs(CPU *cpu, Instruction *instruc);
void jzc(CPU *cpu, Instruction *instruc);
void jcs(CPU *cpu, Instruction *instruc);
void jcc(CPU *cpu, Instruction *instruc);
void jns(CPU *cpu, Instruction *instruc);
void jnc(CPU *cpu, Instruction *instruc);

void in(CPU *cpu, Instruction *instruc);
void out(CPU *cpu, Instruction *instruc);

void rnd(CPU *cpu, Instruction *instruc);

void hlt(CPU *cpu, Instruction *instruc);

/* DEBUG functions */
void printRegister(int tab[], int number);
void printInstruction(Instruction *instruc);
void printList(int tab[], int len);

#endif // VIRTUALMACHINE_H
