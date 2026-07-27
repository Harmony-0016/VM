LDI R1 65535 ;to give the ouput store 
LDI R2 65534 ;to give input keyboard


LOOP_START:
LOAD R3 R2
STORE R1 R3

LDI R4 LOOP_START
JMP R4

HALT