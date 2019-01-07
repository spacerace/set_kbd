#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <graph.h>
#include <inttypes.h>
#include <dos.h>

#include "con.h"

static uint8_t far *vram;
static uint8_t def_c;
static uint8_t attr;
static int curx, cury;


void textcolor(uint8_t color) {
    attr = color;
    return;
}

void init_con(void) {
    vram = MK_FP(0xB800, 0x0000);
    def_c = 0x20;
    attr = 0x07;
    
    return;
}

void clrscr(void) {
    int i;
   
    vram = MK_FP(0xB800, 0x0000);
    for(i = 0; i < 2000; i++) {
        *vram = def_c;
        vram++;
        *vram = attr;
        vram++;
    }
    gotoxy(0,0);

    return;
}

void putc_attr(int x, int y, uint8_t c, uint8_t attr) {
    unsigned int offset;

    offset = (((y*80)+x)*2);
    
    vram = MK_FP(0xB800, offset);
    if(*vram != c)
		*vram = c;
    vram++;
    if(*vram != attr)
		*vram = attr;
    
    return;
}

void puts_attr(int x, int y, char *str, uint8_t attr) { 
    while(*str) {
        putc_attr(x, y, *str, attr);
        x++;
        if(x > 79) {
            x = 0;
            y++;
        }
        str++;
    }
    
    return;
}

void gotoxy(int x, int y) {
    union REGS i, o;

    i.h.ah = 0x0f;
    int86(0x10, &i, &o);

    i.h.ah = 0x02;
    i.h.bh = o.h.bh;
    i.h.dh = (uint8_t)x;
    i.h.dl = (uint8_t)y;

    int86(0x10, &i, &o);
    
    curx = x;
    cury = y;

    return;    
}


