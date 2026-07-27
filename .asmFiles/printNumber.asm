; R1 = Screen Address
LDI R1 65535

; R2 = The number to print
LDI R2 34

;the divisor
LDI R3 10

; R4 = The ASCII offset (48) to turn raw numbers into characters
LDI R4 48

; Divide by 10 (34 / 10 = 3)
DIV R5 R2 R3

; Convert the raw 3 into the ASCII character '3' (3 + 48 = 51)
ADD R5 R5 R4

; Print the '3' to the screen
STORE R1 R5

; --- 2. ISOLATE AND PRINT THE ONES DIGIT ---

; Modulo by 10 (34 % 10 = 4)
MOD R6 R2 R3

; Convert the raw 4 into the ASCII character '4' (4 + 48 = 52)
ADD R6 R6 R4

; Print the '4' to the screen
STORE R1 R6

; --- 3. CLEANUP ---

; Print a newline character (ASCII 10) so the terminal looks clean
LDI R7 10
STORE R1 R7

HALT