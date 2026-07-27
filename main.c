#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_MEMORY_SIZE 65536 //64 kilobytes of RAM
#define NUM_REGISTERS 32 //total registers will be 32


/**
 * Defining the instruction set - A vocublary of the CPU so that the vm executes properly. 
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
    OP_PUSH = 0x08, //Push register
    OP_POP = 0x09, //Pop stack to register
    OP_CMP = 0x0A, //Compare two registers 
    OP_JNE = 0x0B, //Jump if not equal
    OP_JGT = 0x0C, //Jump if greater than
    OP_JLT = 0x0D, //Jump if lesser than
    OP_CALL = 0x0E, //Saves the current position of the PC
    OP_RET = 0x0F, //Marks the end of a function - go back to the saved position
    OP_AND = 0x10,
    OP_OR = 0x11,
    OP_XOR = 0x12,
    OP_NOT = 0x13,
    OP_SHL = 0x14,
    OP_SHR = 0x15,
} Opcode;

/**
 * Defining a register and it's types - A form of storing data in the CPU so it's faster than RAM
 */
typedef enum {
    //General purpose to be used by temp data and math operators
    R0 = 0, R1,R2,R3,R4,R5,R6,R7, //First 8 general purpose registers
    SP = 29, //StackPointer - shows top of stack
    LR = 30, //Link Register - Where to return after a function call
    PC = 31 //Program Counter - The memory address of the next instruction
} Register;


/**
 * The Machine State - Core Structure
 */
typedef struct{
    uint32_t registers[NUM_REGISTERS]; //The registers, the processor of 32 integers 
    uint8_t memory[MAX_MEMORY_SIZE]; //Massive array of bytes for the memory
    bool isRunning; // Is it running
    
    //Flags
    bool flag_Z; //Zero flag
    bool flag_N; //Negative flag
} VirtualMachine;

/**
 * Boot up sequence of setting the memory and registers to 0
 */
void init_vm(VirtualMachine* vm){
    //Set all the registers to 0
    for (int i = 0; i < NUM_REGISTERS; i++){
        vm -> registers[i] = 0;
    }

    //Make all the memory to 0
    for (int i = 0; i < MAX_MEMORY_SIZE; i++){
        vm->memory[i] = 0;
    }

    //Start the machine
    vm->registers[PC] = 0; //Start execution at address 0
    vm->registers[SP] = MAX_MEMORY_SIZE-1; //The top of memory is total - 1 
    vm->isRunning = true; //on 
}

/**
 * The main loop of the CPU. It will fetch, decode, and execute commands
 */
void run_vm(VirtualMachine* vm){
    printf("Starting Execution...\n");

    //loop until running is turned to off. 
    while(vm->isRunning){
        //FETCHING
        uint32_t pc = vm->registers[PC];
        //Combining the bytes from memory into a 32-bit instruction
        uint32_t instruction = (vm->memory[pc] << 24 | vm->memory[pc+1] << 16 | vm->memory[pc+2] << 8 | vm->memory[pc+3]);

        //increments to the next instruction
        vm->registers[PC] +=4;

        //DECODE 
        uint8_t opcode = (instruction >> 24) & 0xFF; //top byte
        uint8_t rA = (instruction >> 16) & 0xFF; //second byte
        uint8_t rB = (instruction >> 8) & 0xFF; //Third
        uint8_t rC = instruction & 0xFF; //bottom

        //EXECUTE
        switch (opcode){

            case OP_HALT:
                printf("OP_HALT encountered, shutting down\n");
                vm->isRunning = false;
                break; 

            case OP_ADD:
                vm->registers[rA] = vm->registers[rB] + vm->registers[rC];
                break;

            case OP_SUB:
                vm->registers[rA] = vm->registers[rB] - vm->registers[rC];
                break; 

            case OP_LOAD:
                {
                    uint32_t address = vm->registers[rB];

                    if (address == MAX_MEMORY_SIZE - 2){
                        vm->registers[rA] = (uint32_t)getchar();
                        break;
                    }

                    if (address >= MAX_MEMORY_SIZE-3){
                        printf("FATAL ERROR: Memory read out of bounds at address %d", address);
                        vm->isRunning = false;
                        break;
                    }

                    vm->registers[rA] = (vm->memory[address] << 24) | (vm->memory[address+1] << 16) | (vm->memory[address+2] << 8)| vm->memory[address+3];
                   
                }
                break;

            case OP_STORE:
            {
                uint32_t address = vm->registers[rA];
                uint32_t value_to_store = vm->registers[rB];

                if (address == MAX_MEMORY_SIZE - 1){
                     //Output memory onto the screen, can be used for the ui of the vm
                    printf("%c", (char)value_to_store);
                    break;
                }

                if (address >= MAX_MEMORY_SIZE -3){
                    printf("FATAL ERROR: Memory read out of bounds at address %d", address);
                    vm->isRunning = false;
                    break; 
                }

                //Shift right to compare vs 0xFF to determine the byte information.
                vm->memory[address] = (value_to_store >> 24) & 0xFF;
                vm->memory[address + 1] = (value_to_store >> 16) & 0xFF;
                vm->memory[address + 2] = (value_to_store >> 8) & 0xFF;
                vm->memory[address + 3] = value_to_store & 0xFF;
            }
                break;


            case OP_LDI:
                {
                    //isolating the bottom 16 bits (rb and rC)
                    uint16_t immediate = instruction & 0xFFFF;

                    vm->registers[rA] = immediate;
                    break;
                }


            case OP_CMP:
            //CMP R1 R2 
            //Always compare first beofre using a conditional jump
            {
                //Values
                uint32_t val1 = vm->registers[rA];
                uint32_t val2 = vm->registers[rB];

                uint32_t diff = (uint32_t)(val1-val2);

                //Changing flags if necessary
                vm->flag_Z = (diff == 0);
                vm->flag_N = (diff < 0);

            }
            break;


            case OP_JMP:
            //JMP rA 
            //Jumps rA being the desired address, it is not rB
                {
                    uint32_t target_address = vm->registers[rA];
                    
                    vm->registers[PC] = target_address;
                }
                break;


            case OP_JEQ:
            if (vm->flag_Z) {
                vm->registers[PC] = vm->registers[rA];
            } 
            break;

            case OP_JNE:
            //Jump if not equal
            if (!vm->flag_Z){
                vm->registers[PC] = vm->registers[rA];
            }

            case OP_JLT:
            //Jump if less than
            if (vm->flag_N){
                vm->registers[PC] = vm->registers[rA];
            }

            case OP_JGT:
            //Jump if greater than
            if(!vm->flag_N && !vm->flag_Z){
                vm->registers[PC] = vm->registers[rA];
            }
            break; 
            

            case OP_PUSH:
            //PUSH rA
            {
                uint32_t val = vm->registers[rA];
                
                vm->registers[SP] -= 4;
                uint32_t sp = vm->registers[SP];
                
                //Error Check
                if (sp >= MAX_MEMORY_SIZE - 3) {
                    printf("FATAL ERROR: Stack Overflow at address %d\n", sp);
                    vm->isRunning = false;
                    break;
                }

                vm->memory[sp]     = (val >> 24) & 0xFF;
                vm->memory[sp + 1] = (val >> 16) & 0xFF;
                vm->memory[sp + 2] = (val >> 8)  & 0xFF;
                vm->memory[sp + 3] = (val)       & 0xFF;

            }
            break;


            case OP_POP: 
            //POP rA
            {
                uint32_t sp = vm->registers[SP];

                //Error check
                if (sp >= MAX_MEMORY_SIZE - 3) {
                    printf("FATAL ERROR: Stack Underflow (Popped an empty stack!)\n");
                    vm->isRunning = false;
                    break;
                }

                vm->registers[rA] = (vm->memory[sp]     << 24) |
                                    (vm->memory[sp + 1] << 16) |
                                    (vm->memory[sp + 2] << 8)  |
                                    (vm->memory[sp + 3]);
                vm->registers[SP] += 4;
            }
            break;

            case OP_CALL:
            //CALL rA
            {
                //Save current position, jump to rA
                vm->registers[LR] = vm->registers[PC];
                vm->registers[PC] = vm->registers[rA];
            }
            break; 

            case OP_RET:
            //RET
            {
                //Go back to the previous position
                vm->registers[PC] = vm->registers[LR];
            }
            break;

            case OP_AND:
            //AND rA rB rC - Works when the bits align
            {
                vm->registers[rA] = vm->registers[rB] & vm->registers[rC];
            }
            break;

            case OP_OR:
            //OR rA rB rC - Works when a singular bit has a 1
            {
                vm->registers[rA] = vm->registers[rB] | vm->registers[rC];
            }
            break;
            
            case OP_XOR:
            //XOR rA rB rC - One or the other has a 1 in that bit column
            {
                vm->registers[rA] = vm->registers[rB] ^ vm->registers[rC];
            }
            break;

            case OP_NOT:
            //NOT rA rB
            {
                vm->registers[rA] = ~vm->registers[rB];
            }
            break;

            case OP_SHL:
            //SHL rA rB rC
            {
                vm->registers[rA] = vm->registers[rB] << vm->registers[rC];
            }
            break;


            case OP_SHR:
            //SHR rA rB rC
            {
                vm->registers[rA] = vm->registers[rB] >> vm->registers[rC];
            }
            break;

            default:
                printf("ERROR: No operator for command\n");
                vm->isRunning = false;
                break; 
        }

    }
}

/**
 * Determine if I can load the file requested 
 */
bool load_program(VirtualMachine* vm, const char* filename){
    FILE* file = fopen(filename, "rb");

    if (!file) {
        printf("Error could not open binary file: %s\n", filename);
        return false;
    }

    // Determine the file size
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    rewind(file);

    if (file_size > MAX_MEMORY_SIZE) {
        printf("Error: Program too large for VM memory!\n");
        fclose(file);
        return false;
    }

    // Read file directly into VM memory
    size_t bytes_read = fread(vm->memory, sizeof(uint8_t), file_size, file);
    fclose(file);

    printf("Successfully loaded %zu bytes into memory.\n", bytes_read);
    return true;

}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <program.bin>\n", argv[0]);
        return 1;
    }

    VirtualMachine vm;
    init_vm(&vm);

    if (!load_program(&vm, argv[1])) {
        return 1;
    }

    run_vm(&vm);

    return 0;
}
