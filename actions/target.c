//--------------------------------------------------------------------
#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "../dkterm.h"
#include "../dkstring.h"
#include "../hardware.h"
#include "../dkfile_uart.h"
#include "../timeout.h"


//--------------------------------------------------------------------

DKFILE *t_uart = NULL;


//--------------------------------------------------------------------

void t_open( void )
{
// target rst = low
    TRST_PORT.OUTCLR = (1<<TRST_BIT);
    TRST_PORT.DIRSET = (1<<TRST_BIT);              

// enable target vcc
    GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)
    GVCC_PORT.DIRSET = (1<<GVCC_SW);

    GVCC_PORT.OUTCLR = (1<<GVCC_EN);                  // set ENABLE low (4619 enabled)
    GVCC_PORT.DIRSET = (1<<GVCC_EN);


    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

// 10ms delay while rst low
    _delay_ms( 10 );
    
// target rst = high
    TRST_PORT.OUTSET = (1<<TRST_BIT);
    TRST_PORT.DIRSET = (1<<TRST_BIT);              


    return;
}

//--------------------------------------------------------------------

void t_close( void )
{
// target rst = low
    TRST_PORT.OUTCLR = (1<<TRST_BIT);
    TRST_PORT.DIRSET = (1<<TRST_BIT);              

// disable target vcc
    GVCC_PORT.OUTSET = (1<<GVCC_EN);                  // set ENABLE high (4619 disabled)
    GVCC_PORT.DIRSET = (1<<GVCC_EN);

    if( t_uart )
        dkfclose(t_uart);
    t_uart = NULL;


    return;
}


//--------------------------------------------------------------------
char *dkaction_pon( char *line )
{
    t_open();
    
    printf_P( PSTR("Target powered on and reset.\n") );
    

    return line;
}


//--------------------------------------------------------------------
char *dkaction_poff( char *line )
{
    t_close();
    
    printf_P( PSTR("Target powered off.\n") );
    

    return line;
}


//--------------------------------------------------------------------
// connect host (PC) and target device in passthru uart mode
char *dkaction_passthru( char *line )
{
    ((struct _dk_uart *)dkterm_interfaces->devfile->info)->_flag &= ~DKUART_FLAG_TERMINAL;

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );


    PORTD.DIR = 0x00;
    PORTD.OUT = 0x3F;
    PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    
    PORTR.DIR = 0x00;
    PORTR.OUT = 0x03;
    PORTR.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTR.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;

    PORTE.OUTSET = (1<<0);
    PORTE.OUTCLR = (1<<1);
    PORTE.OUTSET = (1<<2);
    PORTE.OUTSET = (1<<3);

    
    while( PORTD.IN & (1<<0) )
    {   // if a button is held down then break of out passthru mode        
    	if( !(PORTD.IN & (1<<2)) )
    	{
            // target rst = low
            TRST_PORT.OUTCLR = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);    	    
    	}
    	else
    	{
            // target rst = high
            TRST_PORT.OUTSET = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);
        }              

        if( !dkfeof(t_uart) )
            _putch(dkfgetc(t_uart));

        if(_kbhit())
            dkfputc(_getch(), t_uart);
    }
    
    ((struct _dk_uart *)dkterm_interfaces->devfile->info)->_flag |= DKUART_FLAG_TERMINAL;

    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_glitch( char *line )
{
    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    while( io_ctrl_char != DKKEY_CTRLC )
    {
        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
            _putch(dkfgetc(t_uart));


        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
        {
            if( _kbhit() && _getch() == 'r' )
            {
                // target rst = low
                TRST_PORT.OUTCLR = (1<<TRST_BIT);
                TRST_PORT.DIRSET = (1<<TRST_BIT);              

                // 10ms delay while rst low
                _delay_ms( 10 );
    
                // target rst = high
                TRST_PORT.OUTSET = (1<<TRST_BIT);
                TRST_PORT.DIRSET = (1<<TRST_BIT);              
            }
        }

        _delay_ms(1);            

        GVCC_PORT.OUTSET = (1<<GVCC_SW);        // 2.0 V / 0.2 V / 2-nops = some success
        _NOP();
        _NOP();     // 2 nop required to cause reset of glitch_vcc tied to gnd
        GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)


        _delay_ms(1);            

        GVCC_PORT.OUTSET = (1<<GVCC_SW);       
        _NOP();
        _NOP();     // 2 nop required to cause reset of glitch_vcc tied to gnd
        GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)


        _delay_ms(1);            

        GVCC_PORT.OUTSET = (1<<GVCC_SW);       
        _NOP();
        _NOP();     // 2 nop required to cause reset of glitch_vcc tied to gnd
        GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)


        _delay_ms(1);            

        GVCC_PORT.OUTSET = (1<<GVCC_SW);       
        _NOP();     // 2 nop required to cause reset of glitch_vcc tied to gnd
        _NOP();
        GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)


        _delay_ms(1);            

        GVCC_PORT.OUTSET = (1<<GVCC_SW);       
        _NOP();     // 2 nop required to cause reset of glitch_vcc tied to gnd
        _NOP();
        GVCC_PORT.OUTCLR = (1<<GVCC_SW);       // clear A and B (select X0/Y0 on 4619, or deactivate ULN2003)



        while( (!(PORTF.IN & (1<<1))) && io_ctrl_char != DKKEY_CTRLC )
        {
            if( _kbhit() && _getch() == 'r' )
            {
                // target rst = low
                TRST_PORT.OUTCLR = (1<<TRST_BIT);
                TRST_PORT.DIRSET = (1<<TRST_BIT);              

                // 10ms delay while rst low
                _delay_ms( 10 );
    
                // target rst = high
                TRST_PORT.OUTSET = (1<<TRST_BIT);
                TRST_PORT.DIRSET = (1<<TRST_BIT);              
            }
        }
    }
   
    
    return line;
}

//--------------------------------------------------------------------

uint8_t glitch_form[8] = {(1<<GVCC_SW), (1<<GVCC_SW), (1<<GVCC_SW), (1<<GVCC_SW), (1<<GVCC_SW), (1<<GVCC_SW), 0, 0};
uint8_t noglitch_form = 0x00;       // GVCC_EN = 0, GVCC_SW = 0

//--------------------------------------------------------------------

char *dkaction_setglitch( char *line )
{
    int i;
    char *token=NULL;    
    int waveform=1;    


    for( i=0; i<8 && line && line[0];  )
    {
        for( line = dkgettoken( line, token ); token[0] && i<8; token++ )
        {
            if( token[0] == '0' )
                glitch_form[i] |= (1<<GVCC_SW);
            else if( token[0] == '1' )
                glitch_form[i] &= ~(1<<GVCC_SW);
            else
                continue;
            i++;                
        }
    }

    
    if( i )     // if some of a waveform was entered, then fill the rest with noglitch_form
    {
        for( ; i<8; i++ )
            glitch_form[i] = noglitch_form;
    }


    printf_P( PSTR("\nVCC glitch waveform:\n\n") );


    printf_P( PSTR("  ") );   
    for( i=0; i<8; i++ )
    {
        if( glitch_form[i] & (1<<GVCC_SW) )
            printf_P( PSTR(" 0") );
        else
            printf_P( PSTR(" 1") );
    }
    printf_P( PSTR("\n") );


    printf_P( PSTR("__") );   
     
    for( i=0; i<8; i++ )
    {
        if( glitch_form[i] & (1<<GVCC_SW) )
        {
            printf_P( PSTR("  ") );
            waveform = 0;
        }
        else
        {
            if( !waveform )
                printf_P( PSTR(" ") );
            else
                printf_P( PSTR("_") );

            printf_P( PSTR("_") );
            waveform = 1;
        }
    }
    if( !waveform )
        printf_P( PSTR(" ") );
    else
        printf_P( PSTR("_") );
    printf_P( PSTR("__\n") );
    printf_P( PSTR("  ") );   
    waveform = 1;

    for( i=0; i<8; i++ )
    {
        if( glitch_form[i] & (1<<GVCC_SW) )
        {
            if( waveform )
                printf_P( PSTR("|") );
            else
                printf_P( PSTR("_") );
                
            printf_P( PSTR("_") );
            waveform = 0;
        }
        else
        {
            if( !waveform )
                printf_P( PSTR("|") );
            else
                printf_P( PSTR(" ") );

            printf_P( PSTR(" ") );
            waveform = 1;
        }
    }
    if( !waveform )
        printf_P( PSTR("|") );
    else
        printf_P( PSTR(" ") );
    printf_P( PSTR("\n") );    
    printf_P( PSTR("  ^               ^\n") );   

    printf_P( PSTR("  |---------------|\n") );   
    printf_P( PSTR("  | 8 clk @32 MHz |\n") );   
    
    printf_P( PSTR("\n") );


    return line;
}

//--------------------------------------------------------------------

char *dkaction_target_glitch( char *line )
{
    uint16_t glitch_delay = 1;      // delay (in 32 MHz cycles) between trigger and glitch
    int slowdown_delay_ms = 0;      // milliseconds to wait between glitch attempts
    int stop_char = -1;
    int stop_char_received = 0;
    char *token = NULL;
    int last_rx;
    int i;    



    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay = strtoul(token,NULL,0);
    }

    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        slowdown_delay_ms = strtoul(token,NULL,0);
    }

    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        stop_char = token[0];
    }

    dkaction_setglitch(NULL);       // print current glitch setup

    printf_P( PSTR("Using delay between trigger and glitch: %d\n"), glitch_delay );

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

    while( io_ctrl_char != DKKEY_CTRLC )
    {
        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
        {
            last_rx = dkfgetc(t_uart);
            _putch(last_rx);                // display any received characters
            if( stop_char >= 0 && last_rx == stop_char )
                stop_char_received = 1;
        }

        // insert artifical slowdown
        for( i=0; i<slowdown_delay_ms; i++ )
            _delay_ms( 1 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
        {
            last_rx = dkfgetc(t_uart);
            _putch(last_rx);                // display any received characters
            if( stop_char >= 0 && last_rx == stop_char )
                stop_char_received = 1;
        }
        
        if( stop_char_received )
            break;

        t_timeout_after_ms(50);
        while( !(PORTF.IN & (1<<4)) && io_ctrl_char != DKKEY_CTRLC )
        {
            if( !t_timeout_check() )
                continue;               // do nothing until timeout expires (wait for signal from target on PF4)
                
            // timeout has expired, reset the target
            
            // target rst = low
            TRST_PORT.OUTCLR = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);              
        
            // 10ms delay while rst low
            _delay_ms( 5 );
        
            // target rst = high
            TRST_PORT.OUTSET = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);
            break;
        }
        if( t_timeout_check() || io_ctrl_char == DKKEY_CTRLC )
            continue;

        // signal to target that we are ready
        PORTF.OUTCLR = (1<<5);
        PORTF.DIRSET = (1<<5);
        _delay_ms(1);
        PORTF.OUTSET = (1<<5);

        t_timeout_start();
        while( PORTF.IN & (1<<4) && io_ctrl_char != DKKEY_CTRLC && !t_timeout_check() )
            ;
        if( t_timeout_check() || io_ctrl_char == DKKEY_CTRLC )
            continue;   // timed out waiting for trigger signal
    
//TODO - the wait for PF0 to go low should be inside the asm() block so it is a proper timing reference

        asm volatile(
    
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //      "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
            "call DelayCycles" "\n\t"
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %0" "\n\t"              // vport0 out

            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %1" "\n\t"
            "out 0x0011, %2" "\n\t"
            "out 0x0011, %3" "\n\t"
            "out 0x0011, %4" "\n\t"
            "out 0x0011, %5" "\n\t"
            "out 0x0011, %6" "\n\t"
            "out 0x0011, %7" "\n\t"
            "out 0x0011, %8" "\n\t"
            :
            :
            "r" (glitch_form[0]),
            "r" (glitch_form[1]),
            "r" (glitch_form[2]),
            "r" (glitch_form[3]),
            "r" (glitch_form[4]),
            "r" (glitch_form[5]),
            "r" (glitch_form[6]),
            "r" (glitch_form[7]),
            "r" (noglitch_form),
            "z" (glitch_delay)
        );
    }
   
    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_rst_glitch( char *line )
{
    uint16_t glitch_delay = 100;
    uint16_t glitch_delay_start = 1;
    uint16_t glitch_delay_end = 1000;
    int stop_after_rx = -1;
    int slowdown_delay_ms = 20;
    char *token = NULL;
    int i;    
    char last_rx;



    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_start = strtoul(token,NULL,0);
    }

    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_end = strtoul(token,NULL,0);
    }

    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        slowdown_delay_ms = strtoul(token,NULL,0);
    }


    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        stop_after_rx = token[0];
    }


    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

    while( io_ctrl_char != DKKEY_CTRLC )
    {
        for( glitch_delay=glitch_delay_start; glitch_delay<glitch_delay_end && io_ctrl_char != DKKEY_CTRLC; glitch_delay++ )
        {
            printf_P( PSTR("\rDelay: %5d "), glitch_delay );

            while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
                _putch(dkfgetc(t_uart));                // display any received characters
            
            // insert artifical slowdown
            for( i=0; i<slowdown_delay_ms; i++ )
                _delay_ms( 1 );
            
            
            // target rst = low
            TRST_PORT.OUTCLR = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);              
            
            // 10ms delay while rst low
            _delay_ms( 10 );
            
            // target rst = high
            TRST_PORT.OUTSET = (1<<TRST_BIT);
            TRST_PORT.DIRSET = (1<<TRST_BIT);              
            
            asm volatile(
            
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //          "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
                "call DelayCycles" "\n\t"
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %0" "\n\t"              // vport0 out
                
                "out 0x0011, %0" "\n\t"              // vport0 out
                "out 0x0011, %1" "\n\t"
                "out 0x0011, %2" "\n\t"
                "out 0x0011, %3" "\n\t"
                "out 0x0011, %4" "\n\t"
                "out 0x0011, %5" "\n\t"
                "out 0x0011, %6" "\n\t"
                "out 0x0011, %7" "\n\t"
                "out 0x0011, %8" "\n\t"
                :
                :
                "r" (glitch_form[0]),
                "r" (glitch_form[1]),
                "r" (glitch_form[2]),
                "r" (glitch_form[3]),
                "r" (glitch_form[4]),
                "r" (glitch_form[5]),
                "r" (glitch_form[6]),
                "r" (glitch_form[7]),
                "r" (noglitch_form),
                "z" (glitch_delay)
            );
            
            _delay_ms( 1 );
            
            dkfputc('?', t_uart);
            
            t_timeout_after_ms(5);
            while( dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC && !t_timeout_check() )
            {
                if( (_kbhit() && _getch() == 'r') || t_timeout_check() )
                {
                    // target rst = low
                    TRST_PORT.OUTCLR = (1<<TRST_BIT);
                    TRST_PORT.DIRSET = (1<<TRST_BIT);              
            
                    // 10ms delay while rst low
                    _delay_ms( 10 );
            
                    // target rst = high
                    TRST_PORT.OUTSET = (1<<TRST_BIT);
                    TRST_PORT.DIRSET = (1<<TRST_BIT);              
                    
                    t_timeout_start();
                }
            }
            if( t_timeout_check() )
                continue;   // timed out waiting for trigger signal
            
            if( !dkfeof(t_uart) )
                last_rx = dkfgetc(t_uart);
            else
                continue;
            
            if( last_rx == 'X' )
                printf_P(PSTR("%c  "),last_rx);
            else
            {
                printf_P(PSTR("%c"),last_rx);

                t_timeout_start();
                while( dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC && !t_timeout_check() )
                {
                }
                if( !dkfeof(t_uart) )
                {
                    last_rx = dkfgetc(t_uart);
                    printf_P(PSTR("%c"),last_rx);
                }
                printf_P(PSTR("  \n"));
            }
            
            if( stop_after_rx >=0 && (unsigned char)last_rx == stop_after_rx )
                break;
        }
        
        if( glitch_delay<glitch_delay_end )
            break;      // the for() loop was broken before end (success)
    }
   
    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_rstloop( char *line )
{
    while( io_ctrl_char != DKKEY_CTRLC )
    {
        // target rst = low
        TRST_PORT.OUTCLR = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              

        // 10ms delay while rst low
        _delay_ms( 10 );
    
        // target rst = high
        TRST_PORT.OUTSET = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              
        
        _delay_ms( 490 );
    }
    
    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_glitch3( char *line )
{
    uint16_t glitch_delay = 1400;
    uint16_t glitch_delay_start = 1400;
    uint16_t glitch_delay_end = 1650;
    char *token = NULL;    
    char last_rx=0;


    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_start = strtoul(token,NULL,0);

        line = dkgettoken( line, token ); 
        if( strlen(token) )
        {
            glitch_delay_end = strtoul(token,NULL,0);
        }
    }


    printf_P( PSTR("Using delay range: %d - %d\n"), glitch_delay_start, glitch_delay_end );

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

   while( io_ctrl_char != DKKEY_CTRLC )
   {
    for( glitch_delay = glitch_delay_start; glitch_delay < glitch_delay_end && io_ctrl_char != DKKEY_CTRLC; glitch_delay++ )
    {
        printf_P(PSTR("\r%d    "), glitch_delay);

        // target rst = low
        TRST_PORT.OUTCLR = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              

        _delay_ms( 20 );
    
        // target rst = high
        TRST_PORT.OUTSET = (1<<TRST_BIT);

//TODO - the wait for PF0 to go low should be inside the asm() block so it is a proper timing reference

        asm volatile(
    
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //      "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
            "call DelayCycles" "\n\t"
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %1" "\n\t"
            "out 0x0011, %2" "\n\t"
            "out 0x0011, %3" "\n\t"
            "out 0x0011, %4" "\n\t"
            "out 0x0011, %5" "\n\t"
            "out 0x0011, %6" "\n\t"
            "out 0x0011, %7" "\n\t"
            "out 0x0011, %8" "\n\t"
            :
            :
            "r" (glitch_form[0]),
            "r" (glitch_form[1]),
            "r" (glitch_form[2]),
            "r" (glitch_form[3]),
            "r" (glitch_form[4]),
            "r" (glitch_form[5]),
            "r" (glitch_form[6]),
            "r" (glitch_form[7]),
            "r" (noglitch_form),
            "z" (glitch_delay)
        );


        _delay_ms( 10 );

        dkfputc('?', t_uart);
        _delay_ms( 70 );
        
        if( !dkfeof(t_uart) )
            last_rx = dkfgetc(t_uart);

        if( last_rx == 'S' && !dkfeof(t_uart) )
        {
            if( dkfgetc(t_uart) == 'y' )
            {
                printf_P(PSTR("Sy"));
                while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
                    _putch(dkfgetc(t_uart));
                printf_P(PSTR("\n"));
            }
        }
        
    }
   }
    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_glitchcrp( char *line )
{
    uint16_t glitch_delay = 1500;
    uint16_t glitch_delay_start = 1500;
    uint16_t glitch_delay_end = 1650;
    char *token = NULL;    
    char last_rx=0;


    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_start = strtoul(token,NULL,0);

        line = dkgettoken( line, token ); 
        if( strlen(token) )
        {
            glitch_delay_end = strtoul(token,NULL,0);
        }
    }


    printf_P( PSTR("Using delay range: %d - %d\n"), glitch_delay_start, glitch_delay_end );

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

   while( io_ctrl_char != DKKEY_CTRLC )
   {
    for( glitch_delay = glitch_delay_start; glitch_delay < glitch_delay_end && io_ctrl_char != DKKEY_CTRLC; glitch_delay++ )
    {
        printf_P(PSTR("\r                    \r"), glitch_delay);
        printf_P(PSTR("%d "), glitch_delay);

        // target rst = low
        TRST_PORT.OUTCLR = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              

        _delay_ms( 10 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
            dkfgetc(t_uart);
    
        // target rst = high
        TRST_PORT.OUTSET = (1<<TRST_BIT);

//TODO - the wait for PF0 to go low should be inside the asm() block so it is a proper timing reference

        asm volatile(
    
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //      "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
            "call DelayCycles" "\n\t"
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %1" "\n\t"
            "out 0x0011, %2" "\n\t"
            "out 0x0011, %3" "\n\t"
            "out 0x0011, %4" "\n\t"
            "out 0x0011, %5" "\n\t"
            "out 0x0011, %6" "\n\t"
            "out 0x0011, %7" "\n\t"
            "out 0x0011, %8" "\n\t"
            :
            :
            "r" (glitch_form[0]),
            "r" (glitch_form[1]),
            "r" (glitch_form[2]),
            "r" (glitch_form[3]),
            "r" (glitch_form[4]),
            "r" (glitch_form[5]),
            "r" (glitch_form[6]),
            "r" (glitch_form[7]),
            "r" (noglitch_form),
            "z" (glitch_delay)
        );


        _delay_ms( 5 );

        dkfputc('?', t_uart);
        _delay_ms( 5 );

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        last_rx = dkfgetc(t_uart);

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("? "));

        if( last_rx != 'S' )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("S"));

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        dkfwrite( "Synchronized\n", 1, strlen("Synchronized\n"), t_uart );
        _delay_ms( 5 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC  )
        {
            last_rx = dkfgetc(t_uart);
            if( last_rx == '\n' )
                break;
//            printf_P(PSTR("{%02X}"),last_rx);
        }

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("ync"));

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC  )
        {
            last_rx = dkfgetc(t_uart);
            if( last_rx == '\n' )
                break;
//            if( last_rx < 0x21 || last_rx > 0x7E )
//                printf_P(PSTR("{%02X}"),last_rx);
//            else
//                printf_P(PSTR("%c"),last_rx);
        }

        dkfwrite( "12000\n", 1, strlen("12000\n"), t_uart );
        _delay_ms( 5 );
        
        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC  )
        {
            last_rx = dkfgetc(t_uart);
            if( last_rx == '\n' )
                break;
//            if( last_rx < 0x21 || last_rx > 0x7E )
//                printf_P(PSTR("{%02X}"),last_rx);
//            else
//                printf_P(PSTR("%c"),last_rx);
        }

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }


        printf_P(PSTR("."));

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        
       
        printf_P(PSTR("R"));

        dkfwrite( "R 0 4\n", 1, strlen("R 0 4\n"), t_uart );
        _delay_ms( 5 );

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        if( dkfeof(t_uart) || dkfgetc(t_uart) == '1' )
        {
//            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("1"));

        printf_P( PSTR("\nSuccess!\n") );
//        return line;

//        if( dkfeof(t_uart) || dkfgetc(t_uart) == '9' )
//        {
//            printf_P(PSTR("\n"));
//            continue;
//        }
//
//        printf_P( PSTR("\nSuccess!\n") );
//        return line;
    }
   }
    
    return line;
}

//--------------------------------------------------------------------

char *dkaction_glitchnoisp( char *line )
{
    uint16_t glitch_delay = 0;
    uint16_t glitch_delay_start = 1600;
    uint16_t glitch_delay_end = 1630;
    char *token = NULL;    


    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_start = strtoul(token,NULL,0);

        line = dkgettoken( line, token ); 
        if( strlen(token) )
        {
            glitch_delay_end = strtoul(token,NULL,0);
        }
    }


    printf_P( PSTR("Using delay range: %d - %d\n"), glitch_delay_start, glitch_delay_end );

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

   while( io_ctrl_char != DKKEY_CTRLC )
   {
    for( glitch_delay = glitch_delay_start; glitch_delay < glitch_delay_end && io_ctrl_char != DKKEY_CTRLC; glitch_delay++ )
    {
        printf_P(PSTR("\r%d    "), glitch_delay);

        // target rst = low
        TRST_PORT.OUTCLR = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              

        _delay_ms( 5 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
            dkfgetc(t_uart);
    
        // target rst = high
        TRST_PORT.OUTSET = (1<<TRST_BIT);

//TODO - the wait for PF0 to go low should be inside the asm() block so it is a proper timing reference

        asm volatile(
    
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //      "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
            "call DelayCycles" "\n\t"
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %1" "\n\t"
            "out 0x0011, %2" "\n\t"
            "out 0x0011, %3" "\n\t"
            "out 0x0011, %4" "\n\t"
            "out 0x0011, %5" "\n\t"
            "out 0x0011, %6" "\n\t"
            "out 0x0011, %7" "\n\t"
            "out 0x0011, %8" "\n\t"
            :
            :
            "r" (glitch_form[0]),
            "r" (glitch_form[1]),
            "r" (glitch_form[2]),
            "r" (glitch_form[3]),
            "r" (glitch_form[4]),
            "r" (glitch_form[5]),
            "r" (glitch_form[6]),
            "r" (glitch_form[7]),
            "r" (noglitch_form),
            "z" (glitch_delay)
        );


        _delay_ms( 5 );

        dkfputc('?', t_uart);
        _delay_ms( 5 );

        if( dkfeof(t_uart) || dkfgetc(t_uart) != 'S' )
            continue;

        if( dkfeof(t_uart) || dkfgetc(t_uart) != 'y' )
            continue;

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        printf_P(PSTR("? "));


        dkfwrite( "Synchronized\n", 1, strlen("Synchronized\n"), t_uart );
        _delay_ms( 10 );

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        if( dkfeof(t_uart) )
            continue;

        printf_P(PSTR("Synchronized "));


        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        dkfwrite( "12000\n", 1, strlen("12000\n"), t_uart );
        _delay_ms( 5 );

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        if( dkfeof(t_uart) )
            continue;

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

printf_P(PSTR("\n"));
        return line;

       

        dkfwrite( "R 0 4\n", 1, strlen("R 0 4\n"), t_uart );
        _delay_ms( 5 );

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        if( dkfeof(t_uart) || dkfgetc(t_uart) == '1' )
            continue;

        if( dkfeof(t_uart) || dkfgetc(t_uart) == '9' )
            continue;
            

        printf_P( PSTR("Success!\n") );
//        break;
        
    }
   }
    
    return line;
}

//--------------------------------------------------------------------
// connect host (PC) and target device in passthru uart mode
// this performs Synchronization with LPC ISP software on PC side
char *dkaction_repass( char *line )
{
    uint8_t c;
    
 
    ((struct _dk_uart *)dkterm_interfaces->devfile->info)->_flag &= ~DKUART_FLAG_TERMINAL;

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );


    PORTD.DIR = 0x00;
    PORTD.OUT = 0x3F;
    PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN2CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN3CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN4CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTD.PIN5CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    
    PORTR.DIR = 0x00;
    PORTR.OUT = 0x03;
    PORTR.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;
    PORTR.PIN1CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_BOTHEDGES_gc;

    PORTE.OUTSET = (1<<0);
    PORTE.OUTCLR = (1<<1);
    PORTE.OUTSET = (1<<2);
    PORTE.OUTSET = (1<<3);

    while( PORTD.IN & (1<<0) )
    {
        PORTE.OUTSET = (1<<0);
        if( _kbhit() && _getch() == '?' )
        {       // received '?' 
            printf_P(PSTR( "Synchronized\r\n" ) );

            while( PORTD.IN & (1<<0) && PORTD.IN & (1<<1) )
            {
                if( _kbhit() )
                {
                    c = _getch();
                    _putch(c);
                    if( c == '\n' )
                        break;
                }
            }

            printf_P(PSTR( "OK\r\n"));

            while( PORTD.IN & (1<<0) && PORTD.IN & (1<<1) )
            {
                if( _kbhit() )
                {
                    c = _getch();
                    _putch(c);
                    if( c == '\n' )
                        break;
                }
            }

            printf_P(PSTR( "OK\r\n" ));

            PORTE.OUTCLR = (1<<0);
            while( PORTD.IN & (1<<0) && PORTD.IN & (1<<1) )
            {
                if( !dkfeof(t_uart) )
                    _putch(dkfgetc(t_uart));
    
                if(_kbhit())
                    dkfputc(_getch(), t_uart);
            }
        }
    }
    

    ((struct _dk_uart *)dkterm_interfaces->devfile->info)->_flag |= DKUART_FLAG_TERMINAL;

    
    return line;
}


//--------------------------------------------------------------------
// note: perform reset after this to change mode
char *dkaction_disableisp( char *line )
{
// target ISP = high
    TISP_PORT.OUTSET = (1<<TISP_BIT);
    TISP_PORT.DIRSET = (1<<TISP_BIT);              

    return line;
}


//--------------------------------------------------------------------
// note: perform reset after this to change mode
char *dkaction_enableisp( char *line )
{
// target ISP = low
    TISP_PORT.OUTCLR = (1<<TISP_BIT);
    TISP_PORT.DIRSET = (1<<TISP_BIT);              

    return line;
}

//--------------------------------------------------------------------

char *dkaction_glitchcrp2( char *line )
{
    uint16_t glitch_delay = 0;
    uint16_t glitch_delay_start = 0;
    uint16_t glitch_delay_end = 1000;
    char *token = NULL;    
    char last_rx=0;


    line = dkgettoken( line, token ); 
    if( strlen(token) )
    {
        glitch_delay_start = strtoul(token,NULL,0);

        line = dkgettoken( line, token ); 
        if( strlen(token) )
        {
            glitch_delay_end = strtoul(token,NULL,0);
        }
    }


    printf_P( PSTR("Using delay range: %d - %d\n"), glitch_delay_start, glitch_delay_end );

    if( !t_uart )
        t_uart = dkfopen( "COM1:115200,N,8,1", "r+" );

    PORTCFG.VPCTRLA = 0;                            // virtual port 0 = porta

   while( io_ctrl_char != DKKEY_CTRLC )
   {
    for( glitch_delay = glitch_delay_start; glitch_delay < glitch_delay_end && io_ctrl_char != DKKEY_CTRLC; glitch_delay++ )
    {
        printf_P(PSTR("\r                    \r"), glitch_delay);
        printf_P(PSTR("%d "), glitch_delay);

        // target rst = low
        TRST_PORT.OUTCLR = (1<<TRST_BIT);
        TRST_PORT.DIRSET = (1<<TRST_BIT);              

        _delay_ms( 5 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC )
            dkfgetc(t_uart);
    
        // target rst = high
        TRST_PORT.OUTSET = (1<<TRST_BIT);

        _delay_ms( 5 );

        dkfputc('?', t_uart);
        _delay_ms( 5 );

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        last_rx = dkfgetc(t_uart);

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("? "));

        if( last_rx != 'S' )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("S"));

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

//LPC812 only:
        dkfwrite( "Synchronized\r\n", 1, strlen("Synchronized\r\n"), t_uart );
//        dkfwrite( "Synchronized\n", 1, strlen("Synchronized\n"), t_uart );
        _delay_ms( 5 );

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC  )
        {
            last_rx = dkfgetc(t_uart);
            if( last_rx == '\n' )
                break;
//            printf_P(PSTR("{%02X}"),last_rx);
        }

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("ync"));

        while( !dkfeof(t_uart) && io_ctrl_char != DKKEY_CTRLC  )
        {
            last_rx = dkfgetc(t_uart);
            if( last_rx == '\n' )
                break;
//            if( last_rx < 0x21 || last_rx > 0x7E )
//                printf_P(PSTR("{%02X}"),last_rx);
//            else
//                printf_P(PSTR("%c"),last_rx);
        }

//LPC812 only:
        dkfwrite( "12000\r\n", 1, strlen("12000\r\n"), t_uart );
//        dkfwrite( "12000\n", 1, strlen("12000\n"), t_uart );
        

//TODO - the wait for PF0 to go low should be inside the asm() block so it is a proper timing reference

        t_timeout_after_ms(25);
        while( !t_timeout_check() && io_ctrl_char != DKKEY_CTRLC )
        {
            if( !dkfeof(t_uart) )
            {
                last_rx = dkfgetc(t_uart);
                if( last_rx == 0x0D )
//                if( last_rx == 'K' )
                    break;
            }
        }
        if( t_timeout_check() || io_ctrl_char == DKKEY_CTRLC )
        {
            printf_P(PSTR("\n"));
            continue;
        }

        asm volatile(
    
    //TODO: wait for PF0 to go low
    //        while( PORTF.IN & (1<<1) && io_ctrl_char != DKKEY_CTRLC )
    
    //      "sts 0x0BC3, %0" "\n\t"             // SPIF_DATA
            "call DelayCycles" "\n\t"
            "out 0x0011, %0" "\n\t"              // vport0 out
            "out 0x0011, %1" "\n\t"
            "out 0x0011, %2" "\n\t"
            "out 0x0011, %3" "\n\t"
            "out 0x0011, %4" "\n\t"
            "out 0x0011, %5" "\n\t"
            "out 0x0011, %6" "\n\t"
            "out 0x0011, %7" "\n\t"
            "out 0x0011, %8" "\n\t"
            :
            :
            "r" (glitch_form[0]),
            "r" (glitch_form[1]),
            "r" (glitch_form[2]),
            "r" (glitch_form[3]),
            "r" (glitch_form[4]),
            "r" (glitch_form[5]),
            "r" (glitch_form[6]),
            "r" (glitch_form[7]),
            "r" (noglitch_form),
            "z" (glitch_delay)
        );


        _delay_ms( 5 );

        if( dkfeof(t_uart) )
        {
            printf_P(PSTR("\n"));
            continue;
        }


        printf_P(PSTR("."));

        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        
       
        printf_P(PSTR("R"));

//LPC812 only:
        dkfwrite( "R 0 4\r\n", 1, strlen("R 0 4\r\n"), t_uart );
//        dkfwrite( "R 0 4\n", 1, strlen("R 0 4\n"), t_uart );

        t_timeout_after_ms(50);
        while( !t_timeout_check() && io_ctrl_char != DKKEY_CTRLC )
        {
            if( !dkfeof(t_uart) )
                last_rx = dkfgetc(t_uart);
//                if( last_rx == 0x0D )
                if( last_rx == 'R' )
                    break;
        }
        if( t_timeout_check() || io_ctrl_char == DKKEY_CTRLC )
        {
            printf_P(PSTR("\n"));
            continue;
        }


        while( !dkfeof(t_uart) && dkfgetc(t_uart) != '\n' && io_ctrl_char != DKKEY_CTRLC  )
            ;        

        if( dkfeof(t_uart) || dkfgetc(t_uart) == '1' )
        {
//            printf_P(PSTR("\n"));
            continue;
        }

        printf_P(PSTR("1"));

        printf_P( PSTR("\nSuccess!\n") );
//        return line;

//        if( dkfeof(t_uart) || dkfgetc(t_uart) == '9' )
//        {
//            printf_P(PSTR("\n"));
//            continue;
//        }
//
//        printf_P( PSTR("\nSuccess!\n") );
//        return line;
    }
   }
    
    return line;
}

