/*
Assignment:
HW3 - Parser and Code Generator for PL/0
Author(s): Mikayla Philpot, Sindhuja Sesham
Language: C (only)
To Compile:
Scanner:
gcc -O2 -std=c11 -o lex lex.c
Parser/Code Generator:
gcc -O2 -std=c11 -o parsercodegen parsercodegen.c
To Execute (on Eustis):
./lex tokens.txt
./parsercodegen
where:
tokens.txt is the path to the PL/0 source program
Notes:
- lex.c accepts ONE command-line argument (input PL/0 source file)
- parsercodegen.c accepts NO command-line arguments
- Input filename is hard-coded in parsercodegen.c
- Implements recursive-descent parser for PL/0 grammar
- Generates PM/0 assembly code (see Appendix A for ISA)
- All development and testing performed on Eustis
Class: COP3402 - System Software - Fall 2025
Instructor: Dr. Jie Lin
Due Date: Friday, October 31, 2025 at 11:59 PM ET
*/
#define MAX_SYMBOL_TABLE_SIZE 500
#include <stdio.h>

typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} symbol;

symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];

void printAssemblyCode() {
    // To-Do: Figure out what parameters need to be passed
    // To-Do: Print Beginning and Format Output
    // To-Do: print actual content
}

void printSymbolTable() {
    // To-Do: Figure out what parameters need to be passed
    // To-Do: Print Beginning and Format Output
    // To-Do: print actual content
}


int main (int argc, char *argv[])
{
    // No arguments accepted
    if(argc != 1){
        printf("Error! Wrong number of arguments.\n");
        return 1;
    } 

    // Note: input file name is hardcoded
    // take input from lex.c output file
    FILE *fp = fopen("tokens.txt", "r");
    FILE *outputFile = fopen("elf.txt", "w");
    
    int nextToken = 0;
    char * identifier = malloc(sizeof(char)*12);
    // reading through each token in input file
    while(fscanf(fp, "%d", &nextToken) == 1) {
        // checking for skipsym error
        if (nextToken == 1) {
            printf("Error: Scanning error detected by lexer (skipsym present)");
            fprintf(outputFile, "Error: Scanning error detected by lexer (skipsym present)");
        }
        // Syntax error 1
        else if (nextToken == 21) {
            nextToken = fscanf(fp, "%d", &nextToken);
            if (nextToken != 18) {
                printf("Error: program must end with period");
                fprintf(outputFile, "Error: program must end with period");
            }
        }
        // Syntax error 2
        else if (nextToken == 28 || nextToken == 29 || nextToken == 32) {
            nextToken = fscanf(fp, "%d", &nextToken);
            if (nextToken != 2) {
                printf("Error: const, var, and read keywords must be followed by identifier");
                fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
            }
        }
        // Syntax error 3
        else if (nextToken == 2) {
            fscanf(fp, "%s", &identifier);
            // TO-DO Check if symbol name has already been declared
            // Call find function for symbol table
            // Call insert function
        }
    }
    // TO-DO read through each token and check for potential errors as you read
    // Produce assembly code if no errors are found
    // Otherwise only the first error message should be produced, both in terminal and elf.txt

    // TO-DO output to elf.txt file
}