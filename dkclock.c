#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>


// setup the clock    

void dk_clock_init( void )
{
    OSC_CTRL |= OSC_RC32MEN_bm;         // enable 32 MHz RC oscillator
    
    while( !(OSC_STATUS & OSC_RC32MRDY_bm) )
        ;                               // wait for 32 MHz RC oscillator to be ready

	_delay_ms(1);
        
    CPU_CCP = CCP_IOREG_gc;             // set CPU_CCP to get access to CLK_CTRL
    CLK_CTRL = CLK_SCLKSEL_RC32M_gc;    // select 32 MHz internal rc osc.
    CLK_PSCTRL = 0;                     // no divisor
    
	_delay_ms(1);
}

