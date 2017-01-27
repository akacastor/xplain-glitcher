Source files:
--------------
dkactiontable.S     Table of all supported action functions
dkclock.c           Clock configuration (Xmega oscillator)
dkclock.h
dkebi.c             EBI configuration (external memory - 8MB SDRAM on Xplained board)
dkebi.h
dkfile.c            File interface (used for stdio support and dkfopen() etc functions)
dkfile.h
dkfile_uart.c       UART file interface (used by terminal emulation)
dkfile_uart.h
dkinteractive.c     Interactive shell
dkinteractive.h
dkstring.c          String manipulation functions
dkstring.h
dktables.h          
dktables.S          Flash table manipulation
dkterm.c            Terminal emulation interface (uses dkfile.c file interface)
dkterm.h
dkvt100.c           VT-100 terminal emulation
dkvt100.h           
Makefile
xplainer.c          main() function
actions/beep.c      BEEP action - make a loud beep with the Xplained speaker
actions/button0.c   action executed when Button0 (SW0) pressed
actions/button1.c   action executed when Button1 (SW1) pressed
actions/button2.c   action executed when Button2 (SW2) pressed
actions/button3.c   action executed when Button3 (SW3) pressed
actions/button4.c   action executed when Button4 (SW4) pressed
actions/button5.c   action executed when Button5 (SW5) pressed
actions/button6.c   action executed when Button6 (SW6) pressed
actions/button7.c   action executed when Button7 (SW7) pressed
actions/disco.c     DISCO action
actions/help.c      HELP action
actions/hotbutton.c HOTBUTTON action - set commands performed when buttons (SW0-SW7) pressed
actions/hotkey.c    HOTKEY action - set commands peformed when hotkeys (F1-F12) pressed
actions/led.c       LED action - turn on/off LED0-LED8
actions/peek.c      PEEK action - read byte from a memory address
actions/poke.c      POKE action - write byte to a memory address

Compiled files:
----------------
xplainer.eep        Generated EEPROM contents for ATxmega128A1
xplainer.hex        Generated FLASH contents for ATxmega128A1

In progress:
-------------
actions/temp.c      TEMP action - read temperature from NTC, using ADCB.0


Compiling:
-----------
Type 'make' build Makefile.
'make clean' will remove object and linked binaries.
'make prog' will make clean, make, and use avrdude to program the target device.


In the Makefile:
-----------------
DEFS           = -DF_CPU=32000000 -D_GLITCHER_VERSION_NUM=2

-DF_CPU=32000000 
The 32MHz RC oscillator is selected by dk_clock_init() in xplainer.c.
If oscillator settings are changed, this should be adjusted to match.

-D_DK_VERSION_NUM=2
The version number for the compiled project.


Adding an action:
------------------
1) create actions/new.c with "char *dkaction_new( char *line )" function
 - return value isn't actually used, but unprocessed remnant of line should be returned
2) add action to dkactiontable in dkactiontable.S
.string "new"
.word	dkaction_new
3) add actions/new.c to CSRC line of Makefile
4) add description and usage to actions/help.c


"Hello, world!" example action:
--------------------------------
1) All action program code is in actions/hello.c

2) Add to dkactiontable: (dkactiontable.S):
.string "hello"                         ; keyword "hello" - "Hello, world!" example
.word	dkaction_hello                  ; dkaction_hello() in actions/hello.c

3) Add actions/hello.c to CSRC = in Makefile

4) Add to printf_P() in dkaction_help() (actions/help.c):
"HELLO  (\"Hello, world!\" example action)\n"


Hardware usage: 
----------------
(as per Xmega-A1 Xplained board (blue PCB), 2011-05-22)

EBI (SDRAM - 3-port EBI)
 PORTK.DIR = 0xFF;
 PORTJ.DIR = 0xFF;
 PORTH.DIR = 0xFF;

LEDs (LED0-LED7)
 PORTE.DIR = 0xFF;

buttons (SW0-SW7)
 PORTD 0-5 (inputs, pullup enabled)
 PORTR 0-2 (inputs, pullup enabled)

thermsistor (NTC)
 PORTB 0 (ADC)
 PORTB 3 [NTC_EN]

light sensor
 PORTB 1 (ADC)

speaker
 PORTB 2 (DAC)
 PORTQ 3 [AUDIO_SD]

dataflash (optional)
 SPIC PORTC 5-7
 PORTQ 2 [DATAFLASH_CS]

USB serial adapter (32UC3B1256)
 USARTC0  (PC2=RXD, PC3=TXD)
     128-byte buffer for incoming data (96-byte limit before flow control kicks in)
     flow control disabled (not supported by 32uc3)
     57600 8-n-1 comm to 32uc3

JTAG
 PORTB 4-7

J1 = PORTF (shared with board controller spi/board controller twi)
J2 = PORTA <adc0-7>
J3 = PORTD 0-5, PORTR 0-1  <gpio0-7> (shared with buttons)
J4 = PORTC (PC2/PC3 USARTC shared with board controller)



VPORT configs
 no vports needed/utilized


interrupts:
global interrupt enabled
PMIC = LOLVLEX
enabled interrupts:
 USARTC0_RXCINTLVL
 button presses


Interactive interface:
-----------------------
VT100 terminal emulation (cursor keys, insert/delete, function keys)

Lines are terminated with carriage return (\r).
On input, newlines (\n) are ignored.  
On output, newlines (\n) may be converted to carriage return (\r) 
- or expanded into CRLF (\r\n) sequence.

NOTE:  ctrl-Z works as pause!


Terminal software:
-------------------
Tested using minicom in Linux and PuTTY in Windows 7 using PJRC generic CDC ACM driver
Settings:
 - 115200 8-N-1
 - VT100 terminal emulation


