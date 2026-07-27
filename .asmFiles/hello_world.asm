; Jump over the data to start executing code
LDI R4 MAIN
JMP R4

; --- DATA SECTION ---
HELLO_MSG:
.STRING "HELLO WORLD!"

; --- CODE SECTION ---
MAIN:
; R1 = Screen address (Memory Mapped Output)
LDI R1 65535

; R2 = Pointer to the current character in our string (Starts at HELLO_MSG)
LDI R2 HELLO_MSG

; R5 = The null terminator (0) for comparison
LDI R5 0

; R6 = To increment our pointer by 4 bytes (since each char is 32 bits)
LDI R6 4

PRINT_LOOP:
; Load the character at the current pointer address into R3
LOAD R3 R2

; Is the character 0? (Did we hit the Null Terminator?)
CMP R3 R5

LDI R4 END

JEQ R4            ; If it is 0, jump to END

; Otherwise, print the character to the screen
STORE R1 R3

; Move pointer to the next character (R2 = R2 + 4)
ADD R2 R2 R6

; Loop back up
LDI R4 PRINT_LOOP
JMP R4

END:
; Print a newline character (10) for a clean terminal
LDI R3 10
STORE R1 R3
HALT