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
    OP_CALL = 0x0E,
    OP_RET = 0x0F,
    OP_AND = 0x10,
    OP_OR = 0x11,
    OP_XOR = 0x12,
    OP_NOT = 0x13,
    OP_SHL = 0x14,
    OP_SHR = 0x15,
    OP_MUL = 0x16,
    OP_DIV = 0x17,
    OP_MOD = 0x18,
    OP_DUMP = 0x19,
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

    char line[256]; 
    char orig_line[256];

    //PASS 1 -- Finding labels and record addresses 
    uint32_t current_address = 0;

    while (fgets(line, sizeof(line), infile)){
        line[strcspn(line, "\r\n")] = 0;    
        strcpy(orig_line, line);

        if (strlen(line) == 0) continue;

        char* token = strtok(line, " ,");

        if (!token) continue;

        if (token[0] == ';')continue;

        int len = strlen(token);
        if (token[len-1] == ':'){
            token[len-1] = '\0';
            strcpy(symbol_table[symbol_count].name, token);
            symbol_table[symbol_count].address = current_address;
            symbol_count++;
            continue;
        }

        //If it's a String
        if (strcmp(token, ".STRING") == 0){
            char* start = strchr(orig_line, '"');
            char* end = strrchr(orig_line, '"');
            //Ensure everything exists and they aren't equal
            if (end && start && start != end){
                int string_len = end - start;
                current_address += (string_len * 4);
            }
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

        strcpy(orig_line, line);
        
        if (strlen(line) == 0) continue;


        char* token = strtok(line, " ,");
        if (!token) continue;
        if (token[0] == ';') continue;

        // Skip labels on pass 2, we already processed them
        if (token[strlen(token) - 1] == ':') continue;

        if (strcmp(token, ".STRING") == 0){
            char* start = strchr(orig_line, '"');
            char* end = strrchr(orig_line, '"');
            if (start && end && start != end){
                for (int i = 1; i < (end-start); i++){
                    uint32_t char_val = (uint32_t)start[i];
                    uint8_t bytes[4] = { (char_val >> 24) & 0xFF, (char_val >> 16) & 0xFF, (char_val >> 8) & 0xFF, char_val & 0xFF };
                    fwrite(bytes, sizeof(uint8_t), 4, outfile);
                }
                uint8_t zeros[4] = {0,0,0,0};
                fwrite(zeros, sizeof(uint8_t), 4, outfile);
            }
            continue;
        }

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
    else if (strcmp(token, "CALL") == 0){
        uint8_t rA = parse_register(strtok(NULL, " ,"));
        instruction = (OP_CALL << 24 | (rA << 16));
    }
    else if (strcmp(token, "RET") == 0) {
        instruction = OP_RET << 24; 
    }
    else if (strcmp(token, "AND") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_AND << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "OR") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_OR << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "XOR") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_XOR << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "NOT") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            instruction = (OP_NOT << 24) | (rA << 16) | (rB << 8);
        }
        else if (strcmp(token, "SHL") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_SHL << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "SHR") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_SHR << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "MUL") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_MUL << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "DIV") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_DIV << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "MOD") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_MOD << 24) | (rA << 16) | (rB << 8) | rC;
        }
        else if (strcmp(token, "DUMP") == 0){
            instruction = OP_DUMP << 24; 
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