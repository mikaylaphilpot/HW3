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

// Struct Declarations
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

FILE *fp, *outputFile, *readFile;
int nextToken; 
int tp;
int symbCount;
int cx = 0; // code index, increments by one each time an instruction is stored

// Function Prototypes
void program();
void block();
void constDeclaration();
int varDeclaration();
void statement();
void condition(); 
void expression();
void term();
void factor();
int findSymbol(char * identifier);
void insertSymbol(int kind, char * identifier, int val, int level, int addr);
void deleteSymbol(char * identifier);
void printAssemblyCode();
void printSymbolTable();
char * determineOpcode(int i);
void error (int errorNumber);
void getNextToken();
void emit(int OP, int L, int M);

//Emit function
void emit(int OP, int L, int M){
    if (cx > 500) {
        error(16);
    }
    instructionSet[cx].OP = OP;
    instructionSet[cx].L = L;
    instructionSet[cx].M = M;
    cx++;
}

// Grammar Functions

void program() {
    block();
    if (nextToken != 18) {
        error(1);
    }
    emit(9, 0, 3); // emit halt
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
            getNextToken(); // get next token
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
            getNextToken();
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
            getNextToken();
        } while (nextToken == 16);
        // Syntax error 6
        if (nextToken != 17) {
            error(6);
        }
        getNextToken();
    }
}

int varDeclaration () {
    int numVars = 0;
    if (nextToken == 29) {
        char * identifier = malloc(sizeof(char)*12);
        do {
            numVars++;
            getNextToken();
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
            getNextToken();
        } while(nextToken == 16);

        if (nextToken != 17) {
            error(6);
        }
        getNextToken();

    }
    return numVars;
}

void statement () {
    char * identifier = malloc(sizeof(char)*12);
    if (nextToken == 2) {
        fscanf(fp, "%s", identifier);
        
        int symIndex = findSymbol(identifier);
        // Syntax error 7
        if (symIndex == -1) {
            error(7);
        }
        // Syntax error 8
        if (symbol_table[symIndex].kind != 2) {
            error(8);
        }
        getNextToken();
        // Syntax error 9
        if (nextToken != 19) {
            error(9);
        }
        getNextToken();
        expression();
        emit(4, 0, symbol_table[symIndex].addr);
        return;
    }
    if(nextToken == 20) {
        do {
            getNextToken();
            statement();
        } while (nextToken == 17);
        // Syntax error 10
        if (nextToken != 21) {
            error(10);
        }
        getNextToken();
        return;
    }
    if(nextToken == 22) {
        getNextToken();
        condition();
        int jpcIndex = cx;
        emit (8, 0, 0);
        if (nextToken != 24) {
            error(11);
        }
        getNextToken();
        statement();
        instructionSet[jpcIndex].M = cx*3;
        getNextToken();
        // next token should be fi based on grammar but no error specified
        if(nextToken == 23) {
            getNextToken();
        }
        return;
    }
    if(nextToken == 25) {
        getNextToken();
        int loopIndex = cx*3;
        condition();
        if (nextToken != 26) {
            error(12);
        }
        getNextToken();
        int jpcIndex = cx;
        emit (8, 0, 0);
        statement();
        emit(7, 0, loopIndex);
        instructionSet[jpcIndex].M = cx*3;
        return;
    }
    if (nextToken == 32) {
        getNextToken();
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
        getNextToken();
        emit(9, 0, 2);
        emit(4, 0, symbol_table[symIndex].addr);
        return;
    }
    if(nextToken == 31) {
        getNextToken();
        expression();
        emit(9, 0, 1);
        return;
    }
}

void condition() { 
    if(nextToken == 34){
        getNextToken();
        expression();
        emit(2, 0, 11); //EVEN
    }else{
        expression();
            if(nextToken == 8){
                getNextToken();
                expression();
                emit(2, 0, 5); //EQl
            }
            else if(nextToken == 9){
                getNextToken();
                expression();
                emit(2, 0, 6); //NEQ
            }
            else if(nextToken == 10){
                getNextToken();
                expression();
                emit(2, 0, 7); //LSS
            }
            else if(nextToken == 11){
                getNextToken();
                expression();
                emit(2, 0, 8); //LEQ
            }
            else if(nextToken == 12){
                getNextToken();
                expression();
                emit(2, 0, 9); //GTR
            }
            else if(nextToken == 13){
                getNextToken();
                expression();
                emit(2, 0, 10); //GEQ
            }
            else{
                //condition error
                error(13);
            }
    }
    
}

void expression() {

    term();
    while(nextToken == 4 || nextToken == 5){
        if(nextToken == 4){
            getNextToken();
            term();
            emit(2, 0, 1); //ADD
        }
        else{
            getNextToken();
            term();
            emit(2, 0, 2); //SUB
        }
    }

}

void term(){

    factor();
    while(nextToken == 6 || nextToken == 7){
        if(nextToken == 6){
            getNextToken();
            factor();
            emit(2, 0, 3); //MUL
        }else{
            getNextToken();
            factor();
            emit(2, 0, 4); //DIV
        }
    }

}

void factor(){

    char * identifier = malloc(sizeof(char)*12);
    int value; 

    if(nextToken == 2){

        fscanf(fp, "%s", identifier);
        int symIdx = findSymbol(identifier);

        if(symIdx == -1){
            error(7);
        }
        else if(symbol_table[symIdx].mark == 1){
            error(17);
        }
        else if(symbol_table[symIdx].kind == 1){
            emit(1, 0, symbol_table[symIdx].val); //LIT 
        }
        else{
            emit(3, 0, symbol_table[symIdx].addr); //LOD 
        }
        getNextToken();
    }
    else if(nextToken == 3){
        fscanf(fp, "%d", &value); // getting num value
        emit(1, 0, value); //LIT
        getNextToken();
    }
    else if(nextToken == 14){
        getNextToken();
        expression();
        if(nextToken != 15){
            error(14);
        }
        getNextToken();
    }
    else{
        error(15); //TO-DO: check if correct error
    }

}

// Symbol Table Functions

int findSymbol(char * identifier) {
    int i;
    for(i = tp; i >= 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0) {
            return i;
        }
    }
    return i;
}

void insertSymbol(int kind, char * identifier, int val, int level, int addr){
    if(findSymbol(identifier) == -1 ){
        symbol s1;
        if (kind == 1){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.val = val;
            s1.level = level;
            s1.addr = 0;
            s1.mark = 0;
        }
        else if(kind == 2){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.val = 0;
            s1.level = level;
            s1.addr = addr;
            s1.mark = 0;
            s1.mark = 0;
        }else{
            error(18);
        }
             
        symbol_table[tp] = s1;
        tp++;
        symbCount++;
        
    }
    
}

void deleteSymbol(char * identifier){

    for(int i = tp; i >= 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0) {
            symbol_table[i].mark = 1;
        }
    }
    tp--;
}

// Print Functions
void printAssemblyCode() {
    printf("Assembly code: \n\n");
    printf("\nLine\tOP\tL\tM\n");
    for (int i = 0; i < cx; i++) {
        printf("\n%d", i);
        // print instruction name based on its opcode
        printf("\t%s", determineOpcode(i));
        // Print L
        printf("\t%d", instructionSet[i].L);
        // Print M
        printf("\t%d", instructionSet[i].M);

        // printing to file
        fprintf(outputFile, "%d", instructionSet[i].OP);
        fprintf(outputFile, " %d", instructionSet[i].L);
        fprintf(outputFile, " %d\n", instructionSet[i].M);
    }
    printf("\n\n");
}

void printSymbolTable() {
    
    printf("Symbol Table:\n\n");

    printf("Kind | Name \t |  Value |  Level |  Address|  Mark\n");
    printf("---------------------------------------------------\n");
    // for each symbol print kind, name, value, level, adress, and mark
    for(int i = 0; i < symbCount; i++){
        printf(" %d| \t\t%s| \t%d |\t %d |  \t%d | %d\n", symbol_table[i].kind, symbol_table[i].name, symbol_table[i].val, symbol_table[i].level, symbol_table[i].addr, symbol_table[i].mark);
        
    }

}

// Helper functions
char * determineOpcode(int i) {
    // determine instruction name based on its opcode
    if (instructionSet[i].OP == 1) {
        return "LIT";
    }
    if (instructionSet[i].OP == 2) {
        return "OPR";
    }
    else if (instructionSet[i].OP == 3) {
        return "LOD";
    }
    else if (instructionSet[i].OP == 4) {
        return "STO";
    }
    else if (instructionSet[i].OP == 5) {
        return "CAL";
    }
    else if (instructionSet[i].OP == 6) {
        return "INC";

    }
    else if (instructionSet[i].OP == 7) {
        return "JMP";
    }
    else if (instructionSet[i].OP == 8) {
        return "JPC";
    }   
    return "SYS";
}

void getNextToken () {
    fscanf(fp, "%d", &nextToken);
    if (nextToken == 1) {
        error(0);
    }
}

// called when there's an error
void error (int errorNumber) {
    char * errors [19] = {"Error: Scanning error detected by lexer (skipsym present)", "Error: program must end with period", "Error: const, var, and read keywords must be followed by identifier", 
        "Error: symbol name has already been declared", "Error: constants must be assigned with =", "Error: constants must be assigned an integer value", 
        "Error: constant and variable declarations must be followed by a semicolon", "Error: undeclared identifier", "Error: only variable values may be altered", "Error: assignment statements must use :=",
        "Error: begin must be followed by end", "Error: if must be followed by then", "Error: while must be followed by do", "Error: condition must contain comparison operator", "Error: right parenthesis must follow left parenthesis", 
        "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols", "Error: code index exceeded code length", "Error: can't access symbol because mark set to one", "Error: program doesn't handle procedures"};
    fclose(outputFile);
    // closing and re-opening the output file clears all previous printed text
    outputFile = fopen("elf.txt", "w");
    printf("%s\n", errors[errorNumber]);
    fprintf(outputFile, "%s\n", errors[errorNumber]);
    exit(EXIT_FAILURE);
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
    getNextToken();
    tp = 0;
    symbCount = 0;

    program();
    
    for(int i = tp; i >= 0; i--){
        deleteSymbol(symbol_table[i].name);
    }
        
    
    printAssemblyCode();
    printSymbolTable();
    exit(EXIT_SUCCESS);
}