#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "ui.h"
#include "mastodon.h"
#include "config.h"

// Global variables
static u8* top_framebuffer = NULL;
static u8* bottom_framebuffer = NULL;

int main() {
    // Initialize all services
    gfxInitDefault();
    
    // Initialize console for bottom screen
    consoleInit(GFX_BOTTOM, NULL);
    
    // Get framebuffers
    top_framebuffer = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
    bottom_framebuffer = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
    
    printf("\x1b[2J\x1b[H"); // Clear console
    printf("fedii - Mastodon client for 3DS\n");
    printf("Initializing...\n");
    
    // Initialize network
    printf("Initializing network...\n");
    network_init();
    
    // Initialize config
    printf("Loading config...\n");
    config_t config;
    memset(&config, 0, sizeof(config));
    
    config_t loaded_config;
    memset(&loaded_config, 0, sizeof(loaded_config));
    
    if (config_load(&loaded_config) == 0) {
        config = loaded_config;
        printf("Config loaded!\n");
    } else {
        printf("No config found. Using defaults.\n");
        printf("Press A to configure.\n");
    }
    
    // Initialize Mastodon client if we have config
    if (config.instance_url && config.access_token) {
        printf("Initializing Mastodon client...\n");
        mastodon_init(config.instance_url, config.access_token);
    }
    
    // Initialize UI
    printf("Initializing UI...\n");
    ui_init(top_framebuffer, bottom_framebuffer);
    
    // Main loop
    printf("\x1b[2J\x1b[H"); // Clear console
    printf("fedii ready!\n");
    printf("Use D-Pad to navigate\n");
    printf("A: Select, B: Back\n");
    
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        
        if (kDown & KEY_START) {
            break; // Exit
        }
        
        // Update UI
        ui_update(kDown, kHeld);
        
        // Render
        ui_render();
        
        // Swap buffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        // Wait for VSync
        gspWaitForVBlank();
    }
    
    // Cleanup
    printf("Shutting down...\n");
    ui_exit();
    mastodon_exit();
    network_exit();
    config_free(&loaded_config);
    gfxExit();
    
    return 0;
}
