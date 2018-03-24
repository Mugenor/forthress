
( n -- )
( 1 1 2 3 5 8 13 ...   )
(  
  int x1=1, x2=1;
repeat...
  x3 = x1 + x2;
  x1 = x2;
  x2 = x3;
_______

)

: inc 1 + ;
: dec 1 - ; 
: inc-second swap inc swap ;

: <= 2dup < -rot = lor ;
: > <= not ;
: >= < not ;

: =0 0 = if 1 else 0 then ;
: >0 0 > if 1 else 0 then ;
: <0 0 < if 1 else 0 then ;
: >=0 0 >= if 1 else 0 then ;
: <=0 0 <= if 1 else 0 then ;

 ( a b c d - b c d a )
: forth-el
 >r swap >r swap r> -rot r> swap ;
: -forth-el
 -rot >r >r swap r> r> ;
: fifth-el
 >r forth-el r> swap ;
: -fifth-el
 -forth-el >r >r >r swap r> r> r> ;

: fib-n ( n ) 
  dup 0 < if ." Negative argument " else
 ( if n < 0 then error return )   
  dup 2 < ( n [n<2])  if ( n ) drop 1 
 ( if n < 2 then return 1 )   
     else
            >r
            1 1 
            r> 1 
            do
                swap over + 
            loop 
            swap drop 
			then 

then ; 

: even 2 % not ; 

: prime
dup <=0 if drop 0 else 
dup 3 <= if drop 1 else 
	1 swap dup 2 / 2 ( for [int i=2;i<n/2; ++i] ) ( 1 n n/2 1 )
	do
	dup r@ % not ( 1 n [n%i==0] )
	if 
		0 rot drop swap dup r> drop >r ( 0 n )
	then
	loop ( 	1 n )
	drop
	then
then ; 

: prime-alloc
	prime
	8 allot dup -rot ! ; 
		
: string-length 
	0 swap ( counter str ) 
	repeat
		inc-second
		dup c@ not
		inc-second
	until 
	drop
	dec ;
	
: string-concat 				( str1 str2 )
	dup string-length			( str1 str2 str2len )
	rot dup string-length 		( str2 str2len str1 str1len )
	rot over + inc 				( str2 str1 str1len fullLen )
	heap-alloc 					( str2 str1 str1len addr )
	dup forth-el string-copy 	( str2 str1Len addr )
	dup rot + rot string-copy 	( addr )
	;
	
: rad-n 
 dup =0 if drop 0 else 
	1 swap dup 2 / inc 2	( for [int i=2;i<n/2 + 1;++i]) ( 1 n n/2 2)
	do ( result n )
		dup r@ % not if 	( if n%i==0 )
						r@ prime if 
									swap r@ * swap 
								then
					then
	loop
	drop	
 then ;
	
	
