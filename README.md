# Virtual Machine Simulator - User Manual

## Introduction

This project is a virtual machine simulator. The simulator allows users to compile and run assembly programs with specific syntactical requirements.

## Prerequisites

- Ensure you have a terminal/command line interface
- Have `make` installed on your system

## Installation and Usage

### Compilation
To compile the simulator, run:
```bash
$ make all
```

### Running a Program
To execute a program (e.g., demo_sum.txt):
```bash
$ ./simulateur demo_sum.txt
```

Note: Successful compilation will generate a `hexa_demo_sum.txt` file with the compilation result.

## Syntax Guidelines

### Label Definitions
- No spaces between label and colon
- No newline after a label
- Correct: `label: instruction`
- Incorrect: `label : instruction` or `label:\n    instruction`

### Argument Formatting
- Commas should be immediately after arguments
- Correct: `add r1, r0, #5`
- Incorrect: `add r1 , r0 , #5`

## Technical Details

### Literal Values
- Decimal values: `#5`, `#1025`, `#-2511`
- Hexadecimal values: `#hFFFF` (two's complement representation)

### Limitations
- 32 registers available (r0 to r31)
- 16-bit signed literal values (-32,767 to 32,768)

### Code Constraints
- Maximum 100 characters per line
- Maximum 30 characters per label
- Maximum 50 labels per program

These constraints can be adjusted in `compiler.h`.

## Caution
- Avoid using reserved words as label names
- Carefully check label usage to prevent unexpected behavior

## Examples

Sample assembly programs are provided to help you get familiar with the simulator. Compile and run these examples to understand the system's functionality.
