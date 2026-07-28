#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

//Token Definitions
typedef enum {
    TOKEN_IDENTIFIER, //name of the token: x, y, myVar 
    TOKEN_NUMBER, //Raw numbers, 1, 21,35 
    TOKEN_KEYWORD, //the reserved words for the program
    TOKEN_SYMBOL, //Punctuations
    TOKEN_EOF, //End of file token 
    TOKEN_UNKNOWN, //Unlisted tokens
} TokenType;

//The Token
typedef struct {
    TokenType type; 
    char lexeme [64]; //the actual text of the token
} Token;

//LEXER State
char* source_code;
int current_pos = 0; 

//Helper to see the current character
char peek() {
    return source_code[current_pos];
}

//Helper to see the next character
char advance(){
    return source_code[current_pos++];
}

Token get_next_token(){
    Token token;
    token.lexeme[0] = '\0'; //In case of an error it is not gibberish

    //skip over blank spaces
    while (isspace(peek())){
        advance();
    }

    //Check if it the end of the file 
    if (peek() == '\0'){
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    //check if it is a letter
    if (isalpha(peek())){
        int i = 0; 

        //keep reading until it's no longer a letter or number
        while (isalnum(peek())){
            token.lexeme[i++] = advance();
        }
        token.lexeme[i] = '\0';

        if (strcmp(token.lexeme, "int") == 0 || strcmp(token.lexeme, "if") == 0 || strcmp(token.lexeme, "while") == 0 || strcmp(token.lexeme, "print") == 0) {
            token.type = TOKEN_KEYWORD;
        } else { //if it's not a keyword, it must be an identifier
            token.type = TOKEN_IDENTIFIER;
        }
        return token;
    }

    //check if it's a number 
    if(isdigit(peek())){
        int i = 0;
        
        //Copy the entire number over
        while (isdigit(peek())){
            token.lexeme[i++] = advance();
        }
        token.lexeme[i] = '\0';

        token.type = TOKEN_NUMBER;
        return token;
    }

    //if it's a known symbol - there is no math other than addition currently
    if (peek() == '=' || peek() == ';' || peek() == '{' || peek() == '}' || peek() == '(' || peek() == ')' || peek() == '+' || peek() == '<' || peek() == '>') {
        token.lexeme[0] = advance();
        token.lexeme[1] = '\0';
        token.type = TOKEN_SYMBOL;
        return token;
    }

    //Unknown
    token.lexeme[0] = advance();
    token.lexeme[1] = '\0';
    token.type = TOKEN_UNKNOWN;
    return token;
}

int main (int argc, char* argv[]){
    if (argc < 2){
        printf("Usage: %s <source.lang>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        printf("ERROR: Could not open %s\n", argv[1]);
        return 1; 
    }

    //Goes to the end of the file, then says how many bytes in it is
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    //Allocate the memory into the array, then copy the file contents into the source_code variable
    source_code = malloc(length+1);
    fread(source_code, 1, length, file);
    source_code[length] =  '\0';
    fclose(file);

    printf("Lexical Analysis\n");

    Token t; 
    do {

        t = get_next_token();
    
        if (t.type == TOKEN_KEYWORD)    printf("KEYWORD    : %s\n", t.lexeme);
        if (t.type == TOKEN_IDENTIFIER) printf("IDENTIFIER : %s\n", t.lexeme);
        if (t.type == TOKEN_NUMBER)     printf("NUMBER     : %s\n", t.lexeme);
        if (t.type == TOKEN_SYMBOL)     printf("SYMBOL     : %s\n", t.lexeme);
        if (t.type == TOKEN_UNKNOWN)    printf("UNKNOWN    : %s\n", t.lexeme);

    } while (t.type != TOKEN_EOF);

    //Release the memory
    free(source_code);
    return 0;
}
