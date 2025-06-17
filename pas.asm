;************************************
;*  pas.asm                          *
;*  由四元式文件生成的汇编文件       *
;************************************

data segment   
    a           DW ?
    b           DW ?
    h           DW ?
    k           DW ?
    m           DW ?
    n           DW ?
    x           DW ?
    y           DW ?
data ends      

code segment    
main proc far   
    assume cs:code,ds:data

start:
    push ds
    sub bx,bx
    push bx
    mov bx,data
    mov ds,bx
100:
    mov AX, a
    cmp AX, b
    jg  102
101:
    jmp 117
102:
    mov AX, m
    cmp AX, n
    jge 104
103:
    jmp 107
104:
    mov AX, a
    add AX, 1D
105:
    mov a, AX
106:
    jmp 112
107:
    mov AX, k
    cmp AX, h
    je  109
108:
    jmp 112
109:
    mov AX, x
    add AX, 2D
110:
    mov x, AX
111:
    jmp 107
112:
    mov AX, m
    add AX, yD
113:
    mul x
114:
    mov AX, n
    add AX, T4
115:
    mov m, AX
116:
    jmp 100
117:
    ret
main endp
code ends
    end start
