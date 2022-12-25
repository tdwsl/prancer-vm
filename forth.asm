; attempt at forth interpreter

  org $0100-2
  dw $0100

  ; entry point

  ; set up stack and dict
  ld r15,stack
  ld a,10
  ld r0,baseval
  ld (r0),a
  ld r0,lastword
  ld a,deflastword
  ld (r0),a
  ld r0,nextword
  ld a,defnextword
  ld (r0),a

  ld a,12
  ld (r15),a
  inc r15
  inc r15
  ld a,2
  ld (r15),a
  inc r15
  inc r15
  call wd_sub+2+2+1
  call wd_emit+2+5+1
  ;int 0

l0:
  call getnext
  call runtoken
  b l0

runtoken:
  call findword
  bnz runtokeni
  inc r0
  inc r0
  inc r0
  ldb a,(r0)
  inc r0
  add r0
  push
  ret
runtokeni:
  call tointeger
  bnz runtokenq
  ld a,r0
  ld (r15),a
  inc r15
  inc r15
  ret
runtokenq:
  ld r0,wordstr
  ldb a,(r0)
  ld r1,a
  inc r0
  call printstr
  ld r0,runtokenerrmsg
  ldb a,(r0)
  ld r1,a
  inc r0
  call printstr
  ret
runtokenerrmsg:
  db 3," ?\n"

; convert wordstr to integer in r0
tointeger:
  ld r1,wordstr
  ldb a,(r1)
  ld r2,a
  inc r1
  ldb a,(r1)
  ld r2,"-"
  cmp r2
  bnz tointegernm
  inc r1
  dec r2
  bz tointegerfail
tointegernm:
tointeger0:
tointegerfail:
  inc r1
  ret

; r0 = addr, r1 = len
printstr:
  ldb a,(r0)
  int 1
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
  cmp r3
findwordend:
  ret

; compare string r3 r4 with string r5 r6
streq:
  ld a,r4
  cmp r6
  bnz streqend
  ret
  ld a,r3
  ld r7,a
  ld a,r5
  ld r8,a
  ld a,r4
  ld r9,a
streq0:
  ldb a,(r7)
  ld r11,a
  ldb a,(r8)
  cmp r11
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
  int 2
  ret

; dictionary
wd_dup:
  dw 0       ; prev word addr
  db 0       ; flags
  db 3,"dup" ; len, name
  dec r15
  dec r15
  ld a,(r15)
  inc r15
  inc r15
  ld (r15),a
  inc r15
  inc r15
  ret
wd_swap:
  dw wd_dup
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
wd_add:
  dw wd_rpop
  db 0
  db 1,"+"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  dec r15
  dec r15
  ld a,(r15)
  add r0
  ld (r15),a
  inc r15
  inc r15
  ret
wd_sub:
  dw wd_add
  db 0
  db 1,"-"
  dec r15
  dec r15
  ld a,(r15)
  ld r0,a
  dec r15
  dec r15
  ld a,(r15)
  sub r0
  ld (r15),a
  inc r15
  inc r15
  ret
wd_emit:
deflastword:
  dw wd_sub
  db 0
  db 4,"emit"
  dec r15
  dec r15
  ld a,(r15)
  int 1
  ret

stack:
wordstr: equ stack+256
baseval: equ wordstr+256
lastword: equ baseval+2
nextword: equ lastword+2
defnextword: equ nextword+2

