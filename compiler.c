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
    // --- NEW NODE TYPES FOR MATH & LOGIC ---
    AST_NUMBER,    
    AST_VAR_REF,   
    AST_ADD,       
    AST_SUB,
    AST_LT,        // Less Than (<)
    AST_GT,        // Greater Than (>)
    AST_EQ,        // Equal To (==)
    AST_AND,       // Logical AND
    AST_OR,        // Logical OR
    AST_BLOCK      // A block of code inside { }
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
    if (!child) return; // Prevent adding NULL children
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

    // --- UPDATED SYMBOL LOGIC FOR == ---
    if (peek() == '=') {
        token.lexeme[0] = advance();
        if (peek() == '=') {
            token.lexeme[1] = advance();
            token.lexeme[2] = '\0';
        } else {
            token.lexeme[1] = '\0';
        }
        token.type = TOKEN_SYMBOL;
        return token;
    }

    //if it's a known symbol
    if (peek() == ';' || peek() == '{' || peek() == '}' || peek() == '(' || peek() == ')' || peek() == '+' || peek() == '-' || peek() == '<' || peek() == '>') {
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

// Forward declarations
ASTNode* parse_expression();
ASTNode* parse_statement();

// --- EXPRESSION PARSERS (Math & Logic) ---

ASTNode* parse_primary() {
    if (current_token.type == TOKEN_NUMBER) {
        ASTNode* node = create_node(AST_NUMBER);
        node->int_value = atoi(current_token.lexeme);
        eat(TOKEN_NUMBER, "Expected number");
        return node;
    } 
    else if (current_token.type == TOKEN_IDENTIFIER) {
        ASTNode* node = create_node(AST_VAR_REF);
        strcpy(node->val_name, current_token.lexeme);
        eat(TOKEN_IDENTIFIER, "Expected identifier");
        return node;
    }
    else if (current_token.type == TOKEN_SYMBOL && strcmp(current_token.lexeme, "(") == 0) {
        eat(TOKEN_SYMBOL, "Expected '('");
        ASTNode* expr = parse_expression();
        if (strcmp(current_token.lexeme, ")") != 0) {
            printf("Syntax Error: Expected closing ')'\n");
            exit(1);
        }
        eat(TOKEN_SYMBOL, "Expected ')'");
        return expr;
    }
    
    printf("Syntax Error: Expected a number, variable, or '(', but found '%s'\n", current_token.lexeme);
    exit(1);
}

// Handles * and / (If you add them later, they go here to handle Order of Operations)
ASTNode* parse_term() {
    return parse_primary();
}

// Handles + and -
ASTNode* parse_math() {
    ASTNode* left = parse_term();
    
    while (current_token.type == TOKEN_SYMBOL && (strcmp(current_token.lexeme, "+") == 0 || strcmp(current_token.lexeme, "-") == 0)) {
        ASTNodeType op_type = (strcmp(current_token.lexeme, "+") == 0) ? AST_ADD : AST_SUB;
        eat(TOKEN_SYMBOL, "Expected math operator");
        
        ASTNode* math_node = create_node(op_type);
        ASTNode* right = parse_term();
        
        add_child(math_node, left);
        add_child(math_node, right);
        left = math_node; 
    }
    return left;
}

// Handles <, >, ==
ASTNode* parse_comparison() {
    ASTNode* left = parse_math();
    
    if (current_token.type == TOKEN_SYMBOL && (strcmp(current_token.lexeme, "<") == 0 || strcmp(current_token.lexeme, ">") == 0 || strcmp(current_token.lexeme, "==") == 0)) {
        ASTNodeType op_type;
        if (strcmp(current_token.lexeme, "<") == 0) op_type = AST_LT;
        else if (strcmp(current_token.lexeme, ">") == 0) op_type = AST_GT;
        else op_type = AST_EQ;
        
        eat(TOKEN_SYMBOL, "Expected comparison operator");
        
        ASTNode* comp_node = create_node(op_type);
        ASTNode* right = parse_math();
        
        add_child(comp_node, left);
        add_child(comp_node, right);
        return comp_node;
    }
    return left;
}

// Handles AND, OR (This is the top level of our expression logic)
ASTNode* parse_expression() {
    ASTNode* left = parse_comparison();
    
    while (current_token.type == TOKEN_KEYWORD && (strcmp(current_token.lexeme, "and") == 0 || strcmp(current_token.lexeme, "or") == 0)) {
        ASTNodeType op_type = (strcmp(current_token.lexeme, "and") == 0) ? AST_AND : AST_OR;
        eat(TOKEN_KEYWORD, "Expected logical operator");
        
        ASTNode* logic_node = create_node(op_type);
        ASTNode* right = parse_comparison();
        
        add_child(logic_node, left);
        add_child(logic_node, right);
        left = logic_node;
    }
    
    return left;
}


// --- STATEMENT PARSERS ---

// Helper function to eat optional newlines
void skip_newlines() {
    while (current_token.type == TOKEN_NEWLINE) {
        eat(TOKEN_NEWLINE, "Skipping blank lines");
    }
}

// Parses { statement1 \n statement2 \n }
ASTNode* parse_block() {
    ASTNode* block = create_node(AST_BLOCK);
    
    skip_newlines(); // Ignore any Enter keys between the condition and the brace
    
    if (strcmp(current_token.lexeme, "{") != 0) {
        printf("Syntax Error: Expected '{' to start block. Found '%s' instead.\n", current_token.lexeme);
        exit(1);
    }
    eat(TOKEN_SYMBOL, "Expected '{'");
    
    skip_newlines();
    
    while (strcmp(current_token.lexeme, "}") != 0 && current_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement();
        if (stmt) {
            add_child(block, stmt);
        }
        
        // We expect a newline after every statement inside a block
        if (current_token.type == TOKEN_NEWLINE) {
            skip_newlines();
        } else if (strcmp(current_token.lexeme, "}") != 0) {
             printf("Syntax Error: Expected newline after statement in block, found '%s'\n", current_token.lexeme);
             exit(1);
        }
    }
    
    eat(TOKEN_SYMBOL, "Expected '}'");
    return block;
}

ASTNode* parse_if() {
    ASTNode* node = create_node(AST_IF);
    eat(TOKEN_KEYWORD, "Expected 'if'");
    
    // Parse the condition (e.g., x < 10)
    ASTNode* condition = parse_expression();
    add_child(node, condition);
    
    // Parse the body { ... }
    ASTNode* body = parse_block();
    add_child(node, body);
    
    return node;
}

ASTNode* parse_while() {
    ASTNode* node = create_node(AST_WHILE);
    eat(TOKEN_KEYWORD, "Expected 'while'");
    
    // Parse the condition
    ASTNode* condition = parse_expression();
    add_child(node, condition);
    
    // Parse the body { ... }
    ASTNode* body = parse_block();
    add_child(node, body);
    
    return node;
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

    return node;
}

ASTNode* parse_assignment() {
    ASTNode* node = create_node(AST_ASSIGNMENT);
    
    strcpy(node->val_name, current_token.lexeme);
    eat(TOKEN_IDENTIFIER, "Expected variable name for assignment");
    
    if (strcmp(current_token.lexeme, "=") != 0){
        printf("Syntax Error: Expected '=' after variable name\n");
        exit(1);
    }
    eat(TOKEN_SYMBOL, "Expected '='");
    
    ASTNode* expr = parse_expression();
    add_child(node, expr);
    
    return node;
}

ASTNode* parse_print() {
    ASTNode* node = create_node(AST_PRINT);
    eat(TOKEN_KEYWORD, "Expected 'print'");
    
    if (strcmp(current_token.lexeme, "(") != 0) { printf("Syntax Error: Expected '('\n"); exit(1); }
    eat(TOKEN_SYMBOL, "Expected '('");
    
    ASTNode* expr = parse_expression();
    add_child(node, expr);
    
    if (strcmp(current_token.lexeme, ")") != 0) { printf("Syntax Error: Expected ')'\n"); exit(1); }
    eat(TOKEN_SYMBOL, "Expected ')'");
    
    return node;
}


// --- CORE PARSER LOGIC ---

ASTNode* parse_statement() {
    if (current_token.type == TOKEN_KEYWORD) {
        if (strcmp(current_token.lexeme, "int") == 0) {
            return parse_var_decl();
        } else if (strcmp(current_token.lexeme, "print") == 0) {
            return parse_print();
        } else if (strcmp(current_token.lexeme, "if") == 0) {
            return parse_if();
        } else if (strcmp(current_token.lexeme, "while") == 0) {
            return parse_while();
        }
    } else if (current_token.type == TOKEN_IDENTIFIER) {
        return parse_assignment();
    }
    
    printf("Syntax Error: Unexpected token '%s' starting a statement.\n", current_token.lexeme);
    exit(1);
}

ASTNode* parse_program(){
    ASTNode* root = create_node(AST_PROGRAM);
    current_token = get_next_token();

    skip_newlines();

    while (current_token.type != TOKEN_EOF) {
        ASTNode* stmt = parse_statement();
        if (stmt) {
            add_child(root, stmt);
        }
        
        // After every top-level statement, we expect a newline or EOF
        if (current_token.type == TOKEN_NEWLINE) {
            skip_newlines();
        } else if (current_token.type != TOKEN_EOF) {
            printf("Syntax Error: Expected newline after statement, found '%s'\n", current_token.lexeme);
            exit(1);
        }
    }

    return root;
}

// -- CODE GENERATOR -- 

typedef struct {
    char name[64];
    int address;
} Symbol;

Symbol symbol_table[100];
int symbol_count = 0;
int next_free_address = 0; //Keeps track of vm ram addresses
int label_count = 0;  //Keeps labels unique 

int get_var_address(const char* name){
    for (int i = 0; i < symbol_count; i++){
        if (strcmp(symbol_table[i].name, name) == 0) return symbol_table[i].address;
    }
    printf("Semantic Error: Variable '%s' used before declaration!\n", name);
    exit(1);
}

/**
 * Does the math and puts it into the proper target register
 * Uses target_reg+1 for the right hand side to prevent overwriting itself
 */
void generate_expression(ASTNode* node, FILE* out, int target_reg){
    if (node->type == AST_NUMBER){
        fprintf(out, "LDI R%d %d\n", target_reg, node->int_value);
    }
    else if (node->type == AST_VAR_REF){
        int addr = get_var_address(node->val_name);
        fprintf(out, "LDI R28 %d\n", addr);
        fprintf(out, "LOAD R%d R28\n", target_reg);
    }
    else if (node->type == AST_ADD || node->type == AST_SUB) {
        generate_expression(node->children[0], out, target_reg);
        generate_expression(node->children[1], out, target_reg+1);

        if (node->type == AST_ADD) fprintf(out, "ADD R%d R%d R%d\n", target_reg, target_reg, target_reg + 1);
        if (node->type == AST_SUB) fprintf(out, "SUB R%d R%d R%d\n", target_reg, target_reg, target_reg + 1);
    }
    else if (node->type == AST_AND || node->type == AST_OR){
        generate_expression(node->children[0], out, target_reg);
        generate_expression(node->children[1], out, target_reg+1);

        if (node->type == AST_AND) fprintf(out, "AND R%d R%d R%d\n", target_reg, target_reg, target_reg + 1);
        if (node->type == AST_OR)  fprintf(out, "OR R%d R%d R%d\n", target_reg, target_reg, target_reg + 1);
    }
    else if (node->type == AST_LT || node->type == AST_GT || node->type == AST_EQ) {
        int l_id = label_count++;
        generate_expression(node->children[0], out, target_reg);
        generate_expression(node->children[1], out, target_reg + 1);
        
        fprintf(out, "CMP R%d R%d\n", target_reg, target_reg + 1);
        fprintf(out, "LDI R%d 1\n", target_reg);
        fprintf(out, "LDI R28 COND_TRUE_%d\n", l_id);
        
        if (node->type == AST_LT) fprintf(out, "JLT R28\n");
        if (node->type == AST_GT) fprintf(out, "JGT R28\n");
        if (node->type == AST_EQ) fprintf(out, "JEQ R28\n");
        
        fprintf(out, "LDI R%d 0\n", target_reg);
        fprintf(out, "COND_TRUE_%d:\n", l_id);
    }
}

void generate_statement(ASTNode* node, FILE* out) {
    if (node->type == AST_VAR_DECL) {
        generate_expression(node->children[0], out, 1);
        
        symbol_table[symbol_count].address = next_free_address;
        strcpy(symbol_table[symbol_count].name, node->val_name);
        symbol_count++;
        
        fprintf(out, "LDI R28 %d\n", next_free_address);
        fprintf(out, "STORE R28 R1\n\n");
        next_free_address += 4;
    }
    else if (node->type == AST_ASSIGNMENT) {
        generate_expression(node->children[0], out, 1);
        int addr = get_var_address(node->val_name);
        fprintf(out, "LDI R28 %d\n", addr);
        fprintf(out, "STORE R28 R1\n\n");
    }
    else if (node->type == AST_PRINT) {
        generate_expression(node->children[0], out, 1);
        fprintf(out, "LDI R28 65535\n");
        fprintf(out, "STORE R28 R1\n\n");
        fprintf(out, "LDI R1 10\n");
        fprintf(out, "STORE R28 R1\n\n");
    }
    else if (node->type == AST_BLOCK) {
        for(int i=0; i<node->child_count; i++) generate_statement(node->children[i], out);
    }
    else if (node->type == AST_IF) {
        int l_id = label_count++;
        generate_expression(node->children[0], out, 1); // R1 will hold 1 (true) or 0 (false)
        
        fprintf(out, "LDI R2 0\n");
        fprintf(out, "CMP R1 R2\n");
        fprintf(out, "LDI R28 IF_END_%d\n", l_id);
        fprintf(out, "JEQ R28\n\n"); // If R1 is 0 (False), jump to the end!
        
        generate_statement(node->children[1], out); // The Body
        
        fprintf(out, "IF_END_%d:\n\n", l_id);
    }
    else if (node->type == AST_WHILE) {
        int l_id = label_count++;
        fprintf(out, "WHILE_START_%d:\n", l_id);
        
        generate_expression(node->children[0], out, 1);
        fprintf(out, "LDI R2 0\n");
        fprintf(out, "CMP R1 R2\n");
        fprintf(out, "LDI R28 WHILE_END_%d\n", l_id);
        fprintf(out, "JEQ R28\n\n"); // If condition is false, break the loop
        
        generate_statement(node->children[1], out); // The Body
        
        fprintf(out, "LDI R28 WHILE_START_%d\n", l_id);
        fprintf(out, "JMP R28\n\n"); // Teleport back to the top
        
        fprintf(out, "WHILE_END_%d:\n\n", l_id);
    }
}

// --- HELPER FUNCTION TO PRINT THE TREE ---
void print_ast(ASTNode* node, int depth) {
    if (!node) return;
    
    for(int i=0; i<depth; i++) printf("  ");
    
    if (node->type == AST_PROGRAM) printf("PROGRAM\n");
    else if (node->type == AST_VAR_DECL) printf("VAR_DECL: %s =\n", node->val_name);
    else if (node->type == AST_ASSIGNMENT) printf("ASSIGNMENT: %s =\n", node->val_name);
    else if (node->type == AST_PRINT) printf("PRINT\n");
    else if (node->type == AST_IF) printf("IF\n");
    else if (node->type == AST_WHILE) printf("WHILE\n");
    else if (node->type == AST_BLOCK) printf("BLOCK {}\n");
    else if (node->type == AST_NUMBER) printf("NUMBER: %d\n", node->int_value);
    else if (node->type == AST_VAR_REF) printf("VAR_REF: %s\n", node->val_name);
    else if (node->type == AST_ADD) printf("ADD (+)\n");
    else if (node->type == AST_SUB) printf("SUB (-)\n");
    else if (node->type == AST_LT) printf("LESS THAN (<)\n");
    else if (node->type == AST_GT) printf("GREATER THAN (>)\n");
    else if (node->type == AST_EQ) printf("EQUAL TO (==)\n");
    else if (node->type == AST_AND) printf("LOGICAL AND\n");
    else if (node->type == AST_OR) printf("LOGICAL OR\n");
    
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
    size_t bytes_read = fread(source_code, 1, length, file);
    source_code[bytes_read] = '\0';
    fclose(file);

    printf("--- PARSING AST ---\n");
    ASTNode* program = parse_program();
    print_ast(program, 0);

    const char* out_name = (argc >= 3) ? argv[2] : "output.asm";
    
    FILE* out = fopen(out_name, "w");
    fprintf(out, "; === AUTO-GENERATED ASSEMBLY FROM COMPILER ===\n\n");
    for (int i = 0; i < program->child_count; i++) {
        generate_statement(program->children[i], out);
    }
    fprintf(out, "HALT\n");
    fclose(out);
    
    printf("\nSuccess! Generated assembly saved to %s\n", out_name);

    free(source_code);
    return 0;
}