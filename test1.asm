; hello world for spincom

  org $100-2
  dw $100

  ld r0,hellomsg
  call printstr
  int 0

hellomsg:
  db "Hello, world!\n",0

printstr:
  ldb a,(r0)
  bz printstr0
  inc r0
  int 1
  b printstr
printstr0:
  ret
