ASM=fasm
CC=gcc
LD=ld
CFLAGS=-O3 -c -Wall -std=gnu99 -m32 -fno-builtin
LDFLAGS=-m elf_i386  -nostdlib -T kolibri.ld 
SOURCES=e80.c system/ctype.c system/kolibri.c system/stdlib.c system/string.c system/msgbox.c z80/z80.c
OBJECTS=e80.o ctype.o kolibri.o stdlib.o string.o msgbox.o z80.o
ASM_OBJECTS=start.o rom.o
TARGET=e80
KOLIBRI_IMG_FILE=./kolibri/kolibri.img

clean:
	rm -rf *.o
	rm -rf $(TARGET)

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
	cp games/*.sna bin
	cp config/e80.ini bin
	make clean

run:
	test -s $(KOLIBRI_IMG_FILE) \
	 || { wget -O bin/latest-img.7z http://builds.kolibrios.org/eng/latest-img.7z ; \
	 	  7za x -okolibri bin/latest-img.7z ; \
	 	  rm -f bin/latest-img.7z ; }
	-mdeltree -i $(KOLIBRI_IMG_FILE) 3d demos games e80
	mmd -i $(KOLIBRI_IMG_FILE) e80
	mcopy -i $(KOLIBRI_IMG_FILE) bin/e80 ::/e80/e80
	mcopy -i $(KOLIBRI_IMG_FILE) bin/othello.sna ::/e80/othello.sna
	mcopy -i $(KOLIBRI_IMG_FILE) bin/enduro.sna ::/e80/enduro.sna
	mcopy -i $(KOLIBRI_IMG_FILE) bin/e80.ini ::/e80/e80.ini
	qemu-system-i386 -m 256M -blockdev driver=file,node-name=f0,filename=$(KOLIBRI_IMG_FILE) -device floppy,drive=f0
