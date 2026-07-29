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
    TOKEN_NEWLINE, //Newline to mark end of statement
    TOKEN_EOF, //End of file token 
    TOKEN_UNKNOWN, //Unlisted tokens
} TokenType;

//The Token
typedef struct {
    TokenType type; 
    char lexeme [64]; //the actual text of the token
} Token;

//AST - Abstract Syntax Token node types. What type of rule definition will commands require
typedef enum {
    AST_PROGRAM,
    AST_VAR_DECL,
    AST_ASSIGNMENT,
    AST_PRINT,
    AST_IF,
    AST_WHILE,
    AST_NUMBER,    //Just a raw number
    AST_VAR_REF,   //Looking up an existing variable (like 'x')
    AST_ADD,       //Addition
    AST_SUB        //Subtraction
} ASTNodeType;

/**
 * Defining an ASTNode, what it must contain and it's pointer to the next node (linked lists)
 */
typedef struct ASTNode {
    ASTNodeType type;
    char val_name[64];
    int int_value;

    struct ASTNode** children; 
    int child_count;
} ASTNode;

/**
 * Creation of a Node - what it needs
 */
ASTNode* create_node(ASTNodeType type){
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    node->child_count = 0;
    node->children = NULL;
    return node; 
}

/**
 * Adding a node to the tree (linked list via pointers)
 */
void add_child(ASTNode* parent, ASTNode* child){
    parent->child_count++;
    parent->children = realloc(parent->children, sizeof(ASTNode*)*parent->child_count);
    parent->children[parent->child_count-1] = child; 
}

//LEXER State
char* source_code;
int current_pos = 0; 

Token current_token; 

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
    while (peek() == ' ' || peek() == '\t' || peek() == '\r'){
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

        if (strcmp(token.lexeme, "int") == 0 || strcmp(token.lexeme, "if") == 0 || strcmp(token.lexeme, "while") == 0 || strcmp(token.lexeme, "print") == 0 || strcmp(token.lexeme, "and") == 0 || strcmp(token.lexeme, "or") == 0) {
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

    if(peek() == '\n'){
        token.lexeme[0] = advance();
        token.lexeme[1] = '\0';
        token.type = TOKEN_NEWLINE;
        return token;
    }

    //if it's a known symbol - there is no math other than addition currently
    if (peek() == '=' || peek() == ';' || peek() == '{' || peek() == '}' || peek() == '(' || peek() == ')' || peek() == '+' || peek() == '-' || peek() == '<' || peek() == '>') {
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

void eat(TokenType expected_type, const char* error_msg){
    
    if (current_token.type == expected_type){
        current_token = get_next_token();
    } else {
        printf("Syntax Error: %s. Found '%s' instead. \n", error_msg, current_token.lexeme);
        exit(1);
    }
}

ASTNode* parse_primary() {
    //If it's a number, make the node with the type and value. 
    if (current_token.type == TOKEN_NUMBER) {
        ASTNode* node = create_node(AST_NUMBER);
        node->int_value = atoi(current_token.lexeme);
        eat(TOKEN_NUMBER, "Expected number");
        return node;
    } 
    //Note that it's an identifier, make the node with the variable's value. 
    else if (current_token.type == TOKEN_IDENTIFIER) {
        ASTNode* node = create_node(AST_VAR_REF);
        strcpy(node->val_name, current_token.lexeme);
        eat(TOKEN_IDENTIFIER, "Expected identifier");
        return node;
    }
    
    printf("Syntax Error: Expected a number or variable, but found '%s'\n", current_token.lexeme);
    exit(1);
}

ASTNode* parse_expression() {
    //Grab the node. Move onto the next token
    ASTNode* left = parse_primary();

    //Check which symbol it is 
    if (current_token.type == TOKEN_SYMBOL && (strcmp(current_token.lexeme, "+") == 0 || strcmp(current_token.lexeme, "-") == 0)) {
        
        //Type of operator saved and move onto the next
        ASTNodeType op_type = (strcmp(current_token.lexeme, "+") == 0) ? AST_ADD : AST_SUB;
        eat(TOKEN_SYMBOL, "Expected math operator");

        //Making the math node with the operator type
        ASTNode* math_node = create_node(op_type);

        //Read the right side
        ASTNode* right = parse_primary();
        
        //add both children
        add_child(math_node, left);
        add_child(math_node, right);
        
        return math_node;
    }
    return left;
}

ASTNode* parse_var_decl(){
    ASTNode* node = create_node(AST_VAR_DECL);
    eat(TOKEN_KEYWORD, "Expected 'int'");

    strcpy(node->val_name, current_token.lexeme);
    eat(TOKEN_IDENTIFIER, "Expected variable name");

    if (strcmp(current_token.lexeme, "=") != 0){
        printf("Syntax Error: Expected '=' after variable name\n");
        exit(1);
    }
    eat(TOKEN_SYMBOL, "Expected '='");

    ASTNode* value_expr = parse_expression();
    add_child(node, value_expr);

    eat(TOKEN_NEWLINE, "Expected a newline to finish the statement.");

    return node;
}

ASTNode* parse_program(){
    ASTNode* root = create_node(AST_PROGRAM);
    current_token = get_next_token();

    while (current_token.type == TOKEN_KEYWORD && strcmp(current_token.lexeme, "int") == 0){
        ASTNode* decl = parse_var_decl();
        add_child(root, decl);

        while(current_token.type == TOKEN_NEWLINE){
            eat(TOKEN_NEWLINE, "Skipping blank lines");
        }
    }

    return root;

}

/**
 * Helper to print the tree of variables 
 */
void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    // Indent based on depth
    for(int i=0; i<depth; i++) printf("  ");
    
    if (node->type == AST_PROGRAM) printf("PROGRAM\n");
    else if (node->type == AST_VAR_DECL) printf("VAR_DECL: %s\n", node->val_name);
    else if (node->type == AST_NUMBER) printf("NUMBER: %d\n", node->int_value);
    else if (node->type == AST_VAR_REF) printf("VAR_REF: %s\n", node->val_name);
    else if (node->type == AST_ADD) printf("ADD (+)\n");
    else if (node->type == AST_SUB) printf("SUB (-)\n");
    
    for (int i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
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

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    source_code = malloc(length+1);
    fread(source_code, 1, length, file);
    source_code[length]  = '\0';
    fclose(file);

    printf("--- PARSING AST ---\n");
    ASTNode* program = parse_program();

    print_ast(program, 0);

    free(source_code);
    return 0;
}