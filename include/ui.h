#ifndef UI_H
#define UI_H

#include <3ds.h>

// Screen dimensions
#define SCREEN_WIDTH_TOP 400
#define SCREEN_HEIGHT_TOP 240
#define SCREEN_WIDTH_BOTTOM 320
#define SCREEN_HEIGHT_BOTTOM 240

// Colors (Pink and white theme)
#define COLOR_PINK 0xFFC0CB
#define COLOR_WHITE 0xFFFFFF
#define COLOR_LIGHT_PINK 0xFFB6C1
#define COLOR_DEEP_PINK 0xFF1493
#define COLOR_BACKGROUND 0xFFF0F5

// UI states
typedef enum {
    UI_STATE_TIMELINE,
    UI_STATE_COMPOSE,
    UI_STATE_SETTINGS,
    UI_STATE_LOGIN
} ui_state_t;

// Initialize UI
void ui_init();

// Update UI with input
void ui_update(u32 kDown);

// Render UI
void ui_render();

// Exit UI
void ui_exit();

// Draw functions
void ui_draw_rectangle(int x, int y, int width, int height, u32 color);
void ui_draw_text(int x, int y, const char *text, u32 color);
void ui_draw_toot(int x, int y, int width, const char *author, const char *content);

#endif // UI_H
