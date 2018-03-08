This is an STM32F103RC board, intended for use as GAL programmer.

Pinout should make it possible to support 16v8, 22v10, and 26v12.  
Two 2x14 headers, with all 51 I/O pins, as well as 5V/3.3V/GND.

               J2                              J3
          ------------                    ------------
        9  pb9 ^^ pb8  8             mosi  pa7 -- pa6  miso
        7 pb11    pb10 led            dac  pa4 ~? pa5  e-sense (also dac)
    15-28 pb13    pb12 15-28          osc pc15 -- pc14 osc
    15-28 pb15    pb14 15-28           28! pd2    pa15 a-nss
     1-14  pc1 -- pc0  1-14           i2c  pb6    pb7  i2c
     1-14  pc3 -- pc2  1-14             7! pb4    pb3  a-sck
     1-14  pc5 -- pc4  1-14          1-14  pa1 -- pa0  1-14
    15-28  pc7    pc6  15-28         1-14  pa3 -- pa2  1-14
    15-28  pc9    pc8  15-28          swd pa14    pa13 swd
    15-28 pc11    pc10 15-28           21! pb5 -- pb0 !10
    15-28  pa8 ^  pc12 15-28          usb pa12    pa11 usb
    15-28  pd0 x  pb2  boot            rx pa10    pa9  tx
            nc  x pd1  15-28         vppe pc13 -- pb1 !12
        -  gnd    3v3  -                -  gnd    5v   -

^ = pwm-able, x = rtc xtal (disabled), - = not 5V-tolerant
! = mosfet control, ~ = dac, ? = adc

ZIF socket connections:

     1       pc0            28 vcc  pb12  pd2!
     2 vpp   pa4~ pa5?      27      pb14 
     3       pc2            26      pc6  
     4       pc4            25      pc8  
     5       pc3            24      pc10 
     6       pc5            23      pc12 
     7 vcc   pb11 pb4!      22      pd1  
     8       pb8            21 gnd  pb13  pb5!
     9       pb9            20      pb15 
    10 gnd   pc1  pb0!      19      pc7  
    11       1-14           18      pc9  
    12 gnd   1-14 pb1!      17      pc11 
    13       1-14           16      pa8  
    14       1-14           15      pd0  

gnd = n-mosfet (3x), vcc = p-mosfet w/ pull-up to 5V on gate (2x)  
vpp = dac w/ power opa548 in 6x mode, vppe = opa548 output enable  
include jumper to switch vcc between 3.3V and 5V

Leaves unassigned pins for 1x uart, 1x spi, 1x i2c.