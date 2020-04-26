ASM=fasm
CC=gcc
LD=ld
CFLAGS=-O3 -c -Wall -std=gnu99 -m32
LDFLAGS=-m elf_i386  -nostdlib -T kolibri.ld 
SOURCES=e80.c system/ctype.c system/kolibri.c system/stdlib.c system/string.c z80/z80.c
OBJECTS=e80.o ctype.o kolibri.o stdlib.o string.o z80.o
ASM_OBJECTS=start.o rom.o
TARGET=e80

clean:
	rm -rf *.o
	rm -rf $(TARGET)
	rm -rf *.iso

$(ASM_OBJECTS):
	$(ASM) start.asm start.o
	$(ASM) rom.asm rom.o

$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES)

$(TARGET): $(OBJECTS) $(ASM_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(ASM_OBJECTS) $(OBJECTS)
	objcopy $@ -O binary

all: $(TARGET)
	rm -rf bin
	mkdir bin
	cp $(TARGET) bin
	make clean

iso:
	make clean
	make all
	cp games/*.sna bin
	mkisofs -lJR  -o e80.iso bin/
