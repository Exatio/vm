#include <stdio.h>
#include <string.h>

#include "compiler.h"
#include "virtualMachine.h"

void printUsage(char* program)
{
    printf("Usage : %s <file.txt>\n", program);
}

int main(int argc, char* argv[])
{
    if(argc == 2)
    {
        char hexaFileName[strlen(argv[1]) + 6];
        sprintf(hexaFileName, "hexa_%s", argv[1]);
        int compileResult = compile(argv[1], hexaFileName);
        if(compileResult == 0)
            return exec(hexaFileName); // If compile doesn't return 0, exec won't be called
        return compileResult;
    }

    printUsage(argv[0]);
    return 1;
}
