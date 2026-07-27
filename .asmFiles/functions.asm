; R1 = Screen address
LDI R1 65535

; Load the address of the function into R4
LDI R4 PRINT_A
CALL R4           ; The CPU jumps down to PRINT_A, and saves this spot!

; After the RET, the CPU will land right here!
LDI R4 PRINT_B
CALL R4

; Let's call PRINT_A one more time just to prove it's reusable
LDI R4 PRINT_A
CALL R4

; Print a newline and stop
LDI R2 10
STORE R1 R2
HALT

; === FUNCTIONS ===

PRINT_A:
; Load 'A' (ASCII 65) into R2 and print it
LDI R2 65
STORE R1 R2
RET               ; Teleport back to the caller!

PRINT_B:
; Load 'B' (ASCII 66) into R2 and print it
LDI R2 66
STORE R1 R2
RET               ; Teleport back to the caller!