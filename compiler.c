#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"


int compile(char* fileName, char* hexaFileName)
{
    INSTRUCTION instructions[INSTRUCTION_NUMBER] = {

        {"add", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"sub", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"mul", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"div", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},

        {"and", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"or" , { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"xor", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},
        {"shl", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},

        {"",{}}, // These "empty" instructions are here so the indexes matches the codes on the project instruction pdf
        {"",{}},

        {"ldb", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_ADDRESS} }},
        {"ldw", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_ADDRESS} }},

        {"stb", { {MEM_DEST, TOKEN_ARG_ADDRESS}, {MEM_SRC_1, TOKEN_ARG_REGISTER} }},
        {"stw", { {MEM_DEST, TOKEN_ARG_ADDRESS}, {MEM_SRC_1, TOKEN_ARG_REGISTER} }},

        {"",{}},
        {"",{}},
        {"",{}},
        {"",{}},
        {"",{}},
        {"",{}},

        {"jmp", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jzs", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jzc", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jcs", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jcc", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jns", {{MEM_SRC_2, TOKEN_ARG_S}}},
        {"jnc", {{MEM_SRC_2, TOKEN_ARG_S}}},

        {"in" , {{MEM_DEST, TOKEN_ARG_REGISTER}}},
        {"out", {{MEM_DEST, TOKEN_ARG_REGISTER}}},

        {"rnd", { {MEM_DEST, TOKEN_ARG_REGISTER}, {MEM_SRC_1, TOKEN_ARG_REGISTER}, {MEM_SRC_2, TOKEN_ARG_S} }},

        {"",{}},

        {"hlt", {}}
    };

    FILE *sourceFile;
    FILE *hexFile;
    sourceFile = fopen(fileName, "r");
    if(sourceFile == NULL)
    {
        printf("Impossible d'ouvrir %s.\n", fileName);
        printf("Veuillez vérifier son emplacement.\n");
        return 2;
    }

    hexFile = fopen(hexaFileName, "w");
    if(hexFile == NULL)
    {
        printf("Impossible de créer le fichier %s.\n", hexaFileName);
        return 3;
    }

    char line[MAX_LINE_LENGTH + 1];
    char hexLine[MAX_LINE_LENGTH + 1];
    int lineNum = 0;

    /* Generate tag list */
    int tagAmount = 0;
    TAG tagList[MAX_TAG_AMOUNT];
    while(fgets(line, sizeof line, sourceFile) != NULL)
    {
        char toTokenize[MAX_LINE_LENGTH + 1];
        strcpy(toTokenize, line);
        char *firstToken = strtok(line, " \t\n");

        if(firstToken != NULL && strlen(firstToken) > 1 && firstToken[strlen(firstToken) - 1] == ':') // A tag
        {
            TAG tag;
            strncpy(tag.str, firstToken, strlen(firstToken) - 1);
            tag.str[strlen(firstToken) - 1] = '\0';
            tag.addr = 4 * lineNum;

            for(int i = 0 ; i < tagAmount ; i++)
            {
                if(strcmp(tag.str, tagList[i].str) == 0)
                {
                    error(lineNum+1, "Tag already exists");
                    return 1;
                }
            }

            tagList[tagAmount++] = tag;
        }

        lineNum++;
    }

    // Places the cursor at the start of the file again
    rewind(sourceFile);

    lineNum = 1;
    while(fgets(line, sizeof line, sourceFile) != NULL)
    {
        hexLine[0] = '\0';
        if(processLine(hexLine, line, lineNum++, instructions, tagList, tagAmount)) { // An error occured
            fclose(sourceFile);
            fclose(hexFile);
            return 1;
        }
        fprintf(hexFile, "%s\n", hexLine);
    }

    fclose(sourceFile);
    fclose(hexFile);
    return 0;
}

/* Returns -1 if the token is not an instruction, otherwise the index in the instructions array */
int getInstructionIndex(char* token, INSTRUCTION instructions[])
{
    for(int i = 0 ; i < INSTRUCTION_NUMBER; i++)
    {
        if(strcmp(instructions[i].str, token) == 0) return i;
    }
    return -1;
}

// Returns 1 if an error occured, 0 else
int processLine(char* hex, char* line, int lineNum, INSTRUCTION instructions[], TAG tagList[], int tagAmount)
{
    TOK_TYPE nextTokenExpectations[7] = { TOKEN_TOI /* tag or instr first */ };
    TOK_TYPE currentExpectation;
    TOK_TYPE currentType;

    INSTRUCTION currentInstruction;
    int isSetInstruction = 0;
    int currentExpectationAfterInstruction = -1;

    LINE_BIN* lineBin = initLine();

    char toTokenize[MAX_LINE_LENGTH + 1];
    strcpy(toTokenize, line);
    char *token = strtok(toTokenize, " \t\n");
    if(token == NULL) return 0;
    while(token != NULL)
    {
        currentExpectation = nextTokenExpectations[0];
        int tokenLength = strlen(token);
        // DEBUG printf("TOKEN : %s\n", token);
        currentType = TOKEN_NOT_A_TOKEN;
        int instructionIndex = getInstructionIndex(token, instructions); // is -1 if the token is not an instruction

        if(token[tokenLength - 1] == ':') // A tag
        {
            currentType = TOKEN_TAG;
            nextTokenExpectations[0] = TOKEN_INSTRUCTION;
            if(tokenLength == 1) // The tag is empty
            {
                error(lineNum, "A tag cannot have no name !");
                return 1;
            }
        }
        else if(instructionIndex != -1)
        {
            currentInstruction = instructions[instructionIndex];
            isSetInstruction = 1;

            currentType = TOKEN_INSTRUCTION;
            for(int i = 0 ; i < 3; i++)
            {
                nextTokenExpectations[i] = currentInstruction.expectations[i].type;
            }

        }
        else if(token[tokenLength - 1] == ',') // Arg ?
        {
            char temp[tokenLength];
            strncpy(temp, token, tokenLength - 1);
            temp[tokenLength - 1] = '\0';
            currentType = getTokTypeArgument(temp, tokenLength - 1, tagList, tagAmount);

            nextTokenExpectations[0] = nextTokenExpectations[1];
            nextTokenExpectations[1] = nextTokenExpectations[2];
            nextTokenExpectations[2] = 0;

            currentExpectationAfterInstruction++;

            if(!isTokTypeArgument(nextTokenExpectations[0]))
            {
                expectationError(lineNum, token, "argument", tokTypeToStr(currentType));
                return 1;
            }
        }
        else
        {
            currentType = getTokTypeArgument(token, tokenLength, tagList, tagAmount);

            currentExpectationAfterInstruction++;

            nextTokenExpectations[0] = nextTokenExpectations[1];
            nextTokenExpectations[1] = nextTokenExpectations[2];
            nextTokenExpectations[2] = 0;
        }

        if(currentExpectation != currentType)
        {
            // Quick workaround to be able to expect two things instead of one
            if(
                (currentExpectation == TOKEN_ARG_S && currentType == TOKEN_ARG_TAG && isSetInstruction && currentInstruction.expectations[0].type == TOKEN_ARG_S)
                || (currentExpectation == TOKEN_TOI && (currentType == TOKEN_INSTRUCTION || currentType == TOKEN_TAG))
                || (currentExpectation == TOKEN_ARG_S && (currentType == TOKEN_ARG_SHARP || currentType == TOKEN_ARG_REGISTER))
            ) {
                // Everything is OK
            }
            else {
                printf("%d\n", instructionIndex);
                expectationError(lineNum, token, tokTypeToStr(currentExpectation), tokTypeToStr(currentType));
                return 1;
            }
        }

        // The token is good so we translate it

        if(currentType == TOKEN_INSTRUCTION) {

            integerToBinaryString(lineBin->code, instructionIndex, 5);

        } else {

            char* buff;

            TOK_MEMORY_TYPE memType = currentInstruction.expectations[currentExpectationAfterInstruction].mem;
            int bits = 0;
            switch (memType) {
                case MEM_DEST:
                    buff = lineBin->dest;
                    bits = 5;
                    break;
                case MEM_SRC_1:
                    buff = lineBin->src1;
                    bits = 5;
                    break;
                case MEM_SRC_2:
                    buff = lineBin->src2;
                    bits = 16;
                    break;
                default:
                    break; // Shouldn't happen
            }

            switch (currentType) {
                case TOKEN_ARG_REGISTER:
                    integerToBinaryString(buff, (int) strtol(token + 1, NULL, 10), bits);
                    break;
                case TOKEN_ARG_SHARP:
                    if(token[1] == 'h') {
                        // supposedly already given as a two's complement (that means we cant have #h-A000 for exemple)
                        integerToBinaryString(buff, (int) strtol(token + 2, NULL, 16), bits);
                    } else {
                        // May be with a minus sign, case treated in the integerToBinaryString function
                        integerToBinaryString(buff, (int) strtol(token + 1, NULL, 10), bits);
                    }

                    strcpy(lineBin->imm, "1");
                    break;
                case TOKEN_ARG_ADDRESS: {

                    // An address is composed of a register and eventually of a S
                    int closeIndex = 0;
                    for(int i = 1; i < tokenLength ; i++) {
                        if(token[i] == ')') {
                            closeIndex = i;
                            break;
                        }
                    }

                    int innerLength = closeIndex - 1;
                    char innerToken[innerLength + 1];
                    strncpy(innerToken, token + 1, innerLength);
                    innerToken[innerLength] = '\0';

                    integerToBinaryString(buff, (int) strtol(innerToken + 1, NULL, 10), bits);

                    if(closeIndex != tokenLength - 1) { // There is a S

                        int outerLength = tokenLength - closeIndex - 1;
                        if(token[tokenLength - 1] == ',') outerLength--; // If there is a comma at the end of the token, we don't count it
                        char outerToken[outerLength + 1];
                        strncpy(outerToken, token + closeIndex + 1, outerLength);
                        outerToken[outerLength] = '\0';

                        TOK_TYPE outer_type = getTokTypeArgument(outerToken, outerLength, tagList, tagAmount);

                        if(outer_type == TOKEN_ARG_SHARP) {
                            strcpy(lineBin->imm, "1");

                            if(outerToken[1] == 'h') {
                                // supposedly already given as a two's complement (that means we cant have #h-A000 for exemple)
                                integerToBinaryString(lineBin->src2, (int) strtol(outerToken + 2, NULL, 16), 16);
                            } else {
                                // May be with a minus sign, case treated in the integerToBinaryString function
                                integerToBinaryString(lineBin->src2, (int) strtol(outerToken + 1, NULL, 10), 16);
                            }

                        } else { // outer_type is TOKEN_ARG_REGISTER
                            integerToBinaryString(lineBin->src2, (int) strtol(outerToken + 1, NULL, 10), 16);
                        }
                    }

                    break;
                }
                case TOKEN_ARG_TAG:
                    for(int i = 0 ; i < tagAmount ; i++)
                    {
                        if(strcmp(token, tagList[i].str) == 0)
                        {
                            integerToBinaryString(buff, tagList[i].addr, 16);
                            if(memType == MEM_SRC_2) { // Probably always the case but we might change that if we add new instructions
                                strcpy(lineBin->imm, "1");
                            }
                            break;
                        }
                    }
                    break;
                default:
                    break;
            }
        }

        token = strtok(NULL, " \t\n");
    }

    if(nextTokenExpectations[0] != TOKEN_NO_TOKEN)
    {
        expectationError(lineNum, token, tokTypeToStr(nextTokenExpectations[0]), "NOTHING");
        return 1;
    }


    char line_bin[33];
    snprintf(line_bin, 33, "%s%s%s%s%s", lineBin->code, lineBin->dest, lineBin->src1, lineBin->imm, lineBin->src2);
    sprintf(hex, "%08lX", strtol(line_bin, NULL, 2));

    return 0;
}

// Takes an integer and returns a string with its binary representation
void integerToBinaryString(char* buff, int toConvert, int bits) {

    if (toConvert < 0) {
        // If the number is negative, set the sign bit and convert its absolute value
        // Had to search online for this one because i wasnt sure how the %b formatter worked
        toConvert = (1 << (bits - 1)) | (toConvert & ((1 << (bits - 1)) - 1));
    }

    char bitsStr[3];
    sprintf(bitsStr, "%d", bits);

    char format[6] = "\0";
    strcat(format, "%0");
    strcat(format, bitsStr);
    strcat(format, "b");
    sprintf(buff, format, toConvert);
}

LINE_BIN* initLine() {
    LINE_BIN* line = (LINE_BIN*) malloc(sizeof(LINE_BIN));
    integerToBinaryString(line->code, 0, 5);
    integerToBinaryString(line->dest, 0, 5);
    integerToBinaryString(line->src1, 0, 5);
    integerToBinaryString(line->imm, 0, 1);
    integerToBinaryString(line->src2, 0, 16);
    return line;
}

void error(int lineNum, char* error)
{

    printf("\033[31mERROR DETECTED\n");
    printf("\033[0mAt: Line %d\n", lineNum);
    /*
    printf("ERROR DETECTED\n");
    printf("At: Line %d\n", lineNum);
    */
    printf("%s\n", error);
}

void expectationError(int line, char* tok, char* expect, char* current)
{
    error(line, "Token error");
    printf("Expected %s but instead got %s (token : \"%s\")\n", expect, current, tok);
}

char* tokTypeToStr(TOK_TYPE type)
{
    return
        type == TOKEN_TOI ? "tag or instruction" :
        type == TOKEN_TAG ? "tag" :
        type == TOKEN_INSTRUCTION ? "instruction" :
        type == TOKEN_ARG_S ? "rd or #n" :
        type == TOKEN_ARG_REGISTER ? "rd" :
        type == TOKEN_ARG_ADDRESS ? "(rn)S" :
        type == TOKEN_ARG_SHARP ? "#n" :
        type == TOKEN_ARG_TAG ? "tag" :
        type == TOKEN_NO_TOKEN ? "nothing" :
        type == TOKEN_NOT_A_TOKEN ? "unkown argument" :
        "NOT DEFINED";
}

int isTokTypeArgument(TOK_TYPE type)
{
    return
        type == TOKEN_ARG_ADDRESS  ||
        type == TOKEN_ARG_REGISTER ||
        type == TOKEN_ARG_S        ||
        type == TOKEN_ARG_TAG      ;
}

TOK_TYPE getTokTypeArgument(char* token, int tokenLength, TAG tagList[], int tagAmount)
{
    // DEBUG printf("Analyzing the token as argument : %s of length %d\n", token, tokenLength);
    if(token[0] == '#')
    {
        TOK_TYPE type = TOKEN_ARG_SHARP;

        if(token[1] == '-') {
            for(int i = 2 ; i < tokenLength ; ++i)
                if(token[i] < '0' || token[i] > '9') type = TOKEN_NOT_A_TOKEN;
            if (tokenLength > 7) type = TOKEN_NOT_A_TOKEN;
            if (strtol(token + 1, NULL, 10) > 32768) type = TOKEN_NOT_A_TOKEN;
        } else if(token[1] == 'h') {
            for(int i = 2 ; i < tokenLength ; ++i)
                if((token[i] < '0' || token[i] > '9') && (token[i] < 'A' || token[i] > 'F') && (token[i] < 'a' || token[i] > 'f')) type = TOKEN_NOT_A_TOKEN;

            if (tokenLength > 6) type = TOKEN_NOT_A_TOKEN;
        } else {
            for(int i = 1 ; i < tokenLength ; ++i)
                if(token[i] < '0' || token[i] > '9') type = TOKEN_NOT_A_TOKEN;
            if (tokenLength > 6) type = TOKEN_NOT_A_TOKEN;
            if (strtol(token + 1, NULL, 10) > 32767) type = TOKEN_NOT_A_TOKEN;
        }


        return type;
    }

    if(token[0] == 'r')
    {
        if(tokenLength > 3) return TOKEN_NOT_A_TOKEN;
        else {
            if(tokenLength == 2 && (token[1] < '0' || token[1] > '9')) return TOKEN_NOT_A_TOKEN;
            if(tokenLength == 3 && (token[1] < '1' || token[1] > '3')) return TOKEN_NOT_A_TOKEN;
            if(tokenLength == 3 && (token[2] < '0' || token[2] > '9')) return TOKEN_NOT_A_TOKEN;
            if(tokenLength == 3 && token[1] == '3' && token[2] > '1') return TOKEN_NOT_A_TOKEN;
        }

        return TOKEN_ARG_REGISTER;
    }

    if(token[0] == '(')
    {
        int closeIndex = -1;
        for(int i = 1; i < tokenLength ; i++) {
            if(token[i] == ')') {
                closeIndex = i;
                break;
            }
        }

        if(closeIndex != -1) {

            int innerLength = closeIndex - 1;
            char innerToken[innerLength + 1];
            strncpy(innerToken, token + 1, innerLength);
            innerToken[innerLength] = '\0';

            int isRegister = (getTokTypeArgument(innerToken, innerLength, tagList, tagAmount) == TOKEN_ARG_REGISTER);
            if(closeIndex == tokenLength - 1) {
                return isRegister ? TOKEN_ARG_ADDRESS : TOKEN_NOT_A_TOKEN;
            } else {
                TOK_TYPE outer_type = getTokTypeArgument(token + closeIndex + 1, tokenLength - closeIndex - 1, tagList, tagAmount);
                int isS = (outer_type == TOKEN_ARG_SHARP) || (outer_type == TOKEN_ARG_REGISTER);
                return (isRegister && isS) ? TOKEN_ARG_ADDRESS : TOKEN_NOT_A_TOKEN;
            }

        }

        return TOKEN_NOT_A_TOKEN;

    }

    for(int i = 0 ; i < tagAmount ; i++)
    {
        if(strcmp(token, tagList[i].str) == 0) return TOKEN_ARG_TAG;
    }

    return TOKEN_NOT_A_TOKEN;
}
