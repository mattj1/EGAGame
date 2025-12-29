; ===================================================================

; ===================================================================
; 
; tasm test.s
; 
.MODEL TPASCAL
.CODE

ENABLE_ALL_MASK MACRO
    mov dx, 3ceh
    mov ax, 0ff08h
    out dx, ax
ENDM

ENABLE_ALL_PLANES MACRO
    mov dx, 3c4h
    mov ax, 0ff02h
    out dx, ax
ENDM

PUBLIC EGA_Init
EGA_Init PROC FAR
    mov al, 0dh
    mov ah, 0
    int 10h
    ret

PUBLIC EGA_SetPlanes
EGA_SetPlanes PROC PASCAL FAR
    ARG planes_mask:BYTE

    mov dx, 3c4h
    mov al, 2
    mov ah, planes_mask
    out dx, ax

    ret



PUBLIC EGA_DrawTileFast

EGA_DrawTileFast PROC PASCAL FAR
    ARG x: WORD, y: WORD, src_seg: WORD, src_offs: WORD, draw_seg

    push ds
         
    mov di, y
    shl di, 8
    mov bx, y
    shl bx, 6
    add di, bx
    add di, x
    shl di, 1

    mov ds, src_seg
    mov si, src_offs

    mov ax, draw_seg
    mov es, ax

    mov bx, 38 

    ; Because this is a latched copy, we can't use movsw

REPT 15
    movsb
    movsb
    add di, bx
ENDM
    movsb
    movsb

    pop ds
    ret

draw_sprite_mask_fast_row macro num
    _draw_sprite_mask_fast_row_&num:
    mov al, es:[di]
    movsb
    add di, dx
endm


draw_sprite_mask_fast_row_jump_table:
draw_sprite_mask_fast_row_jump_table_entry macro num
    dw offset _draw_sprite_mask_fast_row_&num
endm
    counter = 1
REPT 64
    draw_sprite_mask_fast_row_jump_table_entry %counter
    counter = counter + 1
ENDM

PUBLIC EGA_DrawSpriteMaskFast2
EGA_DrawSpriteMaskFast2 PROC PASCAL FAR
    ARG src_seg:WORD, src_offs:WORD, src_skip:WORD, dest_seg:WORD, dest_offs:WORD, dest_rewind:WORD, num_cols:WORD, num_rows:WORD

    ; ENABLE_ALL_MASK
    ; ENABLE_ALL_PLANES

    push ds

    mov ds, src_seg
    mov si, src_offs

    mov ax, dest_seg
    mov es, ax
    mov di, dest_offs

    mov cx, num_cols
    
    sub num_rows, 1
    mov bx, num_rows
    shl bx, 1

    mov dx, 39
column_loop:
    jmp word ptr [draw_sprite_mask_fast_row_jump_table + bx]

    counter = 64
REPT 64
    draw_sprite_mask_fast_row %counter
    counter = counter - 1
ENDM

    add si, src_skip
    sub di, dest_rewind

    dec cx
    jz column_loop_end
    jmp column_loop
column_loop_end: 

    pop ds

    ret


END