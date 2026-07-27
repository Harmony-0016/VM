LDI R1 5    ; Our counter
LDI R2 0    ; Our target to compare against
LDI R3 1    ; The amount to subtract each loop
LDI R4 16   ; The memory address of the top of our loop (The SUB instruction below)

SUB R1 R1 R3   ; Subtract 1 from our counter
CMP R1 R2      ; Compare the counter to 0
JGT R4         ; If the counter is Greater Than 0, jump back to Address 12!

HALT