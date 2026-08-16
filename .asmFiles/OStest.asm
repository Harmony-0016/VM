; === THE OPERATING SYSTEM TEST ===

; --- Test 1: Print a huge integer ---
; We want to use OS Command 1 (Print Integer)
LDI R1 1

; Load the number we want to print into R2
LDI R2 999999

; Wake up the OS!
SYSCALL

; --- Test 2: Generate a Random Number ---
; We want to use OS Command 2 (Random Number)
LDI R1 2

; Wake up the OS! (It will put the random number into R2)
SYSCALL

; Now, let's print that random number!
; Change R1 back to Command 1 (Print Integer)
LDI R1 1

; The random number is already sitting in R2, so just wake up the OS again!
SYSCALL

HALT