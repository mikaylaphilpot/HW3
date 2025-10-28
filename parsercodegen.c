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

instruction code[500];
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];

instruction instructionSet[MAX_SYMBOL_TABLE_SIZE];

FILE *fp, *outputFile, *readFile;
int nextToken; 
int tp;
int cx = 0; // code index, increments by one each time an instruction is stored

//Emit function
void emit(int OP, int L, int M){
    instructionSet[cx].OP = OP;
    instructionSet[cx].L = L;
    instructionSet[cx].M = M;
    cx++;
}
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
void insertSymbol(int kind, char * identifier, int val, int level, int addr, int mark);
void deleteSymbol(char * identifier, int level);
void printAssemblyCode();
void printSymbolTable();
char * determineOpcode(int i);
void error (int errorNumber);
void getNextToken();


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
            insertSymbol(1, identifier, value, 0, 0, 0);
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
            insertSymbol(2, identifier, 0, 0, numVars + 2, 0);
            getNextToken();
        } while(nextToken == 16);

        if (nextToken != 17) {
            error(6);
        }
        fscanf(fp, " %d ", &nextToken);

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
            fscanf(fp, " %d", &nextToken);
            statement();
        } while (nextToken == 17);
        // Syntax error 10
        if (nextToken != 21) {
            error(10);
        }
        fscanf(fp, "%d ", &nextToken);
        return;
    }
    if(nextToken == 22) {
        getNextToken();
        condition();
        int jpcIndex = cx;
        // conditional jump to token 24
        emit (8, 0, 24);
        if (nextToken != 24) {
            error(11);
        }
        getNextToken();
        statement();
        code[jpcIndex].M = cx;
        // next token should be fi based on grammar but no error specified
        if(nextToken == 23) {
            getNextToken();
        }
        return;
    }
    if(nextToken == 25) {
        getNextToken();
        int loopIndex = cx;
        condition();
        if (nextToken != 26) {
            error(12);
        }
        getNextToken();
        int jpcIndex = cx;
        emit (8, 0, 0);
        statement();
        emit(7, 0, loopIndex);
        code[jpcIndex].M = cx;
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
        emit(3, 0, symbol_table[symIndex].addr);
        emit(4, 0, symbol_table[symIndex].addr); // TO-DO: figure out/verify what instructions read calls
        return;
    }
    if(nextToken == 31) {
        getNextToken();
        expression();
        emit(9, 0, 1); // TO-DO: figure out what instructions write calls
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

//SYMBOL TABLE FUNCTIONS

//&& symbol_table[i].level == level no level check 
int findSymbol(char * identifier) {
    for(int i = tp; i > 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0) {
            return i;
        }
    }
    return -1;
}

void insertSymbol(int kind, char * identifier, int val, int level, int addr, int mark){
    if(findSymbol(identifier) == -1 ){
        symbol s1;
        if (kind == 1){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.val = val;
            s1.level = level;
            s1.mark = mark;
        }
        else if(kind == 2){
            s1.kind = kind;
            strcpy(s1.name, identifier);
            s1.level = level;
            s1.addr = addr;
            s1.mark = mark;
        }else{
            printf("Procedure Called! Can't do.");
        }
             
        symbol_table[tp] = s1;
        tp++;
    }
    
}

void deleteSymbol(char * identifier, int level){
    for(int i = tp; i > 0; i--) {
        if(strcmp(symbol_table[i].name, identifier) == 0 && symbol_table[i].level == level) {
            symbol_table[i].mark = 1;
        }
        tp--;
    }
}

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
    }
    printf("\n");
}

void printSymbolTable() {
    // To-Do: Figure out what parameters need to be passed
    // To-Do: Print Beginning and Format Output
    // To-Do: print actual content
}

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
    char * errors [16] = {"Error: Scanning error detected by lexer (skipsym present)", "Error: program must end with period", "Error: const, var, and read keywords must be followed by identifier", 
        "Error: symbol name has already been declared", "Error: constants must be assigned with =", "Error: constants must be assigned an integer value", 
        "Error: constant and variable declarations must be followed by a semicolon", "Error: undeclared identifier", "Error: only variable values may be altered", "Error: assignment statements must use :=",
        "Error: begin must be followed by end", "Error: if must be followed by then", "Error: while must be followed by do", "Error: condition must contain comparison operator", "Error: right parenthesis must follow left parenthesis", 
        "Error: arithmetic equations must contain operands, parentheses, numbers, or symbols"};
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
    tp = 1;

    program();
    printAssemblyCode();
    exit(EXIT_SUCCESS);
}