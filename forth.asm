; FORTH interpreter for Prancer VM - tdwsl 2022

  org $0100-2
  dw $0100

  i_putc: equ 1
  i_getc: equ 2

  ; entry point

  ; set up stack and dict
  ld r15,stack
  ld r0,baseval
  ld a,10
  ld (r0),a
  ld r0,lastword
  ld a,deflastword
  ld (r0),a
  ld r0,nextword
  ld a,defnextword
  ld (r0),a
  ld r0,compileflags
  ld a,0
  ldb (r0),a

l0:
  call getnext
  call runtoken
  b l0

runtoken:
  call findword
  bnz runtokeni

  inc r0
  inc r0
  ld a,r0
  ld r2,a
  inc r0
  ldb a,(r0)
  inc r0
  add r0
  ld r0,a
  ld r1,compileflags
  ldb a,(r1)
  bz runtokenout

  ldb a,(r2)
  bnz runtokenimm
  ld a,$0e ; call
  ldb (r14),a
  inc r14
  ld a,r0
  ld (r14),a
  inc r14
  inc r14
  ret
runtokenout:
  ldb a,(r2)
  bnz runtokencoerr
runtokenimm:
  ld a,r0
  ld r0,runtokenaddr+1
  ld (r0),a
runtokenaddr:
  call $ffff
  ret
runtokeni:
  call tointeger
  bnz runtokenq
  ld r1,compileflags
  ldb a,(r1)
  bz runtokeniout:
  ld a,$0f ; ld a
  ldb (r14),a
  inc r14
  ld a,r0
  ld (r14),a
  inc r14
  inc r14
  ld a,$5f ; ld (r15),a
  ldb (r14),a
  inc r14
  ld a,$bf ; inc r15
  ldb (r14),a
  inc r14
  ldb (r14),a
  inc r14
  ret
runtokeniout:
  ld a,r0
  ld (r15),a
  inc r15
  inc r15
  ret
runtokenq:
  ld r0,compileflags
  ld a,0
  ldb (r0),a
  ld r0,wordstr
  call printstrp
  ld r0,runtokenerrmsg
  call printstrp
  ret
runtokencoerr:
  ld r0,wordstr
  call printstrp
  ld r0,runtokencoerrmsg
  call printstrp
  ret
runtokenerrmsg:
  db 3," ?\n"
runtokencoerrmsg:
  db 17," is compile-only\n"

; mul r8 by r9
mul:
  ld a,r9
  push
  ld a,0
mul0:
  add r8
  dec r9
  bnz mul0
  ld r8,a
  pop
  ld r9,a
  ret

; div r8 by r9
div:
  ld a,r8
  ld r8,0
div0:
  cmp r9
  bnc div1
  sub r9
  inc r8
  b div0
div1:
  ld r9,a
  ret

; convert wordstr to integer in r0
tointeger:
  ld r1,wordstr
  ldb a,(r1)
  ld r2,a
  inc r1

  ld r3,0
  ldb a,(r1)
  ld r4,"-"
  cmp r4
  bnz tointegerpos
  inc r1
  dec r2
  bz tointegerfail
  inc r3
tointegerpos:

  ld r0,0
  ld r10,"a"-10
  ld r4,"0"
  ld r5,baseval
  ld a,(r5)
  ld r9,a
  ld r7,a
  ld r6,11
  cmp r6
  bc tointegera10
  add r4
  ld r5,a
  ld r7,a
  ld a,r4
  ld r6,a
  b tointeger0
tointegera10:
  ld r5,"9"+1
  ld r6,"a"
  ld r7,"a"-10
  add r7
  ld r7,a

tointeger0:
  ldb a,(r1)
  cmp r4
  bnc tointeger1
  cmp r5
  bc tointeger1
  sub r4
  b tointegernext
tointeger1:
  cmp r6
  bnc tointegerfail
  cmp r7
  bc tointegerfail
  sub r10
tointegernext:
  ld r11,a
  ld a,r0
  ld r8,a
  call mul
  ld a,r8
  ld r0,a
  ld a,r11
  add r0
  ld r0,a
  inc r1
  dec r2
  bnz tointeger0

  ld a,r3
  bz tointegerwaspos
  ld a,r0
  inv
  ld r0,a
  inc r0
tointegerwaspos:

  ld a,r0
  cmp r0
  ret

tointegerfail:
  inc r1
  ret

printstrp:
  ldb a,(r0)
  ld r1,a
  inc r0
; r0 = addr, r1 = len
printstr:
  ldb a,(r0)
  int i_putc
  inc r0
  dec r1
  bnz printstr
  ret

; find word
findword:
  ld r0,lastword
  ld a,(r0)
  ld r0,a
  ld r3,wordstr
  ldb a,(r3)
  ld r4,a  ; r3 = addr, r4 = len
  inc r3
findword0:
  ld a,r0
  ld r5,a
  inc r5
  inc r5
  inc r5
  ldb a,(r5)
  ld r6,a
  inc r5
  call streq
  bz findwordend
  ld a,(r0)
  bz findwordfail
  ld r0,a
  b findword0
findwordfail:
  inc r5
findwordend:
  ret

; compare string r3 r4 with string r5 r6
streq:
  ld a,r4
  cmp r6
  bnz streqend
  ld a,r3
  ld r7,a
  ld a,r5
  ld r8,a
  ld a,r4
  ld r9,a
streq0:
  ldb a,(r7)
  ld r10,a
  ldb a,(r8)
  cmp r10
  bnz streqend
  inc r7
  inc r8
  dec r9
  bnz streq0
streqend:
  ret

; get next word
getnext:
  ld r13,wordstr+1
  ld r12," "+1
  ld r11,"A"
  ld r10,"Z"+1
  ld r9,32
getnext0:
  call getnextc
  cmp r12
  bnc getnext1
  cmp r11
  bnc getnextnaz
  cmp r10
  bc getnextnaz
  add r9
getnextnaz:
  ldb (r13),a
  inc r13
  b getnext0
getnext1:
  ld r0,a
  ld a,r13
  ld r13,wordstr+1
  sub r13
  bz getnextz
  dec r13
  ldb (r13),a
  ret
getnextz:
  ld a,r0
  or r0
  bnc getnexteof
  b getnext0
getnexteof:
  int 0

; load next char into a
getnextc:
  int i_getc
  ret

;;; dictionary ;;;

wd_col:
  dw 0       ; prev word addr
  db 0       ; flags
  db 1,":"   ; len,name

  call getnext
  ld r0,compileflags
  ld a,1
  ldb (r0),a

  ld r0,nextword
  ld a,(r0)
  ld r14,a

  ld r0,lastword
  ld a,(r0)
  ld (r14),a
  inc r14
  inc r14
  ld a,0
  ldb (r14),a
  inc r14

  ld r0,wordstr
  ldb a,(r0)
  ld r1,a
  inc r0
  ldb (r14),a
  inc r14
wd_col0:
  ldb a,(r0)
  ldb (r14),a
  inc r0
  inc r14
  dec r1
  bnz wd_col0

  ret
wd_semi:
  dw wd_col
  db 1
  db 1,";"

  ld a,$0d ; ret
  ldb (r14),a
  inc r14

  ld r0,nextword
  ld a,(r0)
  ld r1,lastword
  ld (r1),a
  ld a,r14
  ld (r0),a

  ld r0,compileflags
  ld a,0
  ldb (r0),a

  ret
wd_dup:
  dw wd_semi
  db 0
  db 3,"dup"
  dec r15
  dec r15
  ld a,(r15)
  inc r15
  inc r15
  ld (r15),a
  inc r15
  inc r15
  ret
wd_drop:
  dw wd_dup
  db 0
  db 4,"drop"
  dec r15
  dec r15
  ret
wd_swap:
  dw wd_drop
  db 0
  db 4,"swap"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  dec r15
  dec r15
  ld a,(r15)
  ld r1,a
  ld a,r0
  ld (r15),a
  inc r15
  inc r15
  ld a,r1
  ld (r15),a
  inc r15
  inc r15
  ret
wd_over:
  dw wd_swap
  db 0
  db 4,"over"
  dec r15
  dec r15
  dec r15
  dec r15
  ld a,(r15)
  inc r15
  inc r15
  inc r15
  inc r15
  ld (r15),a
  inc r15
  inc r15
  ret
wd_rpush:
  dw wd_over
  db 0
  db 2,">r"
  dec r15
  dec r15
  pop
  ld r0,a
  ld a,(r15)
  push
  ld a,r0
  push
  ret
wd_rpop:
  dw wd_rpush
  db 0
  db 2,"r>"
  pop
  ld r0,a
  pop
  ld (r15),a
  inc r15
  inc r15
  ld a,r0
  push
  ret
wd_inc:
  dw wd_rpop
  db 0
  db 2,"1+"
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  ld r0,a
  inc r0
  ld a,r0
  ld (r1),a
  ret
wd_dec:
  dw wd_inc
  db 0
  db 2,"1-"
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  ld r0,a
  dec r0
  ld a,r0
  ld (r1),a
  ret
wd_add:
  dw wd_dec
  db 0
  db 1,"+"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  add r0
  ld (r1),a
  ret
wd_sub:
  dw wd_add
  db 0
  db 1,"-"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  sub r0
  ld (r1),a
  ret
wd_mul:
  dw wd_sub
  db 0
  db 1,"*"
  dec r15
  dec r15
  ld a,(r15)
  ld r9,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  ld r8,a
  call mul
  ld a,r8
  ld (r1),a
  ret
wd_divmod:
  dw wd_mul
  db 0
  db 4,"/mod"
  dec r15
  dec r15
  ld a,(r15)
  ld r9,a
  dec r15
  dec r15
  ld a,(r15)
  ld r8,a
  call div
  ld a,r9
  ld (r15),a
  inc r15
  inc r15
  ld a,r8
  ld (r15),a
  inc r15
  inc r15
  ret
wd_div:
  dw wd_divmod
  db 0
  db 1,"/"
  call wd_divmod
  call wd_swap
  dec r15
  dec r15
  ret
wd_mod:
  dw wd_div
  db 0
  db 3,"mod"
  call wd_divmod
  dec r15
  dec r15
  ret
wd_and:
  dw wd_mod
  db 0
  db 3,"and"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  and r0
  ld (r1),a
  ret
wd_or:
  dw wd_and
  db 0
  db 2,"or"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  or r0
  ld (r1),a
  ret
wd_xor:
  dw wd_or
  db 0
  db 3,"xor"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld a,r15
  ld r1,a
  dec r1
  dec r1
  ld a,(r1)
  xor r0
  ld (r1),a
  ret
wd_invert:
  dw wd_xor
  db 0
  db 6,"invert"
  ld a,r15
  ld r0,a
  dec r0
  dec r0
  ld a,(r0)
  inv
  ld (r0),a
  ret
wd_get:
  dw wd_invert
  db 0
  db 1,"@"
  ld a,r15
  ld r0,a
  dec r0
  dec r0
  ld a,(r0)
  ld r1,a
  ld a,(r1)
  ld (r0),a
  ret
wd_set:
  dw wd_get
  db 0
  db 1,"!"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  dec r15
  dec r15
  ld a,(r15)
  ld (r0),a
  ret
wd_getc:
  dw wd_set
  db 0
  db 2,"c@"
  ld a,r15
  ld r0,a
  dec r0
  dec r0
  ld a,(r0)
  ld r1,a
  ldb a,(r1)
  ld (r0),a
  ret
wd_setc:
  dw wd_getc
  db 0
  db 2,"c!"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  dec r15
  dec r15
  ld a,(r15)
  ldb (r0),a
  ret
wd_emit:
  dw wd_setc
  db 0
  db 4,"emit"
  dec r15
  dec r15
  ld a,(r15)
  int i_putc
  ret
wd_prnum:
  dw wd_emit
  db 0
  db 1,"."
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  ld r1,$8000
  and r1
  bz wd_prnumpos
  ld a,"-"
  int i_putc
  ld a,r0
  inv
  ld r0,a
  inc r0
wd_prnumpos:
  ld r1,wordstr
  ld r2,baseval
  ld a,(r2)
  ld r2,a
  ld a,r0
wd_prnum0:
  ld r8,a
  ld a,r2
  ld r9,a
  call div
  ld a,r9
  ldb (r1),a
  inc r1
  ld a,r8
  bnz wd_prnum0

  ld r2,10
  ld r3,"0"
  ld r4,"a"
  ld r5,wordstr
wd_prnum1:
  dec r1
  ld a,r1
  cmp r5
  bnc wd_prnumend
  ldb a,(r1)
  cmp r2
  bc wd_prnum2
  add r3
  b wd_prnum3
wd_prnum2:
  add r4
wd_prnum3:
  int i_putc
  b wd_prnum1

wd_prnumend:
  ld a," "
  int i_putc
  ret
wd_base:
  dw wd_prnum
  db 0
  db 4,"base"
  ld a,baseval
  ld (r15),a
  inc r15
  inc r15
  ret
deflastword:
wd_bye:
  dw wd_base
  db 0
  db 3,"bye"
  int 0
  ret

stack:
wordstr: equ stack+256
baseval: equ wordstr+256
lastword: equ baseval+2
nextword: equ lastword+2
compileflags: equ nextword+2
defnextword: equ compileflags+1+10

