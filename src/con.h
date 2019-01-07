#ifndef __CON_H__
#define __CON_H__

void init_con(void);

void clrscr(void);

void putc_attr(int x, int y, uint8_t c, uint8_t attr);
void puts_attr(int x, int y, char *str, uint8_t attr);

void textcolor(uint8_t color);
void gotoxy(int x, int y);

void screen_save(void);
void screen_restore(void);
void area_save_to(uint16_t *buf, int x1, int y1, int x2, int y2);
void area_restore_from(uint16_t *buf, int x1, int y1, int x2, int y2);

#endif // __CON_H__
