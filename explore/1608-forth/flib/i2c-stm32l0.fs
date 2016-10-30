\ hardware i2c driver - not working yet

$40005400 constant I2C1
     I2C1 $00 + constant I2C1-CR1
     I2C1 $04 + constant I2C1-CR2
\    I2C1 $08 + constant I2C1-OAR1
\    I2C1 $0C + constant I2C1-OAR2
     I2C1 $10 + constant I2C1-TIMINGR
\    I2C1 $14 + constant I2C1-TIMEOUTR
     I2C1 $18 + constant I2C1-ISR
     I2C1 $1C + constant I2C1-ICR
\    I2C1 $20 + constant I2C1-PXCR
     I2C1 $24 + constant I2C1-RXDR
     I2C1 $28 + constant I2C1-TXDR

: i2c? ( -- )
  I2C1
  cr ."       CR1 " dup @ hex. 4 +
          ."  CR2 " dup @ hex. 4 +
         ."  OAR1 " dup @ h.4 space 4 +
         ."  OAR2 " dup @ h.4 space 4 +
      ."  TIMINGR " dup @ hex. 4 +
  cr ."  TIMEOUTR " dup @ hex. 4 +
          ."  ISR " dup @ hex. 4 +
         ."   ICR " dup @ h.4 space 4 +
         ."  PXCR " dup @ h.2 space 4 +
       ."    RXDR " dup @ h.2 space 4 +
         ."  TXDR " dup @ h.2 space drop cr ;

: +i2c ( -- )  \ initialise I2C hardware
  OMODE-AF-OD PB6 io-mode!
  OMODE-AF-OD PB7 io-mode!
  21 bit RCC-APB1ENR bis!  \ set I2C1EN
  $00300619 I2C1-TIMINGR !
  0 bit I2C1-CR1 bis!  \ PE
;

: i2c-start ( -- )
  13 bit I2C1-CR2 bis! ;
: i2c-stop  ( -- )
  14 bit I2C1-CR2 bis! ;

: nak? ( -- f ) 4 bit I2C1-ISR bit@ 0<> ;

: >i2c ( b -- nak )  \ send one byte
  8 . dup h.2 space
  16 bit I2C1-CR2 bis!  \ set NBYTES to 1
\ 24 bit I2C1-CR2 bis!  \ RELOAD
  I2C1-TXDR h!
  begin I2C1-ISR @ 0 bit and until
  9 .
  nak? ;
: i2c> ( nak -- b )  \ read one byte
  15 bit I2C1-CR2 rot if bis! else bic! then
  5 . begin I2C1-ISR @ 2 bit and until 6 .
  I2C1-RXDR h@ ;

: i2c-rxtx ( addr rw -- f )
  0 bit I2C1-CR1 bic!  \ clear PE to reset line state
  0 bit I2C1-CR1 bic!  \ TODO is this delay needed?
  0 bit I2C1-CR1 bis!  \ set PE
\ $3F38 I2C1-ICR !  \ clear all flags
  9 lshift or shl $01012000  or I2C1-CR2 !
  begin 13 bit I2C1-CR2 bit@ 0= until
  nak? ;

: i2c-tx ( addr -- nak ) 0 i2c-rxtx ;  \ start device send
: i2c-rx ( addr -- nak ) 1 i2c-rxtx ;  \ start device receive

: i2c. ( -- )  \ scan and report all I2C devices on the bus
  128 0 do
    cr i h.2 ." :"
    16 0 do  space
      i j +  dup i2c-tx i2c-stop  if drop ." --" else h.2 then
    loop
  16 +loop ;
