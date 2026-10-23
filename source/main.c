#include <3ds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ui.h"
#include "mastodon.h"
#include "config.h"

int main() {
    // Initialize services
    gfxInitDefault();
    consoleInit(GFX_BOTTOM, NULL);
    
    printf("fedii - Mastodon client for 3DS\n");
    printf("Initializing...\n");
    
    // Initialize UI
    ui_init();
    
    // Main loop
    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        
        if (kDown & KEY_START) {
            break; // Exit
        }
        
        // Update UI
        ui_update(kDown);
        
        // Render
        ui_render();
        
        // Swap buffers
        gfxFlushBuffers();
        gfxSwapBuffers();
        
        // Wait for VSync
        gspWaitForVBlank();
    }
    
    // Cleanup
    ui_exit();
    gfxExit();
    
    return 0;
}
