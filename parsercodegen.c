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

typedef struct {
    int opcode;
    int L;
    int M;
} instruction;

instruction code[500];
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];


FILE *fp, *outputFile;
int nextToken; 
int tp;
int cx = 0; // code index, increments by one each time an instruction is stored

// Grammar Functions

void program() {
    block();
    if (nextToken != 18) {
        error(1);
    }
    exit();
}

void block () {
    constDeclaration();
    int numsVars = varDeclaration();
    emit(6, 0, 3 + numsVars);
    statement();
}

void constDeclaration () {
    if(nextToken == 28) {
        char * identifier = malloc(sizeof(char)*12);
        int value;
        // loops because comma w/ more identifiers can occur 0 or more times
        do { 
            fscanf(fp, "%d", &nextToken); // get next token
            // Syntax error 2
            if (nextToken != 2) {
                error(2);
            }
            fscanf(fp, "%s", identifier);
            // Check if symbol name has already been declared
            // Syntax error 3
            if (findSymbol(identifier) != -1) {
                error(3);
            }
            fscanf(fp, "%d", &nextToken);
            // Syntax error 4
            if(nextToken != 8) {
                error(4);
            }
            // Syntax error 5
            (fscanf(fp, "%d", &nextToken));
            if (nextToken != 3) {
                error(5);
            }
            fscanf(fp, "%d", &value);
            // insert symbol name into table
            insertSymbol(1, identifier, value, 0, 0);
            fscanf(fp, "%d", &nextToken);
        } while (nextToken == 16);
        // Syntax error 6
        if (nextToken != 17) {
            error(6);
        }
        fscanf(fp, "%d", &nextToken);
    }
}

int varDeclaration () {
    int numVars = 0;
    if (nextToken == 29) {
        char * identifier = malloc(sizeof(char)*12);
        int value;
        do {
            numVars++;
            fscanf(fp, "%d", &nextToken);
            // Syntax error 2
            if (nextToken != 2) {
                error(2);
            }
            fscanf(fp, "%s", identifier);
            // Syntax error 3
            if(findSymbol(identifier) != -1) {
                error(3);
            }
            insertSymbol(2, identifier, 0, 0, numVars + 2);
            fscanf(fp, "%d", &nextToken);
        } while(nextToken == 16);

        if (nextToken != 17) {
            error(6);
        }
        fscanf(fp, "%d", &nextToken);
    }
    return numVars;
}

void statement () {
    char * identifier = malloc(sizeof(char)*12);
    fscanf(fp, "%s", identifier);
    if (nextToken == 2) {
        int symIndex = findSymbol(nextToken);
        // Syntax error 7
        if (symIndex == -1) {
            error(7);
        }
        // Syntax error 8
        if (symbol_table[symIndex].kind != 2) {
            error(8);
        }
        fscanf(fp, "%d", &nextToken);
        // Syntax error 9
        if (nextToken != 19) {
            error(9);
        }
        fscanf(fp, "%d", &nextToken);
        expression();
        emit(4, 0, symbol_table[symIndex].addr);
        return;
    }
    if(nextToken == 20) {
        do {
            fscanf(fp, "%d", &nextToken);
            statement();
        } while (nextToken == 17);
        // Syntax error 10
        if (nextToken != 21) {
            error(10);
        }
        fscanf(fp, "%d", &nextToken);
        return;
    }
    if(nextToken == 22) {
        fscanf(fp, "%d", &nextToken);
        condition();
        int jpcIndex = cx;
        emit (8, 0, 0);
        if (nextToken != 24) {
            error(11);
        }
        fscanf(fp, "%d", &nextToken);
        statement();
        code[jpcIndex].M = cx;
        return;
    }
    if(nextToken == 25) {
        fscanf(fp, "%d", &nextToken);
        int loopIndex = cx;
        condition();
        if (nextToken != 26) {
            error(12);
        }
        fscanf(fp, "%d", &nextToken);
        int jpcIndex = cx;
        emit (8, 0, 0);
        statement();
        emit(7, 0, loopIndex);
        code[jpcIndex].M = cx;
        return;
    }
    if (nextToken == 32) {
        fscanf(fp, "%d", &nextToken);
        if (nextToken != 2) {
            error(2);
        }
        fscanf(fp, "%s", identifier);
        int symIndex = findSymbol(identifier);
        if (symIndex == -1) {
            error(7);
        }
        if (symbol_table[symIndex].kind != 2) {
            error(8);
        }
        fscanf(fp, "%d", &nextToken);
        emit(3, 0, symbol_table[symIndex].addr);
        emit(4, 0, symbol_table[symIndex].addr); // TO-DO: figure out/verify what instructions read calls
        return;
    }
    if(nextToken == 31) {
        fscanf(fp, "%d", &nextToken);
        expression();
        emit(0, 0, 0); // TO-DO: figure out what instructions write calls
        return;
    }
}

void expression() {

}

void condition() {

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
        symbol s1 = {kind, identifier, val, level, addr, mark};
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

// exit gracefully
void exit() {

}
// called when there's an error
void error (int errorNumber) {
    char * errors [16] = {"Error: Scanning error detected by lexer (skipsym present)", "Error: program must end with period", "Error: const, var, and read keywords must be followed by identifier", 
        "Error: symbol name has already been declared", "Error: constants must be assigned with =", "Error: constants must be assigned an integer value", 
        "Error: constant and variable declarations must be followed by a semicolon", "Error: undeclared identifier", "Error: only variable values may be altered", "Error: assignment statements must use :=",
        "Error: begin must be followed by end", "Error: if must be followed by then", "Error: while must be followed by do", "Error: condition must contain comparison operator", "Error: right parenthesis must follow left parenthesis", 
        "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols"};
    printf("%s\n", errors[errorNumber]);
    fprintf(outputFile, "%s\n", errors[errorNumber]);
    exit();
}

int main (int argc, char *argv[])
{
    // No arguments accepted
    if(argc != 1){
        printf("Error! Wrong number of arguments.\n");
        return 1;
    } 

    emit(7, 0, 3);
    // Note: input file name is hardcoded
    // take input from lex.c output file
    fp = fopen("tokens.txt", "r");
    outputFile = fopen("elf.txt", "w");

    //global variable declaration
    nextToken = 0;
    tp = 1;

    program();
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