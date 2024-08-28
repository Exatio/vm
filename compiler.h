#ifndef COMPILER_H
#define COMPILER_H

#define MAX_LINE_LENGTH 100
#define INSTRUCTION_NUMBER 32
#define MAX_TAG_AMOUNT 50
#define MAX_TAG_LENGTH 30

typedef enum {
    TOKEN_NO_TOKEN,
    TOKEN_NOT_A_TOKEN,
    TOKEN_TOI,          // Wrapper for Tag or Instruction
    TOKEN_TAG,
    TOKEN_INSTRUCTION,
    TOKEN_ARG_S,        // Wrapper for Register or Sharp
    TOKEN_ARG_REGISTER,
    TOKEN_ARG_SHARP,
    TOKEN_ARG_ADDRESS,
    TOKEN_ARG_TAG
} TOK_TYPE;

typedef enum {
    MEM_OP,
    MEM_DEST,
    MEM_SRC_1,
    MEM_IMM,
    MEM_SRC_2
}  TOK_MEMORY_TYPE;

typedef struct {
    TOK_MEMORY_TYPE mem;
    TOK_TYPE type;
} EXPECTATION;

/* Represents an instruction, with its name and the arguments it expectes to find after it */
typedef struct {
    char* str;
    EXPECTATION expectations[3];
} INSTRUCTION;

/* Represents an instruction line in binary strings */
typedef struct {
    char code[6]; // Ex : "00000"
    char dest[6];
    char src1[6];
    char imm[2];
    char src2[17];
} LINE_BIN;

typedef struct TAG TAG;
struct TAG {
    char str[MAX_TAG_LENGTH];
    int addr;
};

int compile(char* fileName, char* hexaFileName);
int processLine(char* hex, char* line, int lineNum, INSTRUCTION instructions[], TAG tagList[], int tagAmount);

int getInstructionIndex(char* token, INSTRUCTION instructions[]);

char* tokTypeToStr(TOK_TYPE type);
int isTokTypeArgument(TOK_TYPE type);

TOK_TYPE getTokTypeArgument(char* token, int tokenLength, TAG tagList[], int tagAmount);

void error(int lineNum, char* error);
void expectationError(int line, char* tok, char* expect, char* current);

LINE_BIN* initLine();
void integerToBinaryString(char* buff, int toConvert, int bits);

#endif // COMPILER_H
