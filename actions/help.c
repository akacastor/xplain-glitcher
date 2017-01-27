#include <stdio.h>
#include <avr/pgmspace.h>


char *dkaction_help( char *line )
{
    printf_P( PSTR(
"DK-INTERACTIVE help:\n"
"\n"
"PEEK <address> [len]  (PEEK 0x2000 3 to display 3 bytes at 0x2000)\n"
"POKE <address> <value> [value ...]  (POKE 0x2000 0xFF to set 0x2000=0xFF)\n"
"HELLO  (\"Hello, world!\" example action)\n"
"BEEP  (make a short, loud, beep with the speaker)\n"
"LED <#> <1/0>  (LED 2 0 to turn LED2 off)\n"
"BUTTON<#>  (BUTTON0 to perform BUTTON0 action)\n"
"HOTBUTTON <#> <\"string\">  (HOTBUTTON 0 \"BEEP\\n\" for example)\n"
"HOTBUTTON ?  (list hotbutton definitions)\n"
"HOTKEY <#> <\"string\">  (HOTKEY F2 \"BEEP\\n\" for example)\n"
"HOTKEY ?  (list hotkey definitions)\n"
        ) );

    return line;
}
