#include "ui.h"
#include <3ds.h>
#include <stdio.h>
#include <string.h>

// Current UI state
static ui_state_t current_state = UI_STATE_TIMELINE;

// Framebuffers
static u8* top_framebuffer = NULL;
static u8* bottom_framebuffer = NULL;

// Scroll position
static int scroll_position = 0;

// Selected item
static int selected_item = 0;

void ui_init() {
    // Initialize graphics
    top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    
    // Clear screens
    if (top_framebuffer) {
        memset(top_framebuffer, COLOR_BACKGROUND, SCREEN_WIDTH_TOP * SCREEN_HEIGHT_TOP * 3);
    }
}

void ui_update(u32 kDown) {
    switch (current_state) {
        case UI_STATE_TIMELINE:
            if (kDown & KEY_UP) {
                if (selected_item > 0) selected_item--;
                if (scroll_position > 0) scroll_position--;
            }
            if (kDown & KEY_DOWN) {
                selected_item++;
                scroll_position++;
            }
            if (kDown & KEY_A) {
                // Select toot
            }
            if (kDown & KEY_X) {
                current_state = UI_STATE_COMPOSE;
            }
            break;
            
        case UI_STATE_COMPOSE:
            if (kDown & KEY_B) {
                current_state = UI_STATE_TIMELINE;
            }
            break;
            
        case UI_STATE_SETTINGS:
            if (kDown & KEY_B) {
                current_state = UI_STATE_TIMELINE;
            }
            break;
            
        case UI_STATE_LOGIN:
            break;
    }
}

void ui_render() {
    // Clear top screen with light pink background
    ui_draw_rectangle(0, 0, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, COLOR_BACKGROUND);
    
    // Draw header
    ui_draw_rectangle(0, 0, SCREEN_WIDTH_TOP, 30, COLOR_PINK);
    ui_draw_text(10, 5, "fedii", COLOR_WHITE);
    
    // Render based on state
    switch (current_state) {
        case UI_STATE_TIMELINE:
            ui_draw_text(10, 40, "Timeline", COLOR_DEEP_PINK);
            ui_draw_text(10, 60, "Press X to compose", COLOR_DEEP_PINK);
            
            // Draw sample toots (placeholder)
            ui_draw_toot(10, 80, SCREEN_WIDTH_TOP - 20, "@user1", "This is a sample toot!");
            ui_draw_toot(10, 120, SCREEN_WIDTH_TOP - 20, "@user2", "Hello from the fediverse!");
            ui_draw_toot(10, 160, SCREEN_WIDTH_TOP - 20, "@user3", "Testing fedii on 3DS!");
            break;
            
        case UI_STATE_COMPOSE:
            ui_draw_rectangle(0, 0, SCREEN_WIDTH_TOP, SCREEN_HEIGHT_TOP, COLOR_LIGHT_PINK);
            ui_draw_text(10, 40, "Compose Toot", COLOR_DEEP_PINK);
            ui_draw_text(10, 60, "Type your message here...", COLOR_WHITE);
            ui_draw_text(10, 220, "Press A to post, B to cancel", COLOR_DEEP_PINK);
            break;
            
        case UI_STATE_SETTINGS:
            ui_draw_text(10, 40, "Settings", COLOR_DEEP_PINK);
            ui_draw_text(10, 60, "Instance URL:", COLOR_WHITE);
            ui_draw_text(10, 80, "Username:", COLOR_WHITE);
            break;
            
        case UI_STATE_LOGIN:
            ui_draw_text(10, 40, "Login", COLOR_DEEP_PINK);
            break;
    }
    
    // Draw bottom screen (console output)
    consoleSelect(&console);
}

void ui_exit() {
    // Cleanup
}

void ui_draw_rectangle(int x, int y, int width, int height, u32 color) {
    if (!top_framebuffer) return;
    
    for (int dy = 0; dy < height; dy++) {
        for (int dx = 0; dx < width; dx++) {
            if (x + dx < SCREEN_WIDTH_TOP && y + dy < SCREEN_HEIGHT_TOP) {
                u8* pixel = top_framebuffer + ((y + dy) * SCREEN_WIDTH_TOP + (x + dx)) * 3;
                pixel[0] = (color >> 16) & 0xFF; // R
                pixel[1] = (color >> 8) & 0xFF;  // G
                pixel[2] = color & 0xFF;         // B
            }
        }
    }
}

void ui_draw_text(int x, int y, const char *text, u32 color) {
    // Simple text drawing (would use proper font rendering in full implementation)
    // For now, just print to console
    printf("\x1b[%d;%dH%s\x1b[0m", y/10 + 1, x/6 + 1, text);
}

void ui_draw_toot(int x, int y, int width, const char *author, const char *content) {
    // Draw toot card
    ui_draw_rectangle(x, y, width, 40, COLOR_WHITE);
    ui_draw_rectangle(x, y, width, 1, COLOR_LIGHT_PINK);
    
    // Draw author and content
    ui_draw_text(x + 5, y + 5, author, COLOR_DEEP_PINK);
    ui_draw_text(x + 5, y + 20, content, COLOR_DEEP_PINK);
}
