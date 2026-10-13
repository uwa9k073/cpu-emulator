.data
    Array1: DW 10 12, -7, 3, -1, 8, 12, -7, 3, -1, 8
    Array2: DW 10 12, -7, 3, -1, 8, 12, -7, 3, -1, 8
    Result: DW 0
.program
    LDA Array1
    STA R0
    LDA #Array1+1
    STA R1
    LDA #Array2+1
    STA R2
    LDA #0
    STA R3

Loop:
    LDA [R1]
    MUL [R2]
    ADD R3
    STA R3
    LDA R1
    ADD #1
    STA R1
    LDA R2
    ADD #1
    STA R2
    LDA R0
    SUB #1
    JNZ Loop

    LDA R3
    STA Result
    HLT
