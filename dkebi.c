#include <stdio.h>
#include <avr/io.h>


// void ebi_init( void ) - configure the EBI
//
// Xplained board has a 47LC16M4A2 16M x 4 (8 MB) SDRAM
// 8MB SDRAM will live at 000000-7FFFFF
// (xmega internal memories occupy 0000-3FFF, so 004000-7FFFFF is visible SDRAM)
//
// NOTE: as of version 4.3.4 of avr-gcc (2011-05-22), only 16-bit address space is supported on AVR
//       the c compiler can access only 0000-FFFF
void dk_ebi_init( void ) 
{
    PORTK.DIR = 0xFF;                           // set the three EBI ports as outputs
    PORTJ.DIR = 0xFF;                           // EBI module will automatically override pin direction for the data pins
    PORTH.DIR = 0xFF;

   
    EBI.CTRL = EBI_IFMODE_3PORT_gc;             // 3 port EBI (PORTH/PORTJ/PORTK)
    EBI.REFRESH = 0x03FF;
    EBI.INITDLY = 0x0100;
    EBI.SDRAMCTRLA = EBI_SDROW_bm | EBI_SDCOL_10BIT_gc;
    EBI.SDRAMCTRLB = EBI_MRDLY_1CLK_gc | EBI_ROWCYCDLY_1CLK_gc | EBI_RPDLY_1CLK_gc;
    EBI.SDRAMCTRLC = EBI_WRDLY_1CLK_gc | EBI_ESRDLY_1CLK_gc | EBI_ROWCOLDLY_1CLK_gc;
    
    EBI.CS3.CTRLA = (0xF << 2) | 0x3;           // 8M bytes, SDRAM (must use CS3 for SDRAM)
}

