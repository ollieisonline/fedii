#ifndef UI_H
#define UI_H

#include <3ds.h>

// Screen dimensions
#define SCREEN_WIDTH_TOP 400
#define SCREEN_HEIGHT_TOP 240
#define SCREEN_WIDTH_BOTTOM 320
#define SCREEN_HEIGHT_BOTTOM 240

// Colors (Pink and white theme)
#define COLOR_PINK 0xFFC0CBFF
#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_LIGHT_PINK 0xFFB6C1FF
#define COLOR_DEEP_PINK 0xFF1493FF
#define COLOR_BACKGROUND 0xFFF0F5FF
#define COLOR_TEXT 0x000000FF
#define COLOR_TEXT_LIGHT 0x696969FF

// UI states
typedef enum {
    UI_STATE_TIMELINE,
    UI_STATE_COMPOSE,
    UI_STATE_SETTINGS,
    UI_STATE_LOGIN,
    UI_STATE_CONFIG
} ui_state_t;

// Toot structure for display
typedef struct {
    char author[64];
    char content[512];
    char timestamp[32];
} ui_toot_t;

// Initialize UI
void ui_init(u8* top_fb, u8* bottom_fb);

// Update UI with input
void ui_update(u32 kDown, u32 kHeld);

// Render UI
void ui_render();

// Exit UI
void ui_exit();

// Draw functions
void ui_draw_rectangle(u8* fb, int x, int y, int width, int height, u32 color, int screen_width);
void ui_draw_text(int x, int y, const char *text, u32 color);
void ui_draw_toot(int x, int y, int width, const char *author, const char *content);

// UI state management
void ui_set_state(ui_state_t state);
ui_state_t ui_get_state();

// Add a toot to display
void ui_add_toot(const char *author, const char *content);

// Clear toots
void ui_clear_toots();

#endif // UI_H
