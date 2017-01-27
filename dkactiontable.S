;---------------------------------------------------------------------
; dkactiontable.S - table of action names and handler functions for DK-INTERACTIVE - Chris Gerlinsky, 2011
;---------------------------------------------------------------------

.nolist
#include <avr/io.h>	// Include device specific definitions.
.list


;---------------------------------------------------------------------
; FLASH - constants (in code segment)

.section .text


; actions are found in actions/ directory

.global dkactiontable
dkactiontable:
.string "help"                          ; keyword "help"
.word	dkaction_help                   ; dkaction_help() in actions/help.c

.string "hello"                         ; keyword "hello" - "Hello, world!" example
.word	dkaction_hello                  ; dkaction_hello() in actions/hello.c

.string "peek"
.word	dkaction_peek
.string "poke"
.word	dkaction_poke

.string "led"
.word	dkaction_led

.string "spi"                           ; SPI master TX bytes
.word	dkaction_spi
.string "spispy"                        ; SPISPY - dual slave SPI monitoring
.word	dkaction_spispy

.string "hotbutton"
.word dkaction_hotbutton
.string "hotkey"
.word dkaction_hotkey

.string "disco"
.word dkaction_disco

.string "button0"
.word	dkaction_button0
.string "button1"
.word	dkaction_button1
.string "button2"
.word	dkaction_button2
.string "button3"
.word	dkaction_button3
.string "button4"
.word	dkaction_button4
.string "button5"
.word	dkaction_button5
.string "button6"
.word	dkaction_button6
.string "button7"
.word	dkaction_button7


.string "pon"
.word   dkaction_pon
.string "poff"
.word   dkaction_poff

.string "passthru"
.word dkaction_passthru

.string "glitch"
.word dkaction_glitch

.string "setglitch"
.word dkaction_setglitch

.string "target_glitch"
.word dkaction_target_glitch

.string "glitch3"
.word dkaction_glitch3

.string "rstloop"
.word dkaction_rstloop

.string "glitchcrp"
.word dkaction_glitchcrp

.string "glitchcrp2"
.word dkaction_glitchcrp2

.string "glitchnoisp"
.word dkaction_glitchnoisp

.string "rst_glitch"
.word dkaction_rst_glitch

.string "repass"
.word dkaction_repass


.string "laser"
.word dkaction_laser


.byte	0								; end dkactiontable with an empty string (NULL)


.align 2								; code must be aligned on word boundary


;---------------------------------------------------------------------
; dkfindaction
; r25:r24 in = ptr to string in ram
; r25:r24 out = ptr to action handler function
; note: side effect: string is converted to lowercase

.func dkfindaction
.global dkfindaction
dkfindaction:
	ldi		r23, hi8(dkactiontable)	    ; dkactiontable in FLASH is a list of names and matching functions
	ldi		r22, lo8(dkactiontable)

	call	dksearchtable

	lsr		r25							; shift address to the right (adjust from byte address to word address)
	ror		r24

	ret

.endfunc


;---------------------------------------------------------------------

