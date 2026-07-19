#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <stdint.h>

/**
 * Pulled commands from the main.c Opcode enum
 */
typedef enum {
    OP_HALT = 0x00, //Stop the VM
    OP_ADD = 0x01, //Adds
    OP_SUB = 0x02, //Subtracts
    OP_LOAD = 0x03, //Memory to Register
    OP_STORE  = 0x04, //Register to Memory
    OP_LDI = 0x05, //Loading immediates
    OP_JMP = 0x06, //Jump to the address - unconditional
    OP_JEQ = 0x07, //Jump if equal 
    OP_PUSH = 0x08,
    OP_POP = 0x09,
    OP_CMP = 0x0A,
    OP_JNE = 0x0B,
    OP_JLT = 0x0C,
    OP_JGT = 0x0D,
} Opcode;

typedef struct {
    char name[64]; 
    uint32_t address;
} Symbol;

Symbol symbol_table[100]; 
int symbol_count = 0;

/**
 * Change the string into an integer
 */
uint8_t parse_register(char* reg_str){
    return (uint8_t) atoi(reg_str + 1);
}

int main(int argc, char* argv[]){

    //Ensure that all files are listed.
    if (argc < 3){
        printf("Usage: %s <input.asm> <output.bin>\n", argv[0]);
        return 1;
    }

    FILE* infile = fopen(argv[1], "r");
    if (!infile){
        printf("Error: Could not open %s\n", argv[1]);
        return 2; 
    }

    FILE* outfile = fopen(argv[2], "wb");
    if (!outfile) {
        printf("Error: Could not open %s\n", argv[2]);
        fclose(infile);
        return 1;
    }

    //
    char line[256]; 

    //PASS 1 -- Finding labels and record addresses 
    uint32_t current_address = 0;

    while (fgets(line, sizeof(line), infile)){
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue;

        char* token = strtok(line, " ,");
        if (!token) continue;

        int len = strlen(token);
        if (token[len-1] == ':'){
            token[len-1] = '\0';
            strcpy(symbol_table[symbol_count].name, token);
            symbol_table[symbol_count].address = current_address;
            symbol_count++;
            continue;
        }

        current_address +=4 ; 
    }

    rewind(infile);
    
    int line_number = 0; 
    printf("Assembling %s...\n", argv[1]);

    //while there are still lines
    while (fgets(line, sizeof(line), infile)) {
    line_number++;
    line[strcspn(line, "\r\n")] = 0;
    if (strlen(line) == 0) continue;

    char* token = strtok(line, " ,");
    if (!token) continue;

    // Skip labels on pass 2, we already processed them
    if (token[strlen(token) - 1] == ':') continue;

    uint32_t instruction = 0;

    if (strcmp(token, "HALT") == 0) {
        instruction = (OP_HALT << 24);
    } 
    else if (strcmp(token, "ADD") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        uint8_t rB = parse_register(strtok(NULL, " ,"));
        uint8_t rC = parse_register(strtok(NULL, " ,"));
        instruction = (OP_ADD << 24) | (rA << 16) | (rB << 8) | rC;
    }
    else if (strcmp(token, "SUB") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        uint8_t rB = parse_register(strtok(NULL, " ,"));
        uint8_t rC = parse_register(strtok(NULL, " ,"));
        instruction = (OP_SUB << 24) | (rA << 16) | (rB << 8) | rC;
    }
    else if (strcmp(token, "LOAD") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        uint8_t rB = parse_register(strtok(NULL, " ,"));
        instruction = (OP_LOAD << 24) | (rA << 16) | (rB << 8);
    }
    else if (strcmp(token, "STORE") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        uint8_t rB = parse_register(strtok(NULL, " ,"));
        instruction = (OP_STORE << 24) | (rA << 16) | (rB << 8);
    }
    else if (strcmp(token, "LDI") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        char* val_str = strtok(NULL, " ,");
        uint16_t imm = 0;
        int found = 0;

        // NEW: Check if val_str is a label in our cheat sheet
        for (int i = 0; i < symbol_count; i++) {
            if (strcmp(val_str, symbol_table[i].name) == 0) {
                imm = symbol_table[i].address;
                found = 1;
                break;
            }
        }
        
        // If it wasn't a label, just convert the raw number like normal
        if (!found) {
            imm = (uint16_t)atoi(val_str);
        }

        instruction = (OP_LDI << 24) | (rA << 16) | (imm & 0xFFFF);
    }
    else if (strcmp(token, "JMP") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        instruction = (OP_JMP << 24) | (rA << 16);
    }
    else if (strcmp(token, "CMP") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        uint8_t rB = parse_register(strtok(NULL, " ,"));
        instruction = (OP_CMP << 24) | (rA << 16) | (rB << 8);
    }
    else if (strcmp(token, "JEQ") == 0 || strcmp(token, "JNE") == 0 || 
             strcmp(token, "JLT") == 0 || strcmp(token, "JGT") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        if (strcmp(token, "JEQ") == 0) instruction = (OP_JEQ << 24) | (rA << 16);
        if (strcmp(token, "JNE") == 0) instruction = (OP_JNE << 24) | (rA << 16);
        if (strcmp(token, "JLT") == 0) instruction = (OP_JLT << 24) | (rA << 16);
        if (strcmp(token, "JGT") == 0) instruction = (OP_JGT << 24) | (rA << 16);
    }
    else if (strcmp(token, "PUSH") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        instruction = (OP_PUSH << 24) | (rA << 16);
    }
    else if (strcmp(token, "POP") == 0) {
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        instruction = (OP_POP << 24) | (rA << 16);
    }
    else {
        printf("Error on line %d: Unknown instruction '%s'\n", line_number, token);
        continue;
    }

    uint8_t bytes[4];
    bytes[0] = (instruction >> 24) & 0xFF;
    bytes[1] = (instruction >> 16) & 0xFF;
    bytes[2] = (instruction >> 8)  & 0xFF;
    bytes[3] = (instruction)       & 0xFF;

    fwrite(bytes, sizeof(uint8_t), 4, outfile);
}

printf("Assembly complete. Binary written to %s\n", argv[2]);

fclose(infile);
fclose(outfile);
return 0;


}