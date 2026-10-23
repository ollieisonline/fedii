#include "ui.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Forward declaration for font drawing
extern void draw_text(u8* fb, int x, int y, const char *text, u32 color, int screen_width);

// Current UI state
static ui_state_t current_state = UI_STATE_TIMELINE;

// Framebuffers
static u8* top_framebuffer = NULL;
static u8* bottom_framebuffer = NULL;

// Scroll position
static int scroll_position = 0;

// Selected item
static int selected_item = 0;

// Toots buffer
#define MAX_TOOTS 50
static ui_toot_t toots[MAX_TOOTS];
static int toot_count = 0;

// Text input buffer for compose
static char compose_buffer[512] = "";
static int compose_cursor = 0;

// Config input
static char config_instance[256] = "";
static char config_token[256] = "";

// Keyboard state
static SwkbdState swkbd;
static char swkbd_buffer[256];

void ui_init(u8* top_fb, u8* bottom_fb) {
    top_framebuffer = top_fb;
    bottom_framebuffer = bottom_fb;
    
    // Clear screens
    if (top_framebuffer) {
        memset(top_framebuffer, COLOR_BACKGROUND, SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP * 3);
    }
    if (bottom_framebuffer) {
        memset(bottom_framebuffer, 0, SCREEN_WIDTH_BOTTOM * SCREEN_HEIGHT_BOTTOM * 3);
    }
    
    // Initialize software keyboard
    swkbdInit(&swkbd, SWKBD_TYPE_QWERTY, 2, -1);
}

void ui_set_state(ui_state_t state) {
    current_state = state;
    scroll_position = 0;
    selected_item = 0;
}

ui_state_t ui_get_state() {
    return current_state;
}

void ui_add_toot(const char *author, const char *content) {
    if (toot_count >= MAX_TOOTS) {
        // Shift all toots up
        for (int i = 0; i < MAX_TOOTS - 1; i++) {
            toots[i] = toots[i + 1];
        }
        toot_count--;
    }
    
    strncpy(toots[toot_count].author, author, sizeof(toots[toot_count].author) - 1);
    strncpy(toots[toot_count].content, content, sizeof(toots[toot_count].content) - 1);
    toot_count++;
}

void ui_clear_toots() {
    toot_count = 0;
}

void ui_update(u32 kDown, u32 kHeld) {
    switch (current_state) {
        case UI_STATE_TIMELINE:
            if (kDown & KEY_UP) {
                if (selected_item > 0) selected_item--;
                if (scroll_position > 0) scroll_position--;
            }
            if (kDown & KEY_DOWN) {
                if (selected_item < toot_count - 1) selected_item++;
                if (scroll_position < toot_count - 4) scroll_position++;
            }
            if (kDown & KEY_A) {
                // View toot
            }
            if (kDown & KEY_X) {
                current_state = UI_STATE_COMPOSE;
                compose_cursor = 0;
                compose_buffer[0] = '\0';
            }
            if (kDown & KEY_Y) {
                current_state = UI_STATE_SETTINGS;
            }
            break;
            
        case UI_STATE_COMPOSE:
            if (kDown & KEY_B) {
                current_state = UI_STATE_TIMELINE;
            }
            if (kDown & KEY_A) {
                // Post toot
                if (compose_buffer[0] != '\0') {
                    // Would call mastodon_post_status here
                    current_state = UI_STATE_TIMELINE;
                }
            }
            if (kDown & KEY_X) {
                // Open keyboard
                swkbd_buffer[0] = '\0';
                SwkbdResult res = swkbdInputText(&swkbd, swkbd_buffer, sizeof(swkbd_buffer));
                if (res == SWKBD_BUTTON_CONFIRM) {
                    strncpy(compose_buffer, swkbd_buffer, sizeof(compose_buffer) - 1);
                    compose_cursor = strlen(compose_buffer);
                }
            }
            break;
            
        case UI_STATE_SETTINGS:
            if (kDown & KEY_B) {
                current_state = UI_STATE_TIMELINE;
            }
            if (kDown & KEY_A) {
                if (selected_item == 0) {
                    current_state = UI_STATE_CONFIG;
                    selected_item = 0;
                }
            }
            if (kDown & KEY_UP) {
                if (selected_item > 0) selected_item--;
            }
            if (kDown & KEY_DOWN) {
                if (selected_item < 1) selected_item++;
            }
            break;
            
        case UI_STATE_CONFIG:
            if (kDown & KEY_B) {
                current_state = UI_STATE_SETTINGS;
            }
            if (kDown & KEY_A) {
                // Save config
                current_state = UI_STATE_SETTINGS;
            }
            if (kDown & KEY_X) {
                // Open keyboard for selected field
                swkbd_buffer[0] = '\0';
                SwkbdResult res = swkbdInputText(&swkbd, swkbd_buffer, sizeof(swkbd_buffer));
                if (res == SWKBD_BUTTON_CONFIRM) {
                    if (selected_item == 0) {
                        strncpy(config_instance, swkbd_buffer, sizeof(config_instance) - 1);
                    } else {
                        strncpy(config_token, swkbd_buffer, sizeof(config_token) - 1);
                    }
                }
            }
            if (kDown & KEY_UP) {
                if (selected_item > 0) selected_item--;
            }
            if (kDown & KEY_DOWN) {
                if (selected_item < 1) selected_item++;
            }
            break;
            
        case UI_STATE_LOGIN:
            if (kDown & KEY_B) {
                current_state = UI_STATE_TIMELINE;
            }
            break;
    }
}

void ui_render() {
    // Clear top screen with light pink background
    if (top_framebuffer) {
        memset(top_framebuffer, COLOR_BACKGROUND, SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP * 3);
    }
    
    // Draw header
    ui_draw_rectangle(top_framebuffer, 0, 0, SCREEN_WIDTH_TOP, 30, COLOR_PINK, SCREEN_WIDTH_TOP);
    ui_draw_text(10, 5, "fedii", COLOR_WHITE);
    
    // Render based on state
    switch (current_state) {
        case UI_STATE_TIMELINE:
            ui_draw_text(10, 35, "Timeline", COLOR_DEEP_PINK);
            ui_draw_text(10, 218, "X: Compose  Y: Settings", COLOR_TEXT_LIGHT);
            
            // Draw toots
            for (int i = scroll_position; i < toot_count && i < scroll_position + 4; i++) {
                int y = 50 + (i - scroll_position) * 45;
                ui_draw_toot(10, y, SCREEN_WIDTH_TOP - 20, 
                            toots[i].author, toots[i].content);
                
                // Highlight selected
                if (i == selected_item) {
                    ui_draw_rectangle(top_framebuffer, 5, y - 2, SCREEN_WIDTH_TOP - 10, 42, 
                                     COLOR_LIGHT_PINK, SCREEN_WIDTH_TOP);
                }
            }
            
            if (toot_count == 0) {
                ui_draw_text(10, 100, "No toots loaded. Press Y to configure.", COLOR_TEXT);
            }
            break;
            
        case UI_STATE_COMPOSE:
            ui_draw_rectangle(top_framebuffer, 0, 0, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, 
                             COLOR_LIGHT_PINK, SCREEN_WIDTH_TOP);
            ui_draw_text(10, 5, "Compose Toot", COLOR_DEEP_PINK);
            
            // Draw text input area
            ui_draw_rectangle(top_framebuffer, 10, 35, SCREEN_WIDTH_TOP - 20, 160, 
                             COLOR_WHITE, SCREEN_WIDTH_TOP);
            
            // Draw compose text
            ui_draw_text(15, 40, compose_buffer, COLOR_TEXT);
            
            // Draw cursor
            if (compose_cursor >= 0) {
                int cursor_x = 15 + (compose_cursor * 6);
                ui_draw_rectangle(top_framebuffer, cursor_x, 38, 2, 16, 
                                 COLOR_DEEP_PINK, SCREEN_WIDTH_TOP);
            }
            
            ui_draw_text(10, 200, "X: Keyboard  A: Post  B: Cancel", COLOR_TEXT);
            break;
            
        case UI_STATE_SETTINGS:
            ui_draw_text(10, 35, "Settings", COLOR_DEEP_PINK);
            
            ui_draw_text(10, 60, selected_item == 0 ? "> Configure Instance" : "  Configure Instance", 
                        selected_item == 0 ? COLOR_DEEP_PINK : COLOR_TEXT);
            ui_draw_text(10, 80, selected_item == 1 ? "> Back" : "  Back", 
                        selected_item == 1 ? COLOR_DEEP_PINK : COLOR_TEXT);
            
            ui_draw_text(10, 218, "A: Select  B: Back", COLOR_TEXT_LIGHT);
            break;
            
        case UI_STATE_CONFIG:
            ui_draw_text(10, 35, "Configuration", COLOR_DEEP_PINK);
            
            ui_draw_text(10, 60, "Instance URL:", COLOR_TEXT);
            ui_draw_rectangle(top_framebuffer, 10, 75, SCREEN_WIDTH_TOP - 20, 20, 
                             selected_item == 0 ? COLOR_LIGHT_PINK : COLOR_WHITE, SCREEN_WIDTH_TOP);
            ui_draw_text(15, 78, config_instance[0] ? config_instance : "Not set", COLOR_TEXT);
            
            ui_draw_text(10, 100, "Access Token:", COLOR_TEXT);
            ui_draw_rectangle(top_framebuffer, 10, 115, SCREEN_WIDTH_TOP - 20, 20, 
                             selected_item == 1 ? COLOR_LIGHT_PINK : COLOR_WHITE, SCREEN_WIDTH_TOP);
            ui_draw_text(15, 118, config_token[0] ? "********" : "Not set", COLOR_TEXT);
            
            ui_draw_text(10, 218, "X: Edit  A: Save  B: Cancel", COLOR_TEXT_LIGHT);
            break;
            
        case UI_STATE_LOGIN:
            ui_draw_text(10, 100, "Login", COLOR_DEEP_PINK);
            break;
    }
    
    // Draw bottom screen console output
    consoleSelect(&console);
}

void ui_exit() {
    swkbdExit(&swkbd);
}

void ui_draw_rectangle(u8* fb, int x, int y, int width, int height, u32 color, int screen_width) {
    if (!fb) return;
    
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            if (x + dx >= 0 && x + dx < screen_width && y + dy >= 0 && y + dy < SCREEN_HEIGHT_TOP) {
                u8* pixel = fb + ((y + dy) * screen_width + (x + dx)) * 3;
                pixel[0] = (color >> 16) & 0xFF; // R
                pixel[1] = (color >> 8) & 0xFF;  // G
                pixel[2] = color & 0xFF;         // B
            }
        }
    }
}

void ui_draw_text(int x, int y, const char *text, u32 color) {
    // Draw text to top screen using proper font
    if (!top_framebuffer) return;
    draw_text(top_framebuffer, x, y, text, color, SCREEN_WIDTH_TOP);
}

void ui_draw_toot(int x, int y, int width, const char *author, const char *content) {
    // Draw toot card
    ui_draw_rectangle(top_framebuffer, x, y, width, 40, COLOR_WHITE, SCREEN_WIDTH_TOP);
    ui_draw_rectangle(top_framebuffer, x, y, width, 1, COLOR_LIGHT_PINK, SCREEN_WIDTH_TOP);
    
    // Draw author and content
    char author_line[128];
    snprintf(author_line, sizeof(author_line), "@%s", author);
    ui_draw_text(x + 5, y + 5, author_line, COLOR_DEEP_PINK);
    ui_draw_text(x + 5, y + 20, content, COLOR_TEXT);
}
