
: cr 10 emit ;
: space 32 emit ;
: hex 16 base ! ;
: decimal 10 base ! ;
: 0= if 0 else -1 then ;
: 0<> 0= 0= ;
: = - 0= ;
: <> - 0<> ;
hex : >= - 8000 and 0= ; decimal
: > 1+ >= ;
: < >= 0= ;
: <= > 0= ;
: / /mod swap drop ;
: mod /mod drop ;
: cell 4 ;
: cells cell * ;
: cell+ cell + ;
: , here ! cell allot ;
: c, here ! 1 allot ;

: [,] ['] [here] execute ! cell ['] [allot] execute ; immediate compile-only
: [c,] ['] [here] execute ! 1 ['] [allot] execute ; immediate compile-only
\ : begin [here] >r ; immediate compile-only
\ : again 1 [c,] r> [here] - 1- [c,] ; immediate compile-only

