
TARGET         = xplainer
CSRC           = xplainer.c dkfile.c dkfile_uart.c dkterm.c dkvt100.c dkstring.c dkinteractive.c dkbuttons.c dkclock.c dkebi.c timeout.c \
                actions/button0.c actions/button1.c actions/button2.c actions/button3.c actions/button4.c actions/button5.c actions/button6.c actions/button7.c \
                actions/help.c actions/peek.c actions/poke.c actions/hello.c actions/led.c actions/hotbutton.c actions/hotkey.c actions/disco.c actions/spi.c actions/spispy.c \
                actions/target.c actions/laser.c
ASRC           = dktables.S dkactiontable.S delaycycles.S
MCU_TARGET      = atxmega128a1
OPTIMIZE       = -Os -mcall-prologues
DEFS           = -DF_CPU=32000000 -D_GLITCHER_VERSION_NUM=2
LIBS           = 
DEBUG          = dwarf-2

CC             = avr-gcc
ASFLAGS        = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
ALL_ASFLAGS    = -mmcu=$(MCU_TARGET) -I. -x assembler-with-cpp $(ASFLAGS)
CFLAGS         = -g$(DEBUG) -Wall -Werror $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS) -lm
LDFLAGS        = -Wl,-u,vfprintf -lprintf_flt
#LDFLAGS += -Wl,--section-start=.bootloader=$(BOOTLOAD)
#LDFLAGS += -Wl,--section-start=.flashfile=$(FLASHFILE)
OBJ            = $(CSRC:.c=.o) $(ASRC:.S=.o)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump
SIZE           = avr-size

HEX_FLASH_FLAGS = -R .eeprom -R .fuse -R .lock -R .signature
#HEX_FLASH_FLAGS = -j .text -j .data -j .bootloader

HEX_EEPROM_FLAGS = -j .eeprom
HEX_EEPROM_FLAGS += --set-section-flags=.eeprom="alloc,load"
HEX_EEPROM_FLAGS += --change-section-lma .eeprom=0 --no-change-warnings



all: $(TARGET).elf lst text eep size

$(TARGET).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)


prog: clean all
	avrdude -p $(MCU_TARGET) -c jtag2 -P usb -U flash:w:$(TARGET).hex
#	avrdude -p $(MCU_TARGET) -c jtag2 -P usb -U flash:w:$(TARGET).hex -U eeprom:w:$(TARGET).eep:r
#	avrdude -p $(MCU_TARGET) -c stk500 -P /dev/ttyUSB1 -U flash:w:$(TARGET).hex -U eeprom:w:$(TARGET).eep:r

clean:
	rm -rf *.o $(TARGET).elf *.eps *.bak *.a
	rm -rf actions/*.o
	rm -rf *.lst *.map $(EXTRA_CLEAN_FILES)
	rm -rf $(TARGET).hex
	rm -rf $(TARGET).eep

size: $(TARGET).elf
#	$(SIZE) --target=$(MCU_TARGET) $(TARGET).elf
#	$(SIZE) -C --mcu=$(MCU_TARGET) $(TARGET).elf

lst:  $(TARGET).lst
%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

%.o : %.S
	$(CC) -c $(ALL_ASFLAGS) $< -o $@


eep: $(TARGET).eep

%.eep: %.elf
	$(OBJCOPY) -O binary $(HEX_EEPROM_FLAGS) $< $@


text: hex
hex:  $(TARGET).hex

%.hex: %.elf
	$(OBJCOPY) -O ihex $(HEX_FLASH_FLAGS) $< $@


