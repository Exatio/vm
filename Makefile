CFLAGS=-Wall -lm

all: simulateur

simulateur: main compiler virtualmachine
	gcc -o simulateur main.o compiler.o virtualMachine.o $(CFLAGS)

main: main.c
	gcc -o main.o -c main.c $(CFLAGS)

compiler: compiler.c compiler.h
	gcc -o compiler.o -c compiler.c $(CFLAGS)

virtualmachine: virtualMachine.c virtualMachine.h
	gcc -o virtualMachine.o -c virtualMachine.c $(CFLAGS)

clean:
	rm -rf ./*.o
