#include <avr/pgmspace.h>
#include <avr/cpufunc.h>


//--------------------------------------------------------------------
// clk for timer1 for target timeout is / 1024  (TC_CLKSEL_DIV1024_gc)
#define T_TIMEOUT_DIV   1024L

int t_timeout_ticks = F_CPU / T_TIMEOUT_DIV;        // 1 second @ 16 MHz / 1024 is 15625 ticks


// start target timeout, use global 't_timeout_ticks' as time when TIFR.OCF1A will be set to indicate timeout
void t_timeout_start(void)
{
    // stop the timer while configuring it
    TCC0.CTRLB = 0;

    // reset timer count to 0
    TCC0.CNT = 0;
	TCC0.PER = t_timeout_ticks;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
//	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;
	TCC0.INTCTRLA = 0;
	TCC0.INTFLAGS = 0x01;       // clear the OVFIF flag

    // start timer by selecting clock source
	TCC0.CTRLB = TC_WGMODE_NORMAL_gc;
}


void t_timeout_stop(void)
{
    // stop the timer while configuring it
    TCC0.CTRLB = 0;

	TCC0.INTFLAGS = 0x01;       // clear the OVFIF flag
}


void t_timeout_after_ms(int t_ms)
{
    t_timeout_ticks =  ((unsigned long)t_ms * (F_CPU/1000)) / T_TIMEOUT_DIV;
    t_timeout_start();
}


// returns 1 if t_timeout has expired
int t_timeout_check(void)
{
    if( TCC0.INTFLAGS  & 0x01 )
        return 1;
    else
        return 0;
}


void t_timeout_clear(void)
{
	TCC0.INTFLAGS = 0x01;       // clear the OVFIF flag
}

