LDI R1 65535 ;Output memory location

; Load 5 into R2
LDI R2 5

; Load the shift amount (1) into R2
LDI R3 1

;Shift left
SHL R4 R2 R3
STORE R1 R4

;XOR
XOR R5 R4 R4
STORE R1 R5

;Shift right
SHR R6 R4 R3 
STORE R1 R6

;not 
NOT R7 R6
STORE R1 R7

HALT