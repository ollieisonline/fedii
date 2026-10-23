#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <3ds.h>

// Draw a character
void draw_char(u8* fb, int x, int y, char c, u32 color, int screen_width);

// Draw text with proper font
void draw_text(u8* fb, int x, int y, const char *text, u32 color, int screen_width);

#endif // GRAPHICS_H
