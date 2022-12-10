; hello world for spincom

  org $100-3
  ds $100

  ld r0,hellomsg
  call printstr
  int 0

hellomsg:
  db "Hello, world!\n",0

printstr:
  ldb a,(r0)
  z: ret
  inc r0
  int 1
  jmp printstr
