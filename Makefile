TARGET = blinklights

# Target type
MCU=attiny13
PROGRAMMER=avrmkii
F_CPU=1000000

SRC = blinkylights.c

SRC += $(DRIVERS)
OBJ = $(SRC:.c=.o)

CC = avr-gcc

# Compiler / Linker flags:
CFLAGS = -mmcu=$(MCU) -Wall -Os -std=gnu99 -D F_CPU=$(F_CPU) -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
LDFLAGS = -mmcu=$(MCU) -Wl,-Map=$(TARGET).map



###################################
# Makerules:


.PHONY: compile flash clean

compile: $(TARGET).hex $(TARGET).eep $(TARGET).lss

flash: compile
	avrdude -c $(PROGRAMMER) -P usb -p $(MCU) -U flash:w:$(TARGET).hex
	# sleep 2
	# avrdude -c $(PROGRAMMER) -P usb -p $(MCU) -U eeprom:w:$(TARGET).eep

clean:
	rm -f $(OBJ) $(TARGET).{elf,hex,lss,map,eep}

###################################
# Psudorules:

%.eep: %.hex
	@echo "  OBJCOPY    ${<}"
	@avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@

%.hex: %.elf
	@echo "  OBJCOPY    ${<}"
	@avr-objcopy -O ihex -R .eeprom -R .fuse -R .lock -R .signature $< $@

%.lss: %.elf
	@echo "  OBJDUMP    ${<}"
	@avr-objdump -h -S $< > $@
	@echo "  SIZE    ${<}"
	@avr-size -C --mcu=$(MCU) $(TARGET).elf

%.elf: $(OBJ)
	@echo "  LN    ${@}"
	@avr-gcc $^ $(LDFLAGS) -o $@

%.o : %.c
	@echo "  CC    ${<}"
	@avr-gcc $(CFLAGS) -c $< -o $@
