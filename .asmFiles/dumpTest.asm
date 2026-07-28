; === DEBUGGER TEST ===

LDI R1 42
LDI R2 100

; Dump the state to see R1 and R2
DUMP

ADD R3 R1 R2

; Dump the state again to see the result in R3
DUMP

HALT