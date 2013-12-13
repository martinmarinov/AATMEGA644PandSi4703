
# Makefile for programming ATMEGA644 family of devices.
# You might need to modify the file depending on the specific device:

# ATMEGA644 and ATMEG644A use:
#   MCU=atmega644
#   avrdude -p m168 ... (3 places)

# ATMEGA644P and ATMEGA644PA use:
#   MCU=atmega644p
#   avrdude -p m644p ... (3 places)


CPPFLAGS=-I. -I../lib
MCU=atmega644p
VPATH=../lib
all: main.hex

main.elf: main.o fm.o i2c.o spi2.o lcd.o buttons.o

%.o: %.c
	avr-gcc ${CPPFLAGS} -Wall -Os -mmcu=${MCU} -o $@ -c $^

%.elf: %.o
	avr-gcc -Os -mmcu=${MCU} -o $@ $^

%.hex: %.elf
	avr-objcopy -j .text -j .data -O ihex $^ $@

%.lst: %.elf
	avr-objdump -h -S $^ > $@

clean:
	rm -f *.o *.elf *.hex 


program: main.hex
	avrdude -p m644p -P /dev/ttyUSB0 -c avrusb500 -e -U flash:w:$^


# These are the default fuses, which result in using an 8MHz internal oscillator with no CLKOUT signal
# Note that fuse bits are active low
fuses:
	avrdude -p m644p -P /dev/ttyUSB0 -c avrusb500 -e -U hfuse:w:0x99:m
	avrdude -p m644p -P /dev/ttyUSB0 -c avrusb500 -e -U lfuse:w:0xE2:m

