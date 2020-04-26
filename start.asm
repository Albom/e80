format ELF

public Start
public _hStack
public _KOL_PATH
public _KOL_PARAM
public __stack_chk_fail_local

extrn Memory
extrn hEnd
extrn kol_main

section ".text" executable

db "MENUET01"
dd 1, Start, hEnd, Memory, _hStack, _KOL_PARAM, _KOL_PATH

__stack_chk_fail_local:
ret

Start:

; heap init
mov	eax, 68
mov	ebx, 11
int	0x40

; main function
mov	eax, kol_main
call	eax

; exit
mov	eax, -1
int	0x40

section ".bss"

_KOL_PARAM rb 256
_KOL_PATH rb 256

rb 16*1024
_hStack:
