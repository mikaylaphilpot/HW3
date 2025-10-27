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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char name[12]; // name up to 11 chars
    int val; // number (ASCII value)
    int level; // L level
    int addr; // M address
    int mark; // to indicate unavailable or deleted
} symbol;

typedef struct{
    int OP;
    int L;
    int M;

} instruction;

symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];

instruction instructionSet[MAX_SYMBOL_TABLE_SIZE];

FILE *fp, *outputFile;
int nextToken; 
int tp;

// Grammar Functions

void program() {
    block();
}

void block () {
    if (nextToken == 28)
        constDeclaration();
    if (nextToken == 29)
        varDeclaration();
}

void constDeclaration () {
    char * identifier = malloc(sizeof(char)*12);
    int value;
    // loops because comma w/ more identifiers can occur 0 or more times
    do { 
        fscanf(fp, "%d", &nextToken);
        // Syntax error 2
        if (nextToken != 2) {
            printf("Error: const, var, and read keywords must be followed by identifier\n");
            fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
            return; // find diff way to halt
        }
        fscanf(fp, "%s", identifier);
        // Check if symbol name has already been declared
        // Syntax error 3
        if (findSymbol(identifier) != -1) {
            printf("Error: symbol name has already been declared\n");
            fprintf(outputFile, "Error: symbol name has already been declared");
            return 1;
        }
        fscanf(fp, "%d", &nextToken);
        // Syntax error 4
        if(nextToken != 8) {
            printf("Error: constants must be assigned with =\n");
            fprintf(outputFile, "Error: constants must be assigned with =");
            return 1;
        }
        // Syntax error 5
        (fscanf(fp, "%d", &nextToken));
        if (nextToken != 3) {
            printf("Error: constants must be assigned an integer value\n");
            fprintf(outputFile, "Error: constants must be assigned an integer value");
            return 1;
        }
        fscanf(fp, "%d", &value);
        // insert symbol name into table
        for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
            if(symbol_table[i].kind == 4) {
                symbol_table[i].kind = 1;
                strcpy(symbol_table[i].name, identifier);
                symbol_table[i].val = value;
                // TO-DO: set other symbol info
                break;
            }
        }
        fscanf(fp, "%d", &nextToken);
    } while (nextToken == 16);

        // Syntax error 6
    if (nextToken != 17) {
        printf("Error: constant and variable declarations must be followed by a semicolon\n");
        fprintf(outputFile, "Error: constant and variable declarations must be followed by a semicolon");
        return 1;
    }
}

void varDeclaration () {

}

int checkTable(char * identifier, int level) {
    for(int i = tp; i > 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0 && symbol_table[i].level == level) {
            return i;
        }
    }
    return -1;
}

int insertTable(int kind, char * identifier, int val, int level, int addr, int mark){
    if(checkTable(identifier, level) == -1 ){
        symbol s1;
        if (kind == 1){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.val = val;
            s1.level = level;
            s1.mark = mark;
        }
        else if(kind = 2){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.level = level;
            s1.addr = addr;
            s1.mark = mark;
        }else{
            printf("Procedure Called! Can't do.");
        }
             
        symbol_table[tp] = s1;
    }
    
}

void deleteSymbol(char * identifier, int level){
    for(int i = tp; i > 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0 && symbol_table[i].level == level) {
            symbol_table[i].mark = 1;
        }
    }
}

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
    fp = fopen("tokens.txt", "r");
    outputFile = fopen("elf.txt", "w");

    //global variable declaration
    nextToken = 0;
    tp = 1;

    /*char * identifier = malloc(sizeof(char)*12);
    int value; 
    // reading through each token in input file
    while(fscanf(fp, "%d", &nextToken) == 1) {
        // checking for skipsym error
        if (nextToken == 1) {
            printf("Error: Scanning error detected by lexer (skipsym present)\n");
            fprintf(outputFile, "Error: Scanning error detected by lexer (skipsym present)");
            return 1;
        }
        // Syntax error 1
        else if (nextToken == 21) {
            fscanf(fp, "%d", &nextToken);
            if (nextToken != 18) {
                printf("Error: program must end with period\n");
                fprintf(outputFile, "Error: program must end with period");
                return 1;
            }
        }
        
        else if (nextToken == 28) {
            // loops because comma w/ more identifiers can occur 0 or more times
            do { 
                fscanf(fp, "%d", &nextToken);
                // Syntax error 2
                if (nextToken != 2) {
                    printf("Error: const, var, and read keywords must be followed by identifier\n");
                    fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
                    return 1;
                }
                fscanf(fp, "%s", identifier);
                // Check if symbol name has already been declared
                // Syntax error 3
                if (findSymbol(identifier) != -1) {
                    printf("Error: symbol name has already been declared\n");
                    fprintf(outputFile, "Error: symbol name has already been declared");
                    return 1;
                }
                fscanf(fp, "%d", &nextToken);
                // Syntax error 4
                if(nextToken != 8) {
                    printf("Error: constants must be assigned with =\n");
                    fprintf(outputFile, "Error: constants must be assigned with =");
                    return 1;
                }
                // Syntax error 5
                (fscanf(fp, "%d", &nextToken));
                if (nextToken != 3) {
                    printf("Error: constants must be assigned an integer value\n");
                    fprintf(outputFile, "Error: constants must be assigned an integer value");
                    return 1;
                }
                fscanf(fp, "%d", &value);
                // insert symbol name into table
                for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
                    if(symbol_table[i].kind == 4) {
                        symbol_table[i].kind = 1;
                        strcpy(symbol_table[i].name, identifier);
                        symbol_table[i].val = value;
                        // TO-DO: set other symbol info
                        break;
                    }
                }
                fscanf(fp, "%d", &nextToken);
            } while (nextToken == 16);

            // Syntax error 6
            if (nextToken != 17) {
                printf("Error: constant and variable declarations must be followed by a semicolon\n");
                fprintf(outputFile, "Error: constant and variable declarations must be followed by a semicolon");
                return 1;
            }
            
        } 

        // Syntax error 2 - var
        else if (nextToken == 29) {
            do {
                fscanf(fp, "%d", &nextToken);
                if (nextToken != 2) {
                    printf("Error: const, var, and read keywords must be followed by identifier\n");
                    fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
                    return 1;
                }
                
                fscanf(fp, "%s", identifier);
                // Syntax error 3
                if (findSymbol(identifier) != -1) {
                    printf("Error: symbol name has already been declared\n");
                    fprintf(outputFile, "Error: symbol name has already been declared");
                    return 1;
                }
                
                // insert symbol name into table
                for (int i = 0; i < MAX_SYMBOL_TABLE_SIZE; i++) {
                    if(symbol_table[i].kind == 4) {
                        symbol_table[i].kind = 2;
                        strcpy(symbol_table[i].name, identifier);
                        // TO-DO: set other symbol info
                        break;
                    }
                }
                fscanf(fp, "%d", &nextToken);
            } while (nextToken == 16); // check for commasym

            // Syntax error 6
            if (nextToken != 17) {
                printf("Error: constant and variable declarations must be followed by a semicolon\n");
                fprintf(outputFile, "Error: constant and variable declarations must be followed by a semicolon");
                return 1;
            }

        }

        // Syntax error 2 - read
        else if (nextToken == 32) {
            fscanf(fp, "%d", &nextToken);
            if (nextToken != 2) {
                printf("Error: const, var, and read keywords must be followed by identifier\n");
                fprintf(outputFile, "Error: const, var, and read keywords must be followed by identifier");
                return 1;
            }
            fscanf(fp, "%s", identifier);
        }

        // Syntax error 7
        else if (nextToken == 2) {
            fscanf(fp, "%s", identifier);
            if (findSymbol(identifier) == -1) {
                printf("Error: undeclared identifier\n");
                fprintf(outputFile, "Error: undeclared identifier");
                return 1;
            }

        }
    }
    // TO-DO read through each token and check for potential errors as you read
    // Produce assembly code if no errors are found
    // Otherwise only the first error message should be produced, both in terminal and elf.txt

    // TO-DO output to elf.txt file*/
}