;---------------------------------------------------------------------
; dktables.S - for DK-INTERACTIVE - Chris Gerlinsky, 2011
;---------------------------------------------------------------------

.nolist
#include <avr/io.h>	// Include device specific definitions.
.list


;---------------------------------------------------------------------
; FLASH - code
.section .text


;---------------------------------------------------------------------
; dksearchtable
; r25:r24 in = ptr to string in ram
; r23:r22 in = ptr to start of table in flash
; r25:r24 out = ptr to statment handler function
; note: side effect: string is converted to lowercase

.func dksearchtable
.global dksearchtable
dksearchtable:
	push	r2							; save call-saved registers
	push	r3							; save param passed to us in r3:r2
	push	r4							; save Z in r5:r4
	push	r5

	
	movw	r4, r22						; temporarily save param2 (ptr to table in flash) in r5:r4 
	movw	r2, r24						; save param in r2
	call	strlwr						; convert string to lowercase
	movw	r24, r2
	movw	ZL, r4						; copy param2 (ptr to table in flash) into Z

2:
	lpm		r21, Z						; read first byte of entry name from flash
	cpi		r21, 0
	brne	1f
	clr		r24							; name = NULL, end of table was reached
	clr		r25
	rjmp	3f
1:

	movw	r22, ZL						; r23:r22 points to string in flash (setup strcmp_P call)
	movw	r24, r2						; r25:r24 points to string in RAM

	movw	r4, ZL
	call	strcmp_P					; compare string in RAM to string in FLASH
	movw	ZL, r4

1:
	lpm		r21, Z+						; adjust Z to point to end of item name in table (point to next item in table)
	cpi		r21, 0
	brne	1b

	cpi		r24, 0						; check for successful string compare
	breq	1f

	adiw	ZL, 2						; push Z past address of prev handler
	rjmp	2b							; continue loop to check next entry in table


1:
	lpm		r24, Z+						; read 16-bit address of handler
	lpm		r25, Z+	
;	lsr		r25							; shift address to the right (adjust from byte address to word address)
;	ror		r24
3:
	pop		r5							; restore call-saved registers
	pop		r4
	pop		r3
	pop		r2
	ret

.endfunc


;---------------------------------------------------------------------

