#include <avr/io.h>


;----------------------------------------------------------------------------
; DelayCycles - delay for total of 23 + Z card cycles, including ldi/call/ret
.global DelayCycles
DelayCycles:
	lsr		ZL
	brcs    delay1

delay1:
	lsr     ZL
	brcc    delay2
	nop
	rjmp    delay2

delay2:
	inc		ZL
	inc		ZH

delay3_loop:
	nop
	dec		ZL
	brne	delay3_loop
	nop
	ldi		ZL, 0x3F
	dec		ZH
	brne	delay3_loop


	ret

