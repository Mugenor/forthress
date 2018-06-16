: 2dup >r dup r> swap >r dup r> swap ;
: -rot swap >r swap  r> ; 
: <= 2dup < -rot = lor ;
: > <= not ;

: IMMEDIATE last_word @ cfa 1 - 1 swap c! ;

: repeat here ; IMMEDIATE 
: until ' 0branch  , , ; IMMEDIATE 


: if ' 0branch , here dup , ; IMMEDIATE
: else ' branch , here dup , here rot rot ! ; IMMEDIATE
: then here swap ! ; IMMEDIATE


: for ' >r , here ' dup , ' r@ , ' > , ' 0branch , here 0 , ' >r , ; IMMEDIATE 
: endfor ' r> , ' r> , ' lit , 1 , ' + , ' >r , ' branch , swap , here swap !  ' r> , ; IMMEDIATE

