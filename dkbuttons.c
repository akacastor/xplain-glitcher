#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "dkbuttons.h"


unsigned char dk_buttons=0xFF;     // used to remember what previous pin values were, to execut dkaction_buttonX() only on button press


void dkbuttons_init( void )
{
    PORTD.DIR = 0x00;
    PORTD.OUT = 0x3F;
    PORTD.INTCTRL = PORT_INT0LVL_LO_gc;
    PORTD.INT0MASK = 0x3F;              // button0-button5 = PD0-PD5
    PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    
    PORTR.DIR = 0x00;
    PORTR.OUT = 0x03;
    PORTR.INTCTRL = PORT_INT0LVL_LO_gc;
    PORTR.INT0MASK = 0x03;              // button6-button7 = PR0-PR1
    PORTR.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTR.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
}


ISR(PORTD_INT0_vect) 
{
    if( !(PORTD.IN & 1<<0) )
    {
        if( dk_buttons & 1<<0 )
            dkaction_button0( NULL );
    }
    if( !(PORTD.IN & 1<<1) )
    {
        if( dk_buttons & 1<<1 )
            dkaction_button1( NULL );
    }
    if( !(PORTD.IN & 1<<2) )
    {
        if( dk_buttons & 1<<2 )
            dkaction_button2( NULL );
    }
    if( !(PORTD.IN & 1<<3) )
    {
        if( dk_buttons & 1<<3 )
            dkaction_button3( NULL );
    }
    if( !(PORTD.IN & 1<<4) )
    {
        if( dk_buttons & 1<<4 )
            dkaction_button4( NULL );
    }
    if( !(PORTD.IN & 1<<5) )
    {
        if( dk_buttons & 1<<5 )
            dkaction_button5( NULL );
    }
        
    dk_buttons = (dk_buttons & 0xC0) | (PORTD.IN & 0x3F);
}


ISR(PORTR_INT0_vect) 
{
    if( !(PORTR.IN & 1<<0) )
    {
        if( dk_buttons & 1<<6 )
            dkaction_button6( NULL );
    }
    if( !(PORTR.IN & 1<<1) )
    {
        if( dk_buttons & 1<<7 )
            dkaction_button7( NULL );
    }

    dk_buttons = (dk_buttons & 0x3F) | ( (PORTR.IN<<6) & 0xC0);
}
