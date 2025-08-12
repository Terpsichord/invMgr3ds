#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <time.h>

#include "inventory.h"
#include "layout.h"
#include "file.h"
#include "render.h"
#include "folder.h"
#include "main.h"

void cleanup(void) {
    cleanupRender();
}

int main() {
    srand(time(NULL));

    C3D_RenderTarget *top, *bottom;
    initRender(&top, &bottom);
    atexit(cleanup);

    FolderView folderView;
    initFolderView(&folderView);
    loadFolderView(&folderView);

    Inventory inv;
    initInventory(&inv);
    loadInventory(&inv, &folderView);

    Screen screen = SCREEN_FOLDER;

    TouchState touchState;
    ButtonPresses presses = {0, 0, 0, {0}};
    Scroll scroll = {0.0f, 0.0f}, filterScroll = {0.0f, 0.0f};

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();

        if (kDown & KEY_START) break;

        updateUi(&screen, &touchState, &inv, &folderView, &scroll, &presses, &filterScroll);
        render(top, bottom, &inv, screen, scroll, &touchState, &presses, filterScroll, &folderView);
    }

    saveInventory(&inv);
    freeInventory(&inv);

    saveFolderView(&folderView);
    freeFolderView(&folderView);

    return 0;
}
