.data
    Array1: DD 10, 12, -7, 3, -1, 8, 12, -7, 3, -1, 8 ; массив с 32 разрядными числами. первый элемент кол-во чисел в массиве дальше числа
    Array2: DD 10 ,12, -7, 3, -1, 8, 12, -7, 3, -1, 8 ; 
    Result: DD 0
.program
    LDA Array1          ; Acc = 10
    STA R0              ; R0 = Acc
    LDA #Array1+4       ; Acc = 0x04
    STA R1              ; R1 = Acc
    LDA #Array2+4       ; Acc = 0x2C
    STA R2              ; R2 = Acc
    LDA #0              ; Acc = 0
    STA R3              ; R3 = 0

Loop:   LDA [R1]        ; Acc = 12
        MUL [R2]        ; Acc = Acc * 12
        ADС R3          ; it 1: Acc = Acc + 0, it2: Acc = Acc + 144 ...
        STA R3          ; R3 = Acc (acc = 144)
        LDA R1          ; Acc = R1 (0x04)
        ADD #4          ; Acc = Acc + 1 (0x04 + 4)
        STA R1          ; R1 = Acc (0x08)
        LDA R2          ; Acc = R2 (0x2C)
        ADD #4          ; Acc = Acc + 4 (0x30)
        STA R2          ; R2 = Acc (0x30)
        LDA R0          ; Acc = 10
        SUB #1          ; Acc = Acc - 1 (9)
        STA R0          ; R0  = Acc (9)
        JNZ Loop        ; JMP if ACC !=0

    LDA R3              ; Acc = R3 (result sim)
    STA Result          ; Result = R3
    HLT                 ; stop
