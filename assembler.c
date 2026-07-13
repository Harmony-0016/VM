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
} Opcode;

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
    int line_number = 0; 

    printf("Assembling %s...\n", argv[1]);

    //while there are still lines
    while(fgets(line,sizeof(line),infile)){
        line_number++;

        //take out new line characters
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line) == 0) continue; //skip empty lines

        //Tokenize - split at spaces
        char* token = strtok(line, " ");
        if (!token) continue; //if all spaces
        
        uint32_t instruction = 0; 

        //Parse
        if (strcmp(token, "HALT") == 0) {
            instruction = (OP_HALT << 24);
        } 
        else if(strcmp(token, "ADD") == 0) {
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_ADD << 24 | (rA << 16) | (rB << 8) | rC);
        } 
        else if (strcmp(token, "SUB") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = (OP_SUB << 24 | (rA << 16) | (rB << 8) | rC);
        }
        else if (strcmp(token, "LOAD") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            instruction = (OP_LOAD << 24 | (rA << 16) | (rB << 8));
        }
        else if (strcmp(token, "STORE") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            instruction = (OP_STORE << 24 | (rA << 16) | (rB << 8));
        }
        else if (strcmp(token, "LDI") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint16_t imm = (uint16_t)atoi(strtok(NULL, " ,")); 
            instruction = (OP_LDI << 24) | (rA << 16) | (imm & 0xFFFF);
        }
        else if (strcmp(token, "JMP") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            instruction = ((OP_JMP << 24) | (rA << 16));
        }
        else if (strcmp(token, "JEQ") == 0){
            uint8_t rA = parse_register(strtok(NULL, " ,"));
            uint8_t rB = parse_register(strtok(NULL, " ,"));
            uint8_t rC = parse_register(strtok(NULL, " ,"));
            instruction = ((OP_JEQ << 24) | (rA << 16) | (rB << 8) | rC);
        }
        else {
            printf("Error on line %d: Unknown instruction '%s'\n", line_number, token);
            continue;
        }

        uint8_t bytes[4]; 
        bytes[0] = (instruction >> 24) & 0xFF;
        bytes[1] = (instruction >> 16) & 0xFF;
        bytes[2] = (instruction >> 8) & 0xFF;
        bytes[3] = instruction & 0xFF;

        fwrite(bytes, sizeof(uint8_t),4,outfile);
    }

    printf("Assembly complete. Binary written to %s\n", argv[2]);
    fclose(infile);
    fclose(outfile);
    return 0;

}