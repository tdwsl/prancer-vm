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

  call getnext
  inc r1
  inc r1
  inc r1
  ldb a,(r1)
  ld r2,a
  inc r1
  ld a,r1
  ld r0,a
  ld a,r2
  ld r1,a
  call printstr
  ;call findword
  int 0

printstr:
  ld a,(r1)
  int 1
  inc r1
  dec r0
  nz: jmp printstr
  ret

; find word
findword:
  ld r0,lastword
  ld a,(r0)
  ld r0,a
  ld r3,wordstr
  ldb a,(r3)
  inc r3
  ld r4,a  ; r3 = addr, r4 = len
findword0:
  ld a,(r0)
  ld r1,a
  inc r0
  inc r0
  inc r0
  ld a,(r0)
  ld r6,a
  inc r0
  ld a,r0
  ld r5,a
  call streq
  z: ret
  ld a,r0
  z: jmp findwordfail
  ld r0,a
  jmp findword0
findwordfail:
  cmp r3
  ret

; compare string r3 r4 with string r5 r6
streq:
  ld a,r4
  cmp r5
  nz: ret
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
  nz: ret
  inc r7
  inc r8
  dec r9
  nz: jmp streq0
  ret

; get next word
getnext:
  ld r13,wordstr+1
  ld r12,33
getnext0:
  call getnextc
  cmp r12
  nc: jmp getnext1
  ldb (r13),a
  inc r13
  jmp getnext0
getnext1:
  ld r0,a
  ld a,r13
  ld r13,wordstr+1
  sub r13
  z: jmp getnextz
  ldb (r13),a
  ret
getnextz:
  ld a,r0
  or r0
  z: jmp getnexteof
  jmp getnext0
getnexteof:
  int 0

; load next char into a
getnextc:
  int 2
  ret

; dictionary
wd_dup:
  dw 0
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

