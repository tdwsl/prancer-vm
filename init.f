
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
: \ 10 parse ;
: ( [char] ) parse ;

