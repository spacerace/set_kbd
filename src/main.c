#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <graph.h>
#include <inttypes.h>
#include <dos.h>

#include "con.h"

/* these defines are used for screen layout and colors, it's a mess, but
 * far better than hardcoded these values all over the code...
 */
#define TOP_LINE_BG_CHAR    0xDB
#define TOP_LINE_BG_COLOR   0x04
#define TOP_LINE_TEXT_COLOR 0x4f

#define BOT_LINE_BG_CHAR    0xDB
#define BOT_LINE_BG_COLOR   0x04
#define BOT_LINE_TEXT_COLOR 0x4a
#define BOT_LINE_HL_COLOR   0x4F

#define BG_CHAR     0xB2
#define BG_COLOR    0x08

#define BORDER_COLOR    0x47

#define ATTR_ENTRY      0x4F
#define ATTR_ENTRY_HL   0x30

#define ENTRY_LEN       32
#define FIRST_ENTRY_X   5
#define FIRST_ENTRY_Y   3

/* hl = selected entry in list (highlighted)
 * list_start = list displaying starts at this entry
 */
int hl, list_start;

/* functions for text user interface */
void draw_background(void);
void draw_list_win(void);
void draw_info_win(void);
void print_entries(void);
void change_attr(int x, int y, uint8_t attr);
void highlight_entry(int e);
void unhighlight_entry(int e);
void entry_setattr(int e, uint8_t attr);
void update_info_win(void);

/* execute keyb command */
void set_keyboard_layout(void);

void test_layout(void);

/* set environment */
void set_env_lang(void);

/* load conig file and populate list arrays */
void load_cfg(char *cfg_file);

/* these arrays contain the list entries, populated by load_cfg() function */
int entries;                /* number of list entries */
char entries_code[256][9];  /* language codes for keyb command (UK, GR, DK, ...) */
char entries_lang[256][9];  /* environment setting for language (setting for LANG=DE, LANG=...) */
char entries_desc[256][33]; /* human readable description for list */
char keyb_command[33];      /* the command for keyb-tool (keyb.com, mkeyb.exe, ...) */

/* we use this for saving/restoring DOS screen when program exits again. */
static uint16_t buffer_screensaverestore[80*25];

void print_usage(char *own) {
    printf("usage: %s config.cfg\n\n", own);
}

void main(int argc, char **argv) {
    uint8_t c;
    char str[50];

	screen_save();
    
    init_con();
    
    clrscr();

    if(argc != 2) {
        gotoxy(5,0);
        print_usage(argv[0]);
        
        exit(-1);
    }
    
    load_cfg(argv[1]);
    
    draw_background();
    draw_list_win();
    draw_info_win();
    
    hl = 0;             /* selected entry, highlighted in list */
    list_start = 0;     /* list view starts at this entry, if more than LIST_ENTRIES entries in list */
    
    update_info_win();
    print_entries();
    highlight_entry(hl);
    
    for(;;) {
        c = getch();
        switch(c) {
            case 0x0D:  /* RETURN */
                screen_restore();
                gotoxy(24,0);                
                set_keyboard_layout();
                exit(0);
                break;
            case 0x48:  /* ARROW UP */
                if(hl > 0) {
                    hl--;
                } else if(list_start > 0) {
                    list_start--;
                    print_entries();
                }
                unhighlight_entry(hl+1);
                highlight_entry(hl);
                update_info_win();
                break;
            case 0x50:  /* ARROW DOWN */
                if(hl < 18) {
                    hl++;
                } else {
                    if(list_start < (entries-19)) {
                        list_start++;
                        print_entries();
                    }
                }
                unhighlight_entry(hl-1);
                highlight_entry(hl);
                update_info_win();
                break;
            case 0x42:  /* F8 "SET+TEST" */
                clrscr();
                set_keyboard_layout();
                printf("\n");
                test_layout();
                draw_background();
                draw_list_win();
                draw_info_win();       
                update_info_win();
                print_entries();
                highlight_entry(hl);           
                break;
            case 0x44:  /* F10 */
                screen_restore();
                gotoxy(24,0);
                exit(0);
                break;
        }
    }
}

void test_layout(void) {
    char c = 0;
    printf("echoing typed characters. END WITH q OR ESC\n\n");
    while(c != 'q') {
        c = getch();
        putch(c);
    }
    return;
}

/* this function executes keyb-command with corresponding language code
 * taken it's parameters from global:
 *   string    keyb_command
 *   string    entries_code[x], where x is the entry-number of preloaded list entries 
 */
void set_keyboard_layout(void) {
        char line[129];
        sprintf(line, "%s %s", keyb_command, entries_code[hl+list_start]);
        printf("  ==>>> executing %s\n\n", line);
        system(line);
        return;
}

void set_env_lang(void) {
    
}

/* change the attribute of a character on screen */
void change_attr(int x, int y, uint8_t attr) {
    uint8_t far *vram;
    int offset;
    
    offset = (((y*80)+x)*2);
    
    vram = MK_FP(0xB800, offset);
    vram++;
    *vram = attr;
    
    return;
}


void highlight_entry(int e) {
    entry_setattr(e, 0x3F);
}

void unhighlight_entry(int e) {
    entry_setattr(e, 0x4F);
}

void entry_setattr(int e, uint8_t attr) {    
    int x, y, i;
    
    x = FIRST_ENTRY_X;
    y = FIRST_ENTRY_Y + e;
    
    for(i = 0; i < ENTRY_LEN; i++) {
        change_attr(x+i, y, attr);
    }
        
}

void print_entries(void) {
    int x, y, i, j;
    char line[49];
    
    x = FIRST_ENTRY_X;
    y = FIRST_ENTRY_Y;
    
    for(i = 0; i < 19; i++) {
        sprintf(line, " %s  -   %s", entries_code[list_start+i], entries_desc[list_start+i]);
        for(j = 0; j < 35; j++) {
            putc_attr(x-1+j, y+i, ' ', 0x44);
        }
        puts_attr(x, y+i, line, ATTR_ENTRY);
    }
}

void update_info_win(void) {    
    char buf[25];

    sprintf(buf, "Layout  %s     ", entries_code[list_start+hl]);
    puts_attr(50, 6, buf, 0x4f);
    sprintf(buf, "%s", entries_code[list_start+hl]);
    puts_attr(58, 6, buf, 0x4A);

    return;
}

void draw_background(void) { 
    int i;
    uint8_t far *vram;
    
    /* draw top border line */
    vram = MK_FP(0xB800, 0);
    for(i = 0; i < 80; i++) {
        *vram = TOP_LINE_BG_CHAR;
        vram++;
        *vram = TOP_LINE_BG_COLOR;
        vram++;
    }
    
    /* draw bottom border line */
    vram = MK_FP(0xB800, 80*2*24);
    for(i = 0; i < 80; i++) {
        *vram = BOT_LINE_BG_CHAR;
        vram++;
        *vram = BOT_LINE_BG_COLOR;
        vram++;
    }
    
    /* write title bar and bottom bar text */
    puts_attr(27, 0, "CHOOSE A KEYBOARD LAYOUT!", TOP_LINE_TEXT_COLOR); 
    puts_attr(1, 24, " \x18/\x19 NAVIGATE   RET SET+EXIT   F8 SET+TEST                           F10 QUIT", BOT_LINE_TEXT_COLOR); 
    putc_attr(2, 24, 0x18, BOT_LINE_HL_COLOR);
    putc_attr(4, 24, 0x19, BOT_LINE_HL_COLOR);
    puts_attr(17,24, "RET", BOT_LINE_HL_COLOR);
    puts_attr(32,24, "F8", BOT_LINE_HL_COLOR);
    puts_attr(70,24, "F10", BOT_LINE_HL_COLOR);
    
    /* fill background with char and attribute */
    vram = MK_FP(0xB800, 80*2);
    for(i = 0; i < (80*23); i++) {
        *vram = BG_CHAR;
        vram++;
        *vram = BG_COLOR;
        vram++;
    }
    
    return;
}

/* loads config file SET_KBD.CFG and populates arrays for list entries:
 *   entries_code   = language code for keyb.com/mkeyb.exe/...
 *   entries_lang   = environment variable content for this language
 *   entries_desc   = human readable description for entry
 */
void load_cfg(char *cfg_file) {
    char line[8+8+32+1];
    char *ptr, delim[2] = ";";
    FILE *f;
     
    f = fopen(cfg_file, "r");    
    if(f == NULL) {
        clrscr();
        gotoxy(0,0);
        printf("cant open '%s'!!!\n", cfg_file);
        exit(-1);
    }
    
    /* get KEYB-command from config file */
    if(fgets(line, 32, f) == NULL) {
        printf("error reading line 0 from config!\n");
        exit(-2);
    }
    line[strlen(line)-1] = 0;   /* remove trailing \n */
    strncpy(keyb_command, line, 32);
    
    /* get all entries and count entries */
    entries = 0;
    while(fgets(line, 8+8+32, f) != NULL) {
        line[strlen(line)-1] = 0;                   /* remove trailing \n */
        
        ptr = strtok(line, delim);                  /* 1st field = language code */
        strncpy(entries_code[entries], ptr, 8);
        entries_code[entries][8] = 0;

        ptr = strtok(NULL, delim);                  /* 2nd field = environment variable for language */
        strncpy(entries_lang[entries], ptr, 8);
        entries_lang[entries][8] = 0;
        
        ptr = strtok(NULL, delim);                  /* 3rd field = description */
        strncpy(entries_desc[entries], ptr, 32);
        entries_desc[entries][32] = 0;        
                
        entries++;
    }
    fclose(f);
}


void draw_list_win(void) {
    uint16_t x,y,offset;
    uint8_t far *vram;

    for(y = 2; y < 23; y++) {
            x = 2;
            offset = (y*80+x)*2;
            vram = MK_FP(0xB800, offset);
            for(x = 0; x < 40; x++) {
                *vram = TOP_LINE_BG_CHAR;
                vram++;
                *vram = 0x04;
                vram++;
            }
    }

    putc_attr(2, 2, 0xc9, BORDER_COLOR);
    putc_attr(41, 2, 0xbb, BORDER_COLOR);  
    putc_attr(2, 22, 0xc8, BORDER_COLOR);
    putc_attr(41, 22, 0xbc, BORDER_COLOR);
    for(x = 0; x < 38; x++) {
        putc_attr(2+x+1, 2, 0xCD, BORDER_COLOR);
        putc_attr(2+x+1, 22, 0xCD, BORDER_COLOR);
    }
    for(y = 0; y < 19; y++) {
        putc_attr(2, 3+y, 0xba, BORDER_COLOR);
        putc_attr(41, 3+y, 0xB0, BORDER_COLOR);
    }
    putc_attr(41, 3, 0x1e, 0x4a);
    putc_attr(41, 21, 0x1f, 0x4a);

    return;
}

void draw_info_win(void) {
    char buf[25];
    uint16_t x,y,offset;
    uint8_t far *vram;

    for(y = 2; y < 13; y++) {
            x = 47;
            offset = (y*80+x)*2;
            vram = MK_FP(0xB800, offset);
            for(x = 0; x < 29; x++) {
                *vram = TOP_LINE_BG_CHAR;
                vram++;
                *vram = 0x04;
                vram++;
            }
    }
    
    putc_attr(47, 2, 0xc9, BORDER_COLOR);
    putc_attr(47+28, 2, 0xbb, BORDER_COLOR);
    putc_attr(47, 12, 0xc8, BORDER_COLOR);
    putc_attr(47+28, 12, 0xbc, BORDER_COLOR);
    for(x = 0; x < 27; x++) {
        putc_attr(47+x+1, 2, 0xCD, BORDER_COLOR);
        putc_attr(47+x+1, 12, 0xCD, BORDER_COLOR);
    }
    for(y = 0; y < 10-1; y++) {
        putc_attr(47, 3+y, 0xba, BORDER_COLOR);
        putc_attr(47+28, 3+y, 0xba, BORDER_COLOR);
    }
    

    sprintf(buf, "Using '%s'", keyb_command);
    puts_attr(50, 3, buf, 0x4f);
    sprintf(buf, "%s", keyb_command);
    puts_attr(57, 3, buf, 0x4A);
    
    update_info_win();
    
}


/* 
 * saves the whole text window including cursor position for restoring before program exit */
void screen_save(void) {
    area_save_to(&buffer_screensaverestore, 0, 0, 79, 24);
}

/* 
 * restores the text window before exit */
void screen_restore(void) {
   area_restore_from(buffer_screensaverestore, 0, 0, 79, 24);
}

/* copies a screen area to a buffer 
 */
void area_save_to(uint16_t *buf, int x1, int y1, int x2, int y2) {
        int x, y;
        uint16_t far *ptr;
        uint16_t offset;
        
        for(y = y1; y <= y2; y++) {
            offset = ((y*80)+x1)*2;
            ptr = MK_FP(0xB800, offset);
            for(x = 0; x <= x2; x++) {
                *buf = *ptr;
                buf++;
                ptr++;
            }
        }

        return;
}

/* restores a screen area from buffer */
void area_restore_from(uint16_t *buf, int x1, int y1, int x2, int y2) {
        int x, y;
        uint16_t far *ptr;
        uint16_t offset;

        for(y = y1; y <= y2; y++) {
            offset = ((y*80)+x1)*2;
            ptr = MK_FP(0xB800, offset);
            for(x = 0; x <= x2; x++) {
                *ptr = *buf;
                buf++;
                ptr++;

            }
        }

        return;    
}


