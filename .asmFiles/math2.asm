; Load test numbers
LDI R1 10
LDI R2 3
LDI R10 65535

; Test Multiplication (10 * 3 = 30)
MUL R3 R1 R2
STORE R10 R3 

; Test Division (10 / 3 = 3) -> Integer division drops the decimal!
DIV R4 R1 R2
STORE R10 R4

; Test Modulo (10 % 3 = 1) -> The remainder of 10 / 3
MOD R5 R1 R2
STORE R10 R5

HALT