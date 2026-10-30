.program
    ; R0 = количество элементов
            LDA     Array1
            STA     R0
    ; R1 = адрес первого элемента Array1
            LDA     #Array1+2
            STA     R1
    ; R2 = адрес первого элемента Array2
            LDA     #Array2+2
            STA     R2
    ; R4:R3 = 32-битная сумма
    ; R3 = low, R4 = high
            LDA     #0
            STA     R3
            STA     R4
Loop:
    ; A = Array1[i]
            LDA     DW [R1]
            STA     R5
    ; A = Array2[i]
            LDA     DW [R2]
    ; H:A = Array2[i] * Array1[i]
            MUL     R5
    ; Сохраняем младшую часть произведения
            STA     R6
    ; R3 = R3 + product.low
            LDA     R3
            ADD     R6
            STA     R3
    ; R4 = R4 + product.high + carry
            LDA     R4
            ADC     H
            STA     R4
    ; R1 += 2
            LDA     R1
            ADD     #2
            STA     R1
    ; R2 += 2
            LDA     R2
            ADD     #2
            STA     R2
    ; R0--
            LDA     R0
            SUB     #1
            STA     R0
    ; если R0 != 0 повторяем
            JNZ     Loop
    ; Сохраняем 32-битный результат R4:R3
            LDA     R3
            STA     DW Result
            LDA     R4
            STA     DW Result+2
            HLT
.data
    Array1: DW 10, 1, 2, 3, 4, 5, 6, 7000, 8, 9, 10,
    Array2: DW 10, 1, 2, 3, 4, 5, 6, -7, 8, 9, 10,
    Result: DD 0
