# makefile for COM file

CC=wcc
CFLAGS=-q -0 -bt=dos -zm
LINKER=wlink

OBJ=src/main.obj src/con.obj
OUTPUT=set_kbd.com
MEMORY_MODEL=-ms

all: $(OBJ)
	$(LINKER) option eliminate option quiet system com name $(OUTPUT) file { $(OBJ) }
	ls -al $(OUTPUT)

%.obj: %.c
	$(CC) $(CFLAGS) $(MEMORY_MODEL) -fo=$@ $<

clean:
	rm -rf $(OBJ) $(OUTPUT) *.err *.def

test:
	dosbox

upx:
	rm -f upxskbd.com
	upx -9 --8086 -o upxskbd.com $(OUTPUT)
	mv upxskbd.com $(OUTPUT)
	ls -al $(OUTPUT)
