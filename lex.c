

/*
Assignment :
lex - Lexical Analyzer for PL /0

Author : Sindhuja Sesham, Mikayla Philpot

Language : C ( only )

To Compile :
gcc - O2 - std = c11 -o lex lex . c

To Execute ( on Eustis ):
./ lex < input file >

where :
< input file > is the path to the PL /0 source program

Notes :
- Implement a lexical analyser for the PL /0 language .
- The program must detect errors such as
- numbers longer than five digits
- identifiers longer than eleven characters
- invalid characters .
- The output format must exactly match the specification .
- Tested on Eustis .

Class : COP 3402 - System Software - Fall 2025
Instructor : Dr . Jie Lin
Due Date : Friday , October 3 , 2025 at 11:59 PM ET
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXNUM 5
#define MAXID 11

typedef enum {
skipsym = 1,        // Skip / ignore token
identsym = 2,       // Identifier
numbersym = 3,      // Number
plussym = 4,        // +
minussym = 5,       // -
multsym = 6,        // *
slashsym = 7,       // /
eqsym = 8,          // =
neqsym = 9,         // <>
lessym = 10,        // <
leqsym = 11,        // <=
gtrsym = 12,        // >
geqsym = 13,        // >=
lparentsym = 14,    // (
rparentsym = 15,    // )
commasym = 16,      // ,
semicolonsym = 17,  // ;
periodsym = 18,     // .
becomessym = 19,    // :=
beginsym = 20,      // begin
endsym = 21,        // end
ifsym = 22,         // if
fisym = 23,         // fi
thensym = 24,       // then
whilesym = 25,      // while
dosym = 26,         // do
callsym = 27,       // call
constsym = 28,      // const
varsym = 29,        // var
procsym = 30,       // procedure
writesym = 31,      // write
readsym = 32,       // read
elsesym = 33,       // else
evensym = 34,       // even
} TokenType;

typedef struct {
    TokenType tokenType;
    char * lexeme;
    int startingIndex;
} Token;

// String Duplicate function 
char *stringDupe(const char *s) {
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) {
        memcpy(p, s, size);
    }
    return p;
}

//PRINT FUNCTIONS

void printSourceProgram(char *input, int size){

    printf("\nSource Program:\n\n");

    for(int i = 0; i < size; i++) {

        printf("%c", input[i]);

    }

}

void printLexemeTable(Token * allTokens, int size){

    printf("\n\nLexeme Table:\n\n");
    printf("lexeme\ttoken type");

    for (int i = 0; i < size; i++) {
        
        //Error Handling
        if(allTokens[i].tokenType == 1) {

            //Identifier Error
            if(isalpha(allTokens[i].lexeme[0])) {
                printf("\n%s\tIdentifier too long", allTokens[i].lexeme);
            }

            //Number error
            else if (isdigit(allTokens[i].lexeme[0])) {
                printf("\n%s\tNumber too long", allTokens[i].lexeme); 
            }

            //Invalid Token Error
            else {
                
                printf("\n%c\tInvalid token", allTokens[i].lexeme[0]); 
            }

            continue;

        }

        printf("\n%s\t%d", allTokens[i].lexeme, allTokens[i].tokenType);

    }

}

void printTokenList(Token * allTokens, int size){

    FILE * fptr;
    fptr = fopen("tokens.txt", "w");

    fprintf(fptr, "\n\nToken List:\n\n");

    for(int i = 0; i < size; i++) {

        
        //Printing Identifiers and Numbers
        if(allTokens[i].tokenType == 2 || allTokens[i].tokenType == 3) {

            fprintf(fptr, "%d %s ", allTokens[i].tokenType, allTokens[i].lexeme);

        }
        else {

            fprintf(fptr, "%d ", allTokens[i].tokenType);

        }
        fprintf(fptr, "\n");
    }
    
    
}

int main(int argc, char *argv[]){
    // TO-DO: Print to output file instead of stdout 
    // Note: This output file is used as input for parsercodegen.c

    //Argument count check and reading file input
    if(argc != 2){
        printf("Error! Wrong number of arguments.\n");
        return 1;
    } 

    FILE *inputFile = fopen(argv[1], "r");

    if (inputFile == NULL) {
        printf("Error opening file.\n");
        return 1;
    }
    
    //Reading all of file into char point

    char *lines = (char*) malloc(sizeof(char)*500);
    
    int ch; 
    int index = 0;

    while ((ch = fgetc(inputFile)) != EOF) {
        lines[index] = (char)ch;
        index++;
    }

    lines[index] = '\0';

    //Print source program
    printSourceProgram(lines, index);

    //initalizing token list and adding tokens into it
    Token *tokenList = malloc(100*sizeof(Token));
    int tokenIndex = 0;

    for(int i = 0; i < index; i++){
        
        //RESERVED WORD && IDENTIFIER
        if(isalpha(lines[i])){
            int sIndex = 0;
            char check[200];

            //taking in reserved word or identifier into char array 
            while(i < index && isalpha(lines[i]) || i < index && isdigit(lines[i])){
                check[sIndex] = lines[i];
                sIndex++;
                i++;
                
            }
            i--;

            check[sIndex] = '\0';

            //Reserved words
            if(strcmp(check, "begin") == 0){
                tokenList[tokenIndex].tokenType = beginsym;
                tokenList[tokenIndex].lexeme = "begin";
                tokenIndex++;

            } else if(strcmp(check, "end") == 0){
                tokenList[tokenIndex].tokenType = endsym;
                tokenList[tokenIndex].lexeme = "end";
                tokenIndex++;

            }

            else if(strcmp(check, "if") == 0){
                tokenList[tokenIndex].tokenType = ifsym;
                tokenList[tokenIndex].lexeme = "if";
                tokenIndex++;

            }

            else if(strcmp(check, "fi") == 0){
                tokenList[tokenIndex].tokenType = fisym;
                tokenList[tokenIndex].lexeme = "fi";
                tokenIndex++;

            }

            else if(strcmp(check, "then") == 0){
                tokenList[tokenIndex].tokenType = thensym;
                tokenList[tokenIndex].lexeme = "then";
                tokenIndex++;

            }

            else if(strcmp(check, "while") == 0){
                tokenList[tokenIndex].tokenType = whilesym;
                tokenList[tokenIndex].lexeme = "while";
                tokenIndex++;

            }

            else if(strcmp(check, "do") == 0){
                tokenList[tokenIndex].tokenType = dosym;
                tokenList[tokenIndex].lexeme = "dosym";
                tokenIndex++;

            }

            else if(strcmp(check, "call") == 0){
                tokenList[tokenIndex].tokenType = callsym;
                tokenList[tokenIndex].lexeme = "call";
                tokenIndex++;

            }

            else if(strcmp(check, "const") == 0){
                tokenList[tokenIndex].tokenType = constsym;
                tokenList[tokenIndex].lexeme = "const";
                tokenIndex++;

            }

            else if(strcmp(check, "var") == 0){
                tokenList[tokenIndex].tokenType = varsym;
                tokenList[tokenIndex].lexeme = "var";
                tokenIndex++;

            }

            else if(strcmp(check, "procedure") == 0){
                tokenList[tokenIndex].tokenType = procsym;
                tokenList[tokenIndex].lexeme = "procedure";
                tokenIndex++;

            }

            else if(strcmp(check, "write") == 0){
                tokenList[tokenIndex].tokenType = writesym;
                tokenList[tokenIndex].lexeme = "write";
                tokenIndex++;

            }

            else if(strcmp(check, "read") == 0){
                tokenList[tokenIndex].tokenType = readsym;
                tokenList[tokenIndex].lexeme = "read";
                tokenIndex++;

            }

            else if(strcmp(check, "else") == 0){
                tokenList[tokenIndex].tokenType = elsesym;
                tokenList[tokenIndex].lexeme = "else";
                tokenIndex++;

            }

            else if(strcmp(check, "even") == 0){
                tokenList[tokenIndex].tokenType = evensym;
                tokenList[tokenIndex].lexeme = "even";
                tokenIndex++;

            }
            else{
                //Identifier length error check
                if (sIndex > MAXID) {
                    tokenList[tokenIndex].tokenType = skipsym;
                    tokenList[tokenIndex].lexeme = stringDupe(check);
                    tokenIndex++;
                }
                //Adding identifier
                else {
                    tokenList[tokenIndex].tokenType = identsym;
                    tokenList[tokenIndex].lexeme = stringDupe(check);
                    tokenIndex++;
                }
            }
        }
        //NUMBER TOKEN
        else if(isdigit(lines[i]) ){
            int dIndex = 0;
            char dcheck[200];


            while(i < index && isdigit(lines[i])){
                dcheck[dIndex] = lines[i];
                dIndex++;
                i++;
                
            }
            dcheck[dIndex] = '\0';

            // Check for number too long error
            if(dIndex > MAXNUM){
                // Assign error lexeme and token type
                tokenList[tokenIndex].tokenType = skipsym;
                tokenList[tokenIndex].lexeme = stringDupe(dcheck);
                tokenIndex++;
            }else{
                // Assign number lexeme and token type
                dcheck[dIndex] = '\0';
                tokenList[tokenIndex].tokenType = numbersym;
                tokenList[tokenIndex].lexeme = stringDupe(dcheck);
                tokenIndex++;
            }
            // Correct i to end of number
            i--;
            
        }
        //SPECIAl SYMBOLS
        else{
            
            char currentChar [1];
            currentChar[0] = lines[i];
            // Check for known special symbols and assign correct token type and lexeme
            if(lines[i] == '+'){
                tokenList[tokenIndex].tokenType = plussym;
                tokenList[tokenIndex].lexeme = "+";
                // Increase token index to take next token
                tokenIndex++;

            }
            
            else if(lines[i] == '-'){
                tokenList[tokenIndex].tokenType = minussym;
                tokenList[tokenIndex].lexeme = "-";
                tokenIndex++;

            }

            else if(lines[i] == '*'){
                tokenList[tokenIndex].tokenType = multsym;
                tokenList[tokenIndex].lexeme = "*";
                tokenIndex++;

            }
        
            else if(lines[i] == '/'){
                // Check for comments symbol
                if(lines[i+1] == '*'){
                    // increase i to skip over * symbol
                    i += 2;
                    while(lines[i] != '*' && lines[i+1] != '/'){
                        // skip over all information in between /* and */
                        i++; 
                    }
                    // make i end at / symbol so it will start at the next character when loop increments i
                    i++;
                }
                else{
                    // Otherwise, assign info for slash symbol
                    tokenList[tokenIndex].tokenType = slashsym;
                    tokenList[tokenIndex].lexeme = "/";
                    tokenIndex++;
                }
            }

            // Check if equal sign symbol is not a part of := symbol
            else if(lines[i] == '=' && lines[i-1] != ':'){
                tokenList[tokenIndex].tokenType = eqsym;
                tokenList[tokenIndex].lexeme = "=";
                tokenIndex++;

            }

            
            else if(lines[i] == '<'){
            // Check for potential multi-character symbols starting with <
                if(lines[i+1] ==  '>'){
                    tokenList[tokenIndex].tokenType = neqsym;
                    tokenList[tokenIndex].lexeme = "<>";
                    tokenIndex++;
                }else if(lines[i+1] ==  '='){
                    tokenList[tokenIndex].tokenType = leqsym;
                    tokenList[tokenIndex].lexeme = "<=";
                    tokenIndex++;
                }else{
                    tokenList[tokenIndex].tokenType = lessym;
                    tokenList[tokenIndex].lexeme = "<";
                    tokenIndex++;

                }
                

            }

            else if(lines[i]== '>'){
                // Check for potential multi-character symbols starting with >
                if(lines[i+1] ==  '='){
                    tokenList[tokenIndex].tokenType = geqsym;
                    tokenList[tokenIndex].lexeme = ">=";
                    tokenIndex++;
                }else{
                    tokenList[tokenIndex].tokenType = gtrsym;
                    tokenList[tokenIndex].lexeme = ">";
                    tokenIndex++;
                }
            }

            else if(lines[i] == '('){
                tokenList[tokenIndex].tokenType = lparentsym;
                tokenList[tokenIndex].lexeme = "(";
                tokenIndex++;

            }

            else if(lines[i] == ')'){
                tokenList[tokenIndex].tokenType = rparentsym;
                tokenList[tokenIndex].lexeme = ")";
                tokenIndex++;

            }

            else if(lines[i]== ',' ){
                tokenList[tokenIndex].tokenType = commasym;
                tokenList[tokenIndex].lexeme = ",";
                tokenIndex++;

            }

            else if(lines[i] == ';'){
                tokenList[tokenIndex].tokenType = semicolonsym;
                tokenList[tokenIndex].lexeme = ";";
                tokenIndex++;

            }

            else if(lines[i] == '.'){
                tokenList[tokenIndex].tokenType = periodsym;
                tokenList[tokenIndex].lexeme = ".";
                tokenIndex++;

            }
            // Check if := symbol
            else if(lines[i]== ':' && lines[i+1] ==  '='){
                tokenList[tokenIndex].tokenType = becomessym;
                tokenList[tokenIndex].lexeme = ":=";
                tokenIndex++;
                i++;

            }
            // checking for  invisible symbols so they aren't registered as invalid characters
            else if (!(lines[i] == ' ' || lines[i] == '\n' || lines[i] == '\t' || lines[i] == '\r')) {
                // Assign any remaining token as invalid
                tokenList[tokenIndex].tokenType = skipsym;
                tokenList[tokenIndex].lexeme = stringDupe(currentChar);
                tokenIndex++;
            }

        }
    }

    // print lexeme table and token list
    printLexemeTable(tokenList, tokenIndex);

    printTokenList(tokenList, tokenIndex);   
      
}