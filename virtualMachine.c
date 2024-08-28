#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "virtualMachine.h"
#include <string.h>

#define HEX_LEN 8

/*Fonctions d'initialisation de structure*/
CPU *initCPU(void) {
    CPU *cpu = malloc(sizeof(CPU));
    if (cpu == NULL){
        printf("Error malloc in initCPU\n");
        exit(-1);
    }
    cpu->PC = 0;
    cpu->bitC = 0;
    cpu->bitN = 0;
    cpu->bitZ = 0;
    for (int i=0; i<32; i++){
        cpu->registers[i] = 0;
    }
    return cpu;
}

Instruction *initInstruction(void){
    Instruction *instruc = malloc(sizeof(Instruction));
    if (instruc == NULL){
        printf("Error malloc in initInstruction\n");
        exit(-1);
    }
    instruc->codeOp = -1;
    instruc->dest = -1;
    instruc->imm = -1;
    instruc->src1 = -1;
    instruc->src2 = -1;
    return instruc;
}

Instruction *customInstruction(int codeOp, int dest, int src1, int imm, int src2){
    Instruction *instruc = malloc(sizeof(Instruction));
    if (instruc == NULL){
        printf("Error malloc in customInstruction\n");
        exit(-1);
    }
    instruc->codeOp = codeOp;
    instruc->dest = dest;
    instruc->src1 = src1;
    instruc->imm = imm;
    instruc->src2 = src2;
    return instruc;

}

/*Fonction d'affichage*/
void printRegister(int tab[], int number) {
    printf("Register R%d : ", number);
    for (int i = 0; i < 16; i++) {
        printf("%d ", tab[i]);
    }
    printf("\n");
}

/* DEBUG */
void printInstruction(Instruction *instruc){
    printf("codeOp = %d dest= %d src1=%d imm=%d src2=%d\n", instruc->codeOp, instruc->dest, instruc->src1, instruc->imm, instruc->src2);
}

void printList(int tab[], int len) {
    printf("List content : ");
    for (int i = 0; i < len; i++) {
        printf("%d ", tab[i]);
    }
    printf("\n");
}


/*Fonction de simulation*/
/*Charge un programme par la lecture d'un fichier dans la memoire*/
void loadMemory(char *fileName, unsigned char memory[]){
    /*Stock dans la mémoire tout le programme du fichier en hexadecimal */
    FILE *hexFile;

    hexFile = fopen(fileName, "r");
    if (hexFile == NULL) {
        printf( "Error when trying to read file %s\n", fileName);
        exit(-1);
    }

    char hexValue[10]; //taille du buffer
    char temp[3];
    int memIndex = 0;

    while (fgets(hexValue, sizeof hexValue, hexFile)) {
        for(int i = 0 ; i < HEX_LEN ; i+=2) {
            temp[0] = hexValue[i];
            temp[1] = hexValue[i+1];
            temp[2] = '\0';
            memory[memIndex++] = (unsigned char) strtol(temp, NULL, 16);
        }
    }
    fclose(hexFile);
}

/*Fonction permettant de lire les instructions se trouvant dans la memoire*/
Instruction *readMemory(CPU *cpu, unsigned char* memory){
    Instruction *instruc = malloc(sizeof(instruc));
    if (instruc == NULL){
        printf("Error malloc in readMemory\n");
        exit(-1);
    }

    //Recomposer l'instruction à mettre dans les differents pointeurs
    //Logique : A chaque fois que l'on recupere un nombre dans un emplacement on le convertie en un string avec sa representation binaire
    //On concatene les 4 strings obtenue pour reobtenir la forme binaire de l'instruction
    //On recupere par parquet pour ensuite stocker dans la struct instruction

    //On recompose l'instruction en binaire
    char binaryInstr[33] = "\0";
    char tmp[17];
    unsigned char currentMemoryDecimal;
    for (int i = 0; i < 4; i++) {
        currentMemoryDecimal = (unsigned char) memory[cpu->PC++];
        sprintf(tmp, "%08b", currentMemoryDecimal);
        strcat(binaryInstr, tmp);
        tmp[0] = '\0';
    }

    strncpy(tmp, binaryInstr, 5);
    tmp[5] = '\0';
    instruc->codeOp = (int) strtol(tmp, NULL, 2);

    strncpy(tmp, binaryInstr + 5, 5);
    tmp[5] = '\0';
    instruc->dest = (int) strtol(tmp, NULL, 2);

    strncpy(tmp, binaryInstr + 10, 5);
    tmp[5] = '\0';
    instruc->src1 = (int) strtol(tmp, NULL, 2);

    strncpy(tmp, binaryInstr + 15, 1);
    tmp[1] = '\0';
    instruc->imm = (int) strtol(tmp, NULL, 2);

    strncpy(tmp, binaryInstr + 16, 16);
    tmp[16] = '\0';
    instruc->src2  = (short) strtol(binaryInstr + 16, NULL, 2);

    return instruc;
}

/*Fonction de mise à jour du registre d'etat*/
int updateBitC(int value){
    if(value< -32768 || value > 32767){
        return 1;
    }
    return 0;
}

int updateBitZ(int value){
    if (value == 0){
        return 1;
    }
    return 0;
}

int updateBitN(int value){
    if (value < 0){
        return 1;
    }
    return 0;
}

/*instructions arithmétiques et logiques*/
/* à chaque fin d'operation il faut free l'espace occupe par l'instruction*/
void op_add(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] + cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] + instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_sub(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] - cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] - instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_mult(CPU *cpu, Instruction *instruc){
    int compute_value;
    short b;
    short a = cpu->registers[instruc->src1];
    if (instruc->imm == 0){
        b = cpu->registers[instruc->src2];
    }
    else{
        b = instruc->src2;
    }
    a = a & 0xFF;
    b = b & 0xFF;
    compute_value = a * b;
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_div(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] / cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] / instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_and(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] & cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] & instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_or(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] | cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] | instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_xor(CPU *cpu, Instruction *instruc){
    int compute_value;
    if (instruc->imm == 0){
        compute_value = cpu->registers[instruc->src1] ^ cpu->registers[instruc->src2];
    }
    else{
        compute_value = cpu->registers[instruc->src1] ^ instruc->src2;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitC = updateBitC(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

void op_shl(CPU *cpu, Instruction *instruc){
    int shift;
    int MSB;
    int LSB;
    short temp;
    short compute_value = cpu->registers[instruc->src1];

    if (instruc->imm == 0){
        shift = cpu->registers[instruc->src2];
    }

    else{
        shift = instruc->src2;
    }

    if (shift < 0){
        for (int i=0; i<abs(shift); i++){
            temp = compute_value;
            LSB = (temp << 15) & 1;
            compute_value = compute_value >> 1;
        }
        cpu->bitC = LSB;
    }
    else{
        for (int i=0; i<abs(shift); i++){
            temp = compute_value;
            MSB = (temp >> 15) & 1;
            compute_value = compute_value << 1;
        }
        cpu->bitC = MSB;
    }
    cpu->registers[instruc->dest] = compute_value;
    cpu->bitN = updateBitN(compute_value);
    cpu->bitZ = updateBitZ(compute_value);
    free(instruc);
}

/*instructions de transfert*/
void ldb(CPU *cpu, Instruction *instruc, unsigned char mem[]){
    char loaded_value;
    int address;
    if (instruc->imm == 0){
        address = cpu->registers[instruc->src1] + cpu->registers[instruc->src2];
    }
    else{
        address = cpu->registers[instruc->src1] + instruc->src2;
    }
    loaded_value = (char) mem[address];
    printf("Loaded value : %d Address : %d\n", loaded_value, address);

    cpu->registers[instruc->dest] = loaded_value;
    cpu->bitN = updateBitN(loaded_value);
    cpu->bitC = updateBitC(loaded_value);
    cpu->bitZ = updateBitZ(loaded_value);
    free(instruc);
}

void ldw(CPU *cpu, Instruction *instruc, unsigned char mem[]){
    short loaded_value;
    short a, b;
    int address;
    if (instruc->imm == 0){
        address = cpu->registers[instruc->src1] + cpu->registers[instruc->src2];
    }
    else{
        address = cpu->registers[instruc->src1] + instruc->src2;
    }
    a = (short)mem[address];
    b = (short)mem[address+1];
    b = b << 8;
    loaded_value = a+b;
    cpu->registers[instruc->dest] = loaded_value;
    cpu->bitN = updateBitN(loaded_value);
    cpu->bitC = updateBitC(loaded_value);
    cpu->bitZ = updateBitZ(loaded_value);
    free(instruc);
}

void stb(CPU *cpu, Instruction *instruc, unsigned char mem[]){
    char loaded_value;
    int address;
    if (instruc->imm == 0){
        address = cpu->registers[instruc->dest] + cpu->registers[instruc->src2];
    }
    else{
        address = cpu->registers[instruc->dest] + instruc->src2;
    }
    loaded_value = (char) cpu->registers[instruc->src1];

    mem[address] = loaded_value;

    cpu->bitN = updateBitN(loaded_value);
    cpu->bitC = updateBitC(loaded_value);
    cpu->bitZ = updateBitZ(loaded_value);
    free(instruc);
}

void stw(CPU *cpu, Instruction *instruc, unsigned char mem[]){
    short loaded_value;
    short a, b;
    int address;
    if (instruc->imm == 0){
        address = cpu->registers[instruc->dest] + cpu->registers[instruc->src2];
    }
    else{
        address = cpu->registers[instruc->dest] + instruc->src2;
    }
    loaded_value = cpu->registers[instruc->src1];
    a = loaded_value & 0xFF;
    b = loaded_value >> 8;

    mem[address] = (char)a;
    mem[address+1] = (char)b;
    cpu->bitN = updateBitN(loaded_value);
    cpu->bitC = updateBitC(loaded_value);
    cpu->bitZ = updateBitZ(loaded_value);
    free(instruc);
}



/*instructions de sauts*/
void jmp(CPU *cpu, Instruction *instruc){
    if (instruc->imm == 0){
        cpu->PC = cpu->registers[instruc->src2];
    }
    else{
        cpu->PC = instruc->src2;
    }
    free(instruc);
}

void jzs(CPU *cpu, Instruction *instruc){
    if (cpu->bitZ == 1){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

void jzc(CPU *cpu, Instruction *instruc){
    if (cpu->bitZ == 0){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

void jcs(CPU *cpu, Instruction *instruc){
    if (cpu->bitC == 1){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

void jcc(CPU *cpu, Instruction *instruc){
    if (cpu->bitC == 0){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

void jns(CPU *cpu, Instruction *instruc){
    if (cpu->bitN == 1){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

void jnc(CPU *cpu, Instruction *instruc){
    if (cpu->bitN == 0){
        if (instruc->imm == 0){
            cpu->PC = cpu->registers[instruc->src2];
        }
        else{
            cpu->PC = instruc->src2;
        }
    }
    free(instruc);
}

/*instructions d’entree-sortie*/
void in(CPU *cpu, Instruction *instruc){
    short input_value;
    printf("Entrer valeur : ");
    scanf("%hd", &input_value);
    cpu->registers[instruc->dest] = input_value; //a modifier
    cpu->bitN = updateBitN(input_value);
    cpu->bitC = updateBitC(input_value);
    cpu->bitZ = updateBitZ(input_value);
    free(instruc);
}

void out(CPU *cpu, Instruction *instruc){
    int print_value = cpu->registers[instruc->dest];
    printf("Valeur du registre R%d : %d\n", instruc->dest, print_value);
    cpu->bitN = updateBitN(print_value);
    cpu->bitC = updateBitC(print_value);
    cpu->bitZ = updateBitZ(print_value);
    free(instruc);
}

/*instructions diverses*/
void rnd(CPU *cpu, Instruction *instruc){
    srand(time(NULL));
    short a = cpu->registers[instruc->src1];
    short b;
    if (instruc->imm == 1){
        b = (short)instruc->src2;
    }
    else{
        b = cpu->registers[instruc->src2];
    }
    short randomised_value = (a<b)?a:b + rand() % (abs(b - a));
    cpu->registers[instruc->dest] = randomised_value;
    cpu->bitN = updateBitN(randomised_value);
    cpu->bitC = updateBitC(randomised_value);
    cpu->bitZ = updateBitZ(randomised_value);
    free(instruc);
}

void hlt(CPU *cpu, Instruction *instruc){
    printf("--- Fin de programme ---\n");
    free(instruc);
    free(cpu);
    exit(0);
}


/*Selecteur d'operation*/
void instruction_selector(CPU *cpu, Instruction *instruc, unsigned char mem[]){
    /*J'ajouterai les fonctions au selector une fois que les fonctions auront ete teste*/
    switch(instruc->codeOp){
        case 0:
            op_add(cpu, instruc);
            break;
        case 1:
            op_sub(cpu, instruc);
            break;
        case 2:
            op_mult(cpu, instruc);
            break;
        case 3:
            op_div(cpu, instruc);
            break;
        case 4:
            op_and(cpu, instruc);
            break;
        case 5:
            op_or(cpu, instruc);
            break;
        case 6:
            op_xor(cpu, instruc);
            break;
        case 7:
            op_shl(cpu, instruc);
            break;


        case 10:
            ldb(cpu, instruc, mem);
            break;
        case 11:
            ldw(cpu, instruc, mem);
            break;
        case 12:
            stb(cpu, instruc, mem);
            break;
        case 13:
            stw(cpu, instruc, mem);
            break;

        case 20:
            jmp(cpu, instruc);
            break;
        case 21:
            jzs(cpu, instruc);
            break;
        case 22:
            jzc(cpu, instruc);
            break;
        case 23:
            jcs(cpu, instruc);
            break;
        case 24:
            jcc(cpu, instruc);
            break;
        case 25:
            jns(cpu, instruc);
            break;
        case 26:
            jnc(cpu, instruc);
            break;


        case 27:
            in(cpu, instruc);
            break;
        case 28:
            out(cpu, instruc);
            break;


        case 29:
            rnd(cpu, instruc);
            break;
        case 31:
            hlt(cpu, instruc);
            break;
    }

    cpu->registers[0] = 0;
}


int exec(char* fileName){

    unsigned char memory[65536] = {0};
    CPU *processor = initCPU();
    Instruction *instruction;
    loadMemory(fileName, memory);

    while((instruction = readMemory(processor, memory)) != NULL){
        instruction_selector(processor, instruction, memory);
    }

    free(processor);
    return 0;
}