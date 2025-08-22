#include <stdlib.h>

#include "inventory.h"
#include "layout.h"
#include "render.h"
#include "text.h"

//#define CONSOLE 1
//#define CONSOLE_TOP 1
//#define BORDERS 1


u32 white, lightGray, gray, darkGray, black, darkenScreen, accent, darkAccent, lightAccent, scrollGray, lowBatteryColor;

static u32 bgColor;

void initGfx(C3D_RenderTarget **top, C3D_RenderTarget **bottom) {
    gfxInitDefault();
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);
    C2D_Init(C2D_DEFAULT_MAX_OBJECTS);
    C2D_Prepare();

#ifdef CONSOLE_TOP
    consoleInit(GFX_TOP, NULL);
#else
    *top = C2D_CreateScreenTarget(GFX_TOP, GFX_LEFT);
#endif

#ifdef CONSOLE
    consoleInit(GFX_BOTTOM, NULL);
#else
    *bottom = C2D_CreateScreenTarget(GFX_BOTTOM, GFX_LEFT);
#endif
}

u32 calculateAccentColor(u32 color, u32 accentColor, u8 alpha) {
    u8 aTop = alpha;
    u8 bTop = (accentColor >> 16) & 0xFF;
    u8 gTop = (accentColor >> 8) & 0xFF;
    u8 rTop = (accentColor) & 0xFF;

    u8 aBtm = (color >> 24) & 0xFF;
    u8 bBtm = (color >> 16) & 0xFF;
    u8 gBtm = (color >> 8) & 0xFF;
    u8 rBtm = (color) & 0xFF;

    u8 rOut = (u8)((rTop * aTop + rBtm * (255 - aTop) + 127) / 255);
    u8 gOut = (u8)((gTop * aTop + gBtm * (255 - aTop) + 127) / 255);
    u8 bOut = (u8)((bTop * aTop + bBtm * (255 - aTop) + 127) / 255);

    u8 aOut = (u8)(aTop + ((aBtm * (255 - aTop) + 127) / 255));

    return (aOut << 24) | (bOut << 16) | (gOut << 8) | rOut;
}

void initColors(void) {
    white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    lightGray = C2D_Color32(0xF0, 0xF0, 0xF0, 0xFF);
    gray = C2D_Color32(0xE0, 0xE0, 0xE0, 0xFF);
    darkGray = C2D_Color32(0x80, 0x80, 0x80, 0xFF);
    black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
    darkenScreen = C2D_Color32(0x00, 0x00, 0x00, 0x60);
    scrollGray = C2D_Color32(0x40, 0x40, 0x40, 0x80);
    lowBatteryColor = C2D_Color32(0xC8, 0x00, 0x00, 0xFF);
    bgColor = white;

    accent = calculateAccentColor(white, black, 0x30);
    darkAccent = calculateAccentColor(white, black, 0x50);
    lightAccent = calculateAccentColor(white, lightGray, 0x30);

    folderColors[0] = C2D_Color32(0xFF, 0xB3, 0xBA, 0xFF);
    folderColors[1] = C2D_Color32(0xFF, 0xDF, 0xBA, 0xFF);
    folderColors[2] = C2D_Color32(0xFF, 0xFF, 0xBA, 0xFF);
    folderColors[3] = C2D_Color32(0xBA, 0xFF, 0xC9, 0xFF);
    folderColors[4] = C2D_Color32(0xBA, 0xE1, 0xFF, 0xFF);
    folderColors[5] = C2D_Color32(0xC9, 0xBA, 0xFF, 0xFF);
    folderColors[6] = C2D_Color32(0xFF, 0xBA, 0xFA, 0xFF);
    folderColors[7] = C2D_Color32(0xFF, 0xCC, 0xCC, 0xFF);
    folderColors[8] = C2D_Color32(0xFF, 0xE5, 0xCC, 0xFF);
    folderColors[9] = C2D_Color32(0xFF, 0xFF, 0xCC, 0xFF);
    folderColors[10] = C2D_Color32(0xCC, 0xFF, 0xE5, 0xFF);
    folderColors[11] = C2D_Color32(0xCC, 0xE5, 0xFF, 0xFF);
    folderColors[12] = C2D_Color32(0xE5, 0xCC, 0xFF, 0xFF);
    folderColors[13] = C2D_Color32(0xFF, 0xCC, 0xE5, 0xFF);
    folderColors[14] = C2D_Color32(0xF0, 0xE6, 0xC8, 0xFF);
    folderColors[15] = C2D_Color32(0xDC, 0xF0, 0xC8, 0xFF);
    folderColors[16] = C2D_Color32(0xC8, 0xF0, 0xE6, 0xFF);
    folderColors[17] = C2D_Color32(0xE6, 0xD2, 0xF0, 0xFF);
    folderColors[18] = C2D_Color32(0xF0, 0xD2, 0xE6, 0xFF);
    folderColors[19] = C2D_Color32(0xD2, 0xF0, 0xDC, 0xFF);
}

void initRender(C3D_RenderTarget **top, C3D_RenderTarget **bottom) {
    initGfx(top, bottom);
    initColors();
    initText();
}

void cleanupRender(void) {
    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

// --------------------------------------------------------------

bool showFolder(const Inventory *inv, const FolderView *view, const Folder *folder) {
    for (int i = 0; i < view->currentFolder->numChildren; i++) {
        if (findFolder(view->currentFolder->children[i], folder->name)) {
            return true;
        }
    }
    return false;
}

void drawItemList(const Inventory *inv, const FolderView *view, float scrollOffset) {
    if (numShownItems(inv) == 0) return;

    for (int i = 0; i < numShownItems(inv); i++) {
        if (INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset > SCREEN_HEIGHT ||
            INV_TOP_PAD + ITEM_HEIGHT * (i + 1) - scrollOffset < 0)
            continue;

        u32 bg = bgColor;

        if (inv->selectedIdx == i) {
            bg = accent;
            C2D_DrawRectSolid(0.0f, INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f, TOP_WIDTH, ITEM_HEIGHT, accent);
        }

        C2D_DrawText(&getItem(inv, i)->nameText, 0, TEXT_HPAD, INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                     0.0f, 0.6f,
                     0.6f);

        C2D_DrawRectSolid(NAME_WIDTH - 2 * TEXT_HPAD, INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f,
                          TOP_WIDTH, ITEM_HEIGHT, bg);

        C2D_DrawText(&getItem(inv, i)->qtyText, 0, NAME_WIDTH + TEXT_HPAD,
                     INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                     0.0f, 0.6f, 0.6f);

        float width, indent = NAME_WIDTH + QUANTITY_WIDTH + TAG_SPACING, hashWidth;

        if (!isFolderEmpty(view)) {
            C2D_TextGetDimensions(&hashText, 0.55f, 0.55f, &hashWidth, NULL);

            for (int j = 0; j < getItem(inv, i)->numFolders; j++) {
                if (!showFolder(inv, view, getItem(inv, i)->folders[j])) continue;

                Folder *folder = getItem(inv, i)->folders[j];

                C2D_TextGetDimensions(&folder->text, 0.55f, 0.55f, &width, NULL);
                C2D_DrawRectSolid(indent - TAG_SPACING / 4,
                                  INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset + TEXT_VPAD / 2, 0.0f,
                                  width + hashWidth + TAG_SPACING / 2, ITEM_HEIGHT - TEXT_VPAD * 2, folder->color);

                C2D_DrawText(&hashText, 0, indent, INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                             0.0f, 0.55f, 0.55f);
                indent += hashWidth;
                C2D_DrawText(&folder->text, 0, indent,
                             INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                             0.0f, 0.55f, 0.55f);
                indent += width + TAG_SPACING;
            }
        }

        for (int j = 0; j < getItem(inv, i)->numTags; j++) {
            C2D_TextGetDimensions(&getItem(inv, i)->tagsText[j], 0.55f, 0.55f, &width, NULL);
            C2D_DrawText(&getItem(inv, i)->tagsText[j], 0, indent,
                         INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                         0.0f, 0.55f, 0.55f);
            indent += width + TAG_SPACING;
        }

        C2D_DrawRectSolid(NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH - TEXT_HPAD,
                          INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f,
                          TOP_WIDTH, ITEM_HEIGHT, bg);

        C2D_DrawText(&getItem(inv, i)->descText, 0, NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH + TEXT_HPAD,
                     INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset, 0.0f, 0.55f, 0.55f);
    }
}

void drawGrid(const Inventory *inv, const FolderView *view, float scrollOffset) {
    if (numShownItems(inv) == 0) return;

    for (int i = 0; i < (numShownItems(inv) + GRID_COLUMNS - 1) / GRID_COLUMNS; i++) {
        for (int j = 0; j < GRID_COLUMNS; j++) {
            int idx = i * GRID_COLUMNS + j;
            if (idx >= numShownItems(inv) ||
                GRID_VPAD + GRID_SPACING * i - scrollOffset > SCREEN_HEIGHT ||
                GRID_VPAD + GRID_SPACING * (i + 1) - scrollOffset < 0)
                continue;

            C2D_DrawRectSolid(GRID_HPAD + GRID_SPACING * j, GRID_VPAD + GRID_SPACING * i - scrollOffset, 0.0f,
                              GRID_TILE_SIZE, GRID_TILE_SIZE, accent);

            if (true) { // todo: if has no image
                float width, height, scale;
                C2D_TextGetDimensions(&getItem(inv, idx)->nameText, 1.0f, 1.0f, &width, &height);
                scale = (GRID_TILE_SIZE - 2 * GRID_BORDER - 2 * TEXT_HPAD) / width;
                scale = MIN(scale, 0.7f);

                C2D_DrawText(&getItem(inv, idx)->nameText, C2D_WithColor | C2D_AlignCenter,
                             GRID_HPAD + GRID_SPACING * j + GRID_TILE_SIZE / 2,
                             GRID_VPAD + GRID_SPACING * i - scrollOffset + GRID_TILE_SIZE / 2 - height * scale / 2,
                             0.0f, scale, scale, darkAccent);

            }


            C2D_DrawText(&getItem(inv, idx)->qtyText, C2D_WithColor, GRID_QTY_X + GRID_SPACING * j,
                         GRID_QTY_Y + GRID_SPACING * i - scrollOffset, 0.0f, 0.45f, 0.45f, white);

            if (inv->selectedIdx == idx) {
                C2D_DrawRectSolid(GRID_HPAD + GRID_SPACING * j,
                                  GRID_VPAD + GRID_SPACING * i - scrollOffset, 0.0f,
                                  GRID_TILE_SIZE, GRID_BORDER,
                                  darkAccent);
                C2D_DrawRectSolid(GRID_HPAD + GRID_SPACING * j,
                                  GRID_VPAD + GRID_SPACING * i - scrollOffset,
                                  0.0f, GRID_BORDER,
                                  GRID_TILE_SIZE, darkAccent);
                C2D_DrawRectSolid(GRID_HPAD + GRID_SPACING * j + GRID_TILE_SIZE - GRID_BORDER,
                                  GRID_VPAD + GRID_SPACING * i - scrollOffset,
                                  0.0f, GRID_BORDER,
                                  GRID_TILE_SIZE, darkAccent);
                C2D_DrawRectSolid(GRID_HPAD + GRID_SPACING * j,
                                  GRID_VPAD + GRID_SPACING * i - scrollOffset + GRID_TILE_SIZE - GRID_BORDER,
                                  0.0f, GRID_TILE_SIZE,
                                  GRID_BORDER, darkAccent);
            }
        }
    }
}

static void drawQuantity(const Item *item, const TouchState *touchState) {
    C2D_DrawText(&qtyText, 0, QTY_X, QTY_LABEL_Y, 0.0f, 0.5f, 0.5f);

    C2D_DrawRectSolid(QTY_X, INCR_Y, 0.0f, QTY_WIDTH, QTY_BTN_HEIGHT,
                      touchState->item == TOUCH_INCR ? darkGray : accent);
    C2D_DrawRectSolid(QTY_X, QTY_Y, 0.0f, QTY_WIDTH, QTY_HEIGHT, lightAccent);
    C2D_DrawRectSolid(QTY_X, DECR_Y, 0.0f, QTY_WIDTH, QTY_BTN_HEIGHT,
                      touchState->item == TOUCH_DECR ? darkGray : accent);

    u32 incrColor = touchState->item == TOUCH_INCR ? white : darkAccent;
    C2D_DrawTriangle(QTY_X + QTY_WIDTH / 2 - 8, INCR_Y + QTY_BTN_HEIGHT / 2 + 3, incrColor,
                     QTY_X + QTY_WIDTH / 2 + 8, INCR_Y + QTY_BTN_HEIGHT / 2 + 3, incrColor,
                     QTY_X + QTY_WIDTH / 2, INCR_Y + QTY_BTN_HEIGHT / 2 - 6, incrColor, 0);

    u32 decrColor = touchState->item == TOUCH_DECR ? white : darkAccent;
    C2D_DrawTriangle(QTY_X + QTY_WIDTH / 2 - 8, DECR_Y + QTY_BTN_HEIGHT / 2 - 3, decrColor,
                     QTY_X + QTY_WIDTH / 2 + 8, DECR_Y + QTY_BTN_HEIGHT / 2 - 3, decrColor,
                     QTY_X + QTY_WIDTH / 2, DECR_Y + QTY_BTN_HEIGHT / 2 + 6, decrColor, 0);

    C2D_DrawText(&item->qtyText, 0, QTY_X + QTY_HPAD + TEXT_HPAD, QTY_Y + QTY_VPAD + TEXT_VPAD, 0.0f, 0.6f, 0.6f);
}

static void drawTags(const Item *item) {
    float hashWidth, indent = TAGS_X;
    C2D_TextGetDimensions(&hashText, 0.55f, 0.55f, &hashWidth, NULL);

    int numTags = item->numFolders + item->numTags;
    for (int i = 0; i < (numTags + TAG_ROWS - 1) / TAG_ROWS; i++) {
        float width, maxWidth = 0.0f;
        for (int j = 0; j < TAG_ROWS; j++) {
            int idx = i * TAG_ROWS + j;
            if (idx < item->numFolders) {
                idx = item->numFolders - 1 - idx;
                C2D_TextGetDimensions(&item->folders[idx]->text, 0.55f, 0.55f, &width, NULL);
                maxWidth = MAX(maxWidth, width + hashWidth);

                C2D_DrawRectSolid(indent - TAG_SPACING / 4, TAGS_Y + TAG_HEIGHT * j + TEXT_VPAD / 2, 0.0f,
                                  width + hashWidth + TAG_SPACING / 2, TAG_HEIGHT - 2 * TEXT_VPAD,
                                  item->folders[idx]->color);
                C2D_DrawText(&hashText, 0, indent, TAGS_Y + TAG_HEIGHT * j, 0.0f, 0.55f, 0.55f);
                C2D_DrawText(&item->folders[idx]->text, 0, indent + hashWidth, TAGS_Y + TAG_HEIGHT * j, 0.0f, 0.55f,
                             0.55f);
            } else if (idx < numTags) {
                idx -= item->numFolders;
                C2D_TextGetDimensions(&item->tagsText[idx], 0.55f, 0.55f, &width, NULL);
                maxWidth = MAX(maxWidth, width);

                C2D_DrawText(&item->tagsText[idx], 0, indent, TAGS_Y + TAG_HEIGHT * j, 0.0f, 0.55f, 0.55f);
            }
        }
        indent += maxWidth + TAG_SPACING;
    }
}

static void drawSelected(const Item *item, bool editing, const TouchState *touchState) {
    if (editing) {
        C2D_DrawRectSolid(VIEW_HPAD, VIEW_VPAD + VIEW_TOP_PAD, 0.0f, BOTTOM_WIDTH - 2 * VIEW_HPAD,
                          SCREEN_HEIGHT - 2 * VIEW_VPAD - VIEW_TOP_PAD, accent);
        C2D_DrawRectSolid(VIEW_HPAD + BORDER, VIEW_VPAD + VIEW_TOP_PAD + BORDER, 0.0f,
                          BOTTOM_WIDTH - 2 * VIEW_HPAD - 2 * BORDER,
                          SCREEN_HEIGHT - 2 * VIEW_VPAD - 2 * BORDER - VIEW_TOP_PAD, bgColor);

        float w, h;
        C2D_TextGetDimensions(&item->nameText, 0.6f, 0.6f, &w, &h);
        C2D_DrawRectSolid(VIEW_HPAD + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD,
                          bgColor);
    }

    C2D_DrawText(&item->nameText, 0, VIEW_HPAD + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);
    C2D_DrawText(&item->descText, C2D_WordWrap, DESC_X, DESC_Y, 0.0f, 0.55f, 0.55f, DESC_WIDTH);

    drawTags(item);

    // hacky coverup to stop tags and long words in description going outside their area
    C2D_DrawRectSolid(DESC_X + DESC_WIDTH, DESC_Y, 0.0f, BOTTOM_WIDTH - VIEW_HPAD - BORDER - DESC_X - DESC_WIDTH,
                      SCREEN_HEIGHT - VIEW_VPAD - DESC_Y - BORDER, bgColor);
    C2D_DrawRectSolid(BOTTOM_WIDTH - VIEW_HPAD, DESC_Y, 0.0f, VIEW_HPAD, SCREEN_HEIGHT - VIEW_VPAD - DESC_Y - BORDER,
                      bgColor);
    if (editing) {
        C2D_DrawRectSolid(BOTTOM_WIDTH - VIEW_HPAD - BORDER, DESC_Y, 0.0f, BORDER, SCREEN_HEIGHT - VIEW_VPAD - DESC_Y,
                          accent);
    }

    drawQuantity(item, touchState);
}

void drawSortView(const Inventory *inv) {
    C2D_DrawRectSolid(0.0f, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BOX_AREA_X, SORT_HEIGHT, bgColor);

    C2D_DrawRectSolid(VIEW_HPAD, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, SORT_WIDTH, SORT_HEIGHT, accent);
    C2D_DrawRectSolid(VIEW_HPAD + BORDER, VIEW_TOP_PAD + VIEW_VPAD + BORDER, 0.0f, SORT_WIDTH - 2 * BORDER,
                      SORT_HEIGHT - 2 * BORDER, bgColor);

    float w, h;
    C2D_TextGetDimensions(&sortText, 0.6f, 0.6f, &w, &h);
    C2D_DrawRectSolid(VIEW_HPAD + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD,
                      bgColor);
    C2D_DrawText(&sortText, 0, VIEW_HPAD + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);

    for (int i = 0; i < NUM_SORTS; i++) {
        if (i == inv->sortOrder) {
            C2D_DrawRectSolid(SORT_X, SORT_Y + SORT_SPACING * i, 0.0f, SORT_WIDTH - 2 * BORDER - 2 * FILTER_HPAD,
                              SORT_SPACING, accent);
        }
        C2D_DrawText(&sortTexts[i], 0, SORT_X + TEXT_HPAD, SORT_Y + SORT_SPACING * i + 2 * TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }
}

const C2D_Text *getFilterText(const Inventory *inv, int filter) {
    if (filter == 0) {
        return &outText;
    } else if (filter <= inv->numTags) {
        return &inv->tagsText[filter - 1];
    } else {
        return &inv->folders[filter - inv->numTags - 1]->text;
    }
}

void drawFilterView(const Inventory *inv, ButtonPresses *presses, float scroll) {
    float hashWidth;
    C2D_TextGetDimensions(&hashText, 0.5f, 0.5f, &hashWidth, NULL);

    for (int i = 0; i < inventoryNumFilters(inv); i++) {
        if (BOX_SPACING * (i + 1) - VIEW_VPAD - scroll + 2 * FILTER_VPAD < 0 ||
            BOX_SPACING * i - scroll > BOX_AREA_HEIGHT - VIEW_VPAD - 2 * FILTER_VPAD - BORDER)
            continue;

        float x = BOX_X + BOX_SIZE + 2 * FILTER_HPAD;

        float width;
        C2D_TextGetDimensions(getFilterText(inv, i), 0.5f, 0.5f, &width, NULL);
        width += hashWidth * (i > inv->numTags);

        float tagScroll = 0.0f;
        if (presses->heldFilterTag == i) {
            if (width > BOX_AREA_WIDTH - 2 * BORDER - 2 * FILTER_HPAD - BOX_SIZE - 2 * FILTER_HPAD) {
                tagScroll = MAX(presses->tagHeldFrames - FILTER_SCROLL_FRAMES, 0);
                if (tagScroll > width + 2 * FILTER_HPAD) {
                    presses->tagHeldFrames -= width + 2 * FILTER_HPAD;
                }
                x -= tagScroll;
            }
        }

        if (i > inv->numTags) {
            C2D_DrawRectSolid(BOX_X + BOX_SIZE + FILTER_HPAD,
                              BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD / 2 - scroll, 0.0f,
                              width + 2 * FILTER_HPAD, TAG_BG_HEIGHT,
                              inv->folders[i - inv->numTags - 1]->color);
            C2D_DrawText(&hashText, 0, x, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD - scroll, 0.0f, 0.5f, 0.5f);
            if (tagScroll > 0.0f) {
                C2D_DrawText(&hashText, 0, x + width + 2 * FILTER_HPAD, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD - scroll, 0.0f, 0.5f, 0.5f);
            }
            x += hashWidth;
        }

        C2D_DrawText(getFilterText(inv, i), 0, x, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD - scroll,
                     0.0f, 0.5f, 0.5f);
        if (tagScroll > 0.0f) {
            C2D_DrawText(getFilterText(inv, i), 0, x + width + 2 * FILTER_HPAD, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD - scroll,
                         0.0f, 0.5f, 0.5f);
        }

        C2D_DrawRectSolid(BOX_X - FILTER_HPAD, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD / 2 - scroll, 0.0f, BOX_SIZE + 2 * FILTER_HPAD, TAG_BG_HEIGHT, bgColor);
        C2D_DrawRectSolid(BOX_X, BOX_Y + BOX_SPACING * i - scroll, 0.0f, BOX_SIZE, BOX_SIZE, black);
        if (!presses->filters[i]) {
            C2D_DrawRectSolid(BOX_X + 1, BOX_Y + BOX_SPACING * i + 1 - scroll, 0.0f, BOX_SIZE - 2, BOX_SIZE - 2,
                              bgColor);
        }
    }

    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BOX_AREA_WIDTH, BORDER, accent);

    float w, h;
    C2D_TextGetDimensions(&filterText, 0.6f, 0.6f, &w, &h);
    C2D_DrawRectSolid(BOX_AREA_X + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD,
                      bgColor);
    C2D_DrawText(&filterText, 0, BOX_AREA_X + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);

    // covers
    C2D_DrawRectSolid(BOX_AREA_X + BORDER + 3 * TEXT_HPAD + w, VIEW_TOP_PAD + VIEW_VPAD, 0.0f,
                      BOX_AREA_WIDTH - BORDER - 3 * TEXT_HPAD - w, BORDER, accent);
    C2D_DrawRectSolid(BOX_AREA_X + BORDER + 3 * TEXT_HPAD + w, VIEW_TOP_PAD + VIEW_VPAD + BORDER, 0.0f,
                      BOX_AREA_WIDTH - 2 * BORDER - 3 * TEXT_HPAD - w, h + 2 * TEXT_VPAD - VIEW_VPAD - BORDER, bgColor);

    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BORDER, BOX_AREA_HEIGHT, accent);
    C2D_DrawRectSolid(BOX_AREA_X + BOX_AREA_WIDTH - BORDER, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BORDER, BOX_AREA_HEIGHT,
                      accent);
    C2D_DrawRectSolid(BOX_AREA_X + BOX_AREA_WIDTH, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BOTTOM_WIDTH, SCREEN_HEIGHT, bgColor);
    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD + BOX_AREA_HEIGHT - BORDER, 0.0f, BOX_AREA_WIDTH, BORDER,
                      accent);
    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD + BOX_AREA_HEIGHT, 0.0f, BOX_AREA_WIDTH, SCREEN_HEIGHT,
                      bgColor);
}

void drawCross(float x, float y, float size, float t, u32 color) {
    float cx = x + size / 2.0f;
    float cy = y + size / 2.0f;

    float dx = size / 2.0f;
    float dy = size / 2.0f;

    float diag_len = sqrtf(dx * dx + dy * dy);
    float tx = t * dx / diag_len;
    float ty = t * dy / diag_len;

    // Top left to bottom right (\)
    C2D_DrawTriangle(
            cx - dx - tx, cy - dy + ty, color,
            cx + dx - tx, cy + dy + ty, color,
            cx - dx + tx, cy - dy - ty, color,
            0
    );
    C2D_DrawTriangle(
            cx - dx + tx, cy - dy - ty, color,
            cx + dx - tx, cy + dy + ty, color,
            cx + dx + tx, cy + dy - ty, color,
            0
    );

    // Bottom left to top right (/)
    C2D_DrawTriangle(
            cx + dx - tx, cy - dy - ty, color,
            cx - dx - tx, cy + dy - ty, color,
            cx + dx + tx, cy - dy + ty, color,
            0
    );
    C2D_DrawTriangle(
            cx + dx + tx, cy - dy + ty, color,
            cx - dx - tx, cy + dy - ty, color,
            cx - dx + tx, cy + dy + ty, color,
            0
    );
}

void drawSearchBar(const Inventory *inv) {
    C2D_DrawRectSolid(SEARCH_X, SEARCH_Y, 0.0f, SEARCH_WIDTH, SEARCH_HEIGHT, accent);
    C2D_DrawRectSolid(SEARCH_X + BORDER, SEARCH_Y + BORDER, 0.0f, SEARCH_WIDTH - 2 * BORDER, SEARCH_HEIGHT - 2 * BORDER,
                      bgColor);

    const C2D_Text *text = hasQuery(inv) ? &inv->searchQueryText : &searchText;
    C2D_DrawText(text, 0, SEARCH_X + BORDER + TEXT_HPAD, SEARCH_Y + BORDER + TEXT_VPAD, 0.0f, 0.6f, 0.6f);

    if (!hasQuery(inv)) return;
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH - SEARCH_HEIGHT + BORDER - TEXT_HPAD, SEARCH_Y + BORDER, 0.0f,
                      SEARCH_HEIGHT - 2 * BORDER + TEXT_HPAD, SEARCH_HEIGHT - 2 * BORDER, bgColor);
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH - BORDER, SEARCH_Y, 0.0f, BORDER, SEARCH_HEIGHT, accent);
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH, SEARCH_Y, 0.0f, SEARCH_X, SEARCH_HEIGHT, bgColor);
    drawCross(SEARCH_X + SEARCH_WIDTH - SEARCH_HEIGHT + BORDER + SEARCH_PAD, SEARCH_Y + BORDER + SEARCH_PAD,
              SEARCH_HEIGHT - 2 * BORDER - 2 * SEARCH_PAD - 1.0f, 1.0f, black);
}

void drawItemView(const Inventory *inv, bool editing, const TouchState *touchState) {
    drawSelected(getSelectedItem(inv), editing, touchState);
}

bool showFilterBar(const Inventory *inv, Screen screen) {
    return screen != SCREEN_FOLDER && screen != SCREEN_DELETE_FOLDER &&
           (inv->numSelectedFilters > 0 || inv->sortOrder != SORT_NONE || hasQuery(inv));
}

float drawSingleFilter(const Inventory *inv, int filterIdx, float indent, bool final) {
    float filterWidth, commaWidth, hashWidth, width = 0.0f;
    C2D_TextGetDimensions(getFilterText(inv, inv->filters[filterIdx]), 0.5f, 0.5f, &filterWidth, NULL);
    C2D_TextGetDimensions(&commaText, 0.5f, 0.5f, &commaWidth, NULL);
    C2D_TextGetDimensions(&hashText, 0.5f, 0.5f, &hashWidth, NULL);

    bool isFolder = inv->filters[filterIdx] > inv->numTags;
    if (isFolder) {
        C2D_DrawRectSolid(indent - FILTER_HPAD,
                          SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD / 2.0f, 0.0f,
                          filterWidth + hashWidth + 2 * FILTER_HPAD, TAG_BG_HEIGHT,
                          inv->folders[inv->filters[filterIdx] - inv->numTags - 1]->color);
        C2D_DrawText(&hashText, 0, indent, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);

        width += hashWidth;
    }

    C2D_DrawText(getFilterText(inv, inv->filters[filterIdx]), 0, indent + width,
                 SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f,
                 0.5f);
    width += filterWidth;
    if (!final) {
        if (isFolder) width += FILTER_HPAD;
        C2D_DrawText(&commaText, 0, indent + width, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
        width += commaWidth + 2 * TEXT_HPAD;
    }

    return width;
}

void drawFilterBar(const Inventory *inv, Screen screen) {
    if (!showFilterBar(inv, screen)) return;

    C2D_DrawRectSolid(0.0f, SCREEN_HEIGHT - BAR_HEIGHT, 0.0f, TOP_WIDTH, BAR_BORDER, black);
    C2D_DrawRectSolid(0.0f, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER, 0.0f, TOP_WIDTH, BAR_HEIGHT - BAR_BORDER, bgColor);

    float quotesWidth;
    C2D_TextGetDimensions(&quotesText, 0.5f, 0.5f, &quotesWidth, NULL);

    float indent = 2 * TEXT_HPAD;
    if (inv->numSelectedFilters > 0) {
        for (int i = 0; i < inv->numSelectedFilters - 1; i++) {
            float width = drawSingleFilter(inv, i, indent, false);
            indent += width;
        }
        drawSingleFilter(inv, inv->numSelectedFilters - 1, indent, true);
    }

    if (inv->sortOrder == SORT_NONE && !hasQuery(inv)) return;

    float sortEnd = TOP_WIDTH - TEXT_HPAD;
    float queryEnd = sortEnd;

    float sortWidth, queryWidth = 0.0f, coverStart = sortEnd;
    if (inv->sortOrder != SORT_NONE) {
        C2D_TextGetDimensions(&sortTexts[inv->sortOrder], 0.5f, 0.5f, &sortWidth, NULL);
        queryEnd -= sortWidth + 3 * TEXT_HPAD;
        coverStart -= sortWidth + 2 * TEXT_HPAD;
    }
    if (hasQuery(inv)) {
        C2D_TextGetDimensions(&inv->searchQueryText, 0.5f, 0.5f, &queryWidth, NULL);
        queryWidth += 2 * quotesWidth;
        coverStart = queryEnd - queryWidth - 2 * TEXT_HPAD;
    }


    C2D_DrawRectSolid(coverStart, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER, 0.0f, TOP_WIDTH, BAR_HEIGHT - BAR_BORDER,
                      bgColor);

    if (inv->sortOrder != SORT_NONE) {
        C2D_DrawText(&sortTexts[inv->sortOrder], 0, sortEnd - sortWidth,
                     SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }

    if (hasQuery(inv)) {
        C2D_DrawText(&quotesText, 0, queryEnd - queryWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f,
                     0.5f, 0.5f);
        C2D_DrawText(&inv->searchQueryText, 0, queryEnd - queryWidth + quotesWidth,
                     SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
        C2D_DrawText(&quotesText, 0, queryEnd - quotesWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f,
                     0.5f, 0.5f);
    }
}

// -----------------------------------------------------------------------------

void drawFolderList(const FolderView *view/*, float scrollOffset*/) {
    if (view->currentFolder != view->rootFolder) {
        C2D_DrawText(&backText, 0, TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }

    if (isFolderEmpty(view)) {
        C2D_DrawText(&emptyText, 0, TEXT_HPAD, INV_TOP_PAD + TEXT_VPAD, 0.0f, 0.6f, 0.6f);
        if (isEmptyRoot(view)) {
            C2D_DrawText(&emptyRootText, 0, TEXT_HPAD, INV_TOP_PAD + TEXT_VPAD + 30.0f * 0.6f, 0.0f, 0.6f, 0.6f);
        }
        return;
    }

    for (int i = 0; i < view->currentFolder->numChildren; i++) {
        if (view->selectedIdx == i) {
            C2D_DrawRectSolid(0.0f, INV_TOP_PAD + TEXT_VPAD + i * FOLDER_SPACING, 0.0f, TOP_WIDTH, FOLDER_SPACING,
                              black);
            C2D_DrawRectSolid(0.0f, INV_TOP_PAD + TEXT_VPAD + i * FOLDER_SPACING + 1, 0.0f, TOP_WIDTH,
                              FOLDER_SPACING - 2, view->currentFolder->color);
        }

        float width;
        C2D_TextGetDimensions(&view->currentFolder->children[i]->text, 0.6f, 0.6f, &width, NULL);
        C2D_DrawText(&view->currentFolder->children[i]->text, 0, TEXT_HPAD,
                     INV_TOP_PAD + TEXT_VPAD + i * FOLDER_SPACING, 0.0f, 0.6f, 0.6f);

        C2D_DrawText(&slashText, 0, TEXT_HPAD + width, INV_TOP_PAD + TEXT_VPAD + i * FOLDER_SPACING, 0.0f, 0.6f, 0.6f);
    }
}

void drawHeaders(u32 bgColor) {
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, INV_TOP_PAD, bgColor);

#ifdef BORDERS
    C2D_DrawRectSolid(0.0f, INV_TOP_PAD, 0.0f, TOP_WIDTH, 1.0f, black);

    C2D_DrawRectSolid(NAME_WIDTH,  0.0f, 0.0f, 1.0f, SCREEN_HEIGHT, black);
    C2D_DrawRectSolid(NAME_WIDTH + QUANTITY_WIDTH, 0.0f, 0.0f, 1.0f, SCREEN_HEIGHT, black);
    C2D_DrawRectSolid(NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH, 0.0f, 0.0f, 1.0f, SCREEN_HEIGHT, black);
#endif

    C2D_DrawText(&nameText, 0, TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&qtyText, 0, NAME_WIDTH + TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&tagsText, 0, NAME_WIDTH + QUANTITY_WIDTH + TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&descText, 0, NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH + TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
}

void drawHintText(Screen screen, bool folderEmpty) {
    C2D_Text *text = NULL;

    switch (screen) {
        case SCREEN_VIEW:
            text = &viewHintText;
            break;
        case SCREEN_EDIT:
        case SCREEN_DELETE:
            text = &editHintText;
            break;
        case SCREEN_FILTER:
            text = &filterHintText;
            break;
        case SCREEN_FOLDER:
            if (folderEmpty) {
                text = &emptyHintText;
                break;
            }
        case SCREEN_DELETE_FOLDER:
            text = &folderHintText;
            break;
        case SCREEN_FILTER_FOLDER:
            text = &filterFolderHintText;
            break;
        default:
            return;
    }

    C2D_DrawText(text, 0, TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawRectSolid(0, 2 * TEXT_VPAD + 15.0f, 0.0f, BOTTOM_WIDTH, 1.0f, black);
}

void drawHamburger(const TouchState *touchState) {
    bool pressed = touchState->item == TOUCH_BURGER;
    C2D_DrawRectSolid(BOTTOM_WIDTH - BURGER_SIZE - 1, 0.0f, 0.0f, BURGER_SIZE + 1, BURGER_SIZE + 1, black);
    C2D_DrawRectSolid(BOTTOM_WIDTH - BURGER_SIZE, 0.0f, 0.0f, BURGER_SIZE, BURGER_SIZE, pressed ? accent : bgColor);

    C2D_DrawRectSolid(BOTTOM_WIDTH - BURGER_SIZE + BURGER_HPAD, BURGER_VPAD, 0.0f, BURGER_SIZE - 2 * BURGER_HPAD - 1,
                      BURGER_HEIGHT, black);
    C2D_DrawRectSolid(BOTTOM_WIDTH - BURGER_SIZE + BURGER_HPAD, BURGER_VPAD + BURGER_HEIGHT + BURGER_SPACING, 0.0f,
                      BURGER_SIZE - 2 * BURGER_HPAD - 1, BURGER_HEIGHT, black);
    C2D_DrawRectSolid(BOTTOM_WIDTH - BURGER_SIZE + BURGER_HPAD, BURGER_VPAD + 2 * (BURGER_HEIGHT + BURGER_SPACING),
                      0.0f, BURGER_SIZE - 2 * BURGER_HPAD - 1, BURGER_HEIGHT, black);
}

void drawDeleteModal(C2D_Text *name) {
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, BOTTOM_WIDTH, SCREEN_HEIGHT, darkenScreen);
    C2D_DrawRectSolid(30.0f, 30.0f, 0.0f, BOTTOM_WIDTH - 60.0f, SCREEN_HEIGHT - 60.0f, bgColor);

    C2D_DrawText(&confirmationText, C2D_AlignCenter, BOTTOM_WIDTH / 2, 60.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&deleteText, 0, 70.0f, 185.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&cancelText, 0, 190.0f, 185.0f, 0.0f, 0.5f, 0.5f);

    C2D_DrawText(name, C2D_AlignCenter, BOTTOM_WIDTH / 2, 120.0f, 0.0f, 0.9f, 0.9f);
}

void drawRowButtons(const TouchState *touchState) {
    C2D_DrawRectSolid(RENAME_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_RENAME ? darkGray : accent);
    C2D_DrawRectSolid(EDIT_TAGS_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_TAGS ? darkGray : accent);
    C2D_DrawRectSolid(EDIT_DESC_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_DESC ? darkGray : accent);
    C2D_DrawRectSolid(DELETE_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_DELETE ? darkGray : accent);

    C2D_DrawText(&renameText, C2D_AlignCenter | C2D_WithColor, RENAME_X + ROW_BTN_WIDTH / 2, ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_RENAME ? white : black);
    C2D_DrawText(&editTagsText, C2D_AlignCenter | C2D_WithColor, EDIT_TAGS_X + ROW_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_TAGS ? white : black);
    C2D_DrawText(&editDescText, C2D_AlignCenter | C2D_WithColor, EDIT_DESC_X + ROW_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_DESC ? white : black);
    C2D_DrawText(&deleteText, C2D_AlignCenter | C2D_WithColor, DELETE_X + ROW_BTN_WIDTH / 2, ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_DELETE ? white : black);
}

void drawFolderButtons(const TouchState *touchState) {
    TouchItem item = touchState->item;

    C2D_DrawRectSolid(NEW_FOLDER_X, ROW_BTN_Y, 0.0f, FOLDER_BTN_WIDTH, ROW_BTN_HEIGHT,
                      item == TOUCH_NEW_FOLDER ? darkGray : accent);
    C2D_DrawRectSolid(RENAME_FOLDER_X, ROW_BTN_Y, 0.0f, FOLDER_BTN_WIDTH, ROW_BTN_HEIGHT,
                      item == TOUCH_RENAME_FOLDER ? darkGray : accent);
    C2D_DrawRectSolid(COLOR_FOLDER_X, ROW_BTN_Y, 0.0f, FOLDER_BTN_WIDTH, ROW_BTN_HEIGHT,
                      item == TOUCH_COLOR_FOLDER ? darkGray : accent);
    C2D_DrawRectSolid(DELETE_FOLDER_X, ROW_BTN_Y, 0.0f, FOLDER_BTN_WIDTH, ROW_BTN_HEIGHT,
                      item == TOUCH_DELETE_FOLDER ? darkGray : accent);

    C2D_DrawText(&newFolderText, C2D_AlignCenter | C2D_WithColor, NEW_FOLDER_X + FOLDER_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, item == TOUCH_NEW_FOLDER ? white : black);
    C2D_DrawText(&renameText, C2D_AlignCenter | C2D_WithColor, RENAME_FOLDER_X + FOLDER_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, item == TOUCH_RENAME_FOLDER ? white : black);
    C2D_DrawText(&colorFolderText, C2D_AlignCenter | C2D_WithColor, COLOR_FOLDER_X + FOLDER_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, item == TOUCH_COLOR_FOLDER ? white : black);
    C2D_DrawText(&deleteFolderText, C2D_AlignCenter | C2D_WithColor, DELETE_FOLDER_X + FOLDER_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, item == TOUCH_DELETE_FOLDER ? white : black);
}

void drawEmptyFolderButtons(const TouchState *touchState) {
    C2D_DrawRectSolid(NEW_EMPTY_X, ROW_BTN_Y, 0.0f, EMPTY_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_NEW_FOLDER ? darkGray : accent);
    C2D_DrawRectSolid(COLOR_EMPTY_X, ROW_BTN_Y, 0.0f, EMPTY_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_COLOR_FOLDER ? darkGray : accent);

    C2D_DrawText(&newFolderText, C2D_AlignCenter | C2D_WithColor, NEW_FOLDER_X + EMPTY_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_NEW_FOLDER ? white : black);
    C2D_DrawText(&colorFolderText, C2D_AlignCenter | C2D_WithColor, COLOR_FOLDER_X + EMPTY_BTN_WIDTH / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_COLOR_FOLDER ? white : black);
}

void drawViewButton(const TouchState *touchState) {
    float width = DELETE_X - NEW_FOLDER_X + FOLDER_BTN_WIDTH;
    C2D_DrawRectSolid(NEW_FOLDER_X, ROW_BTN_Y, 0.0f, width, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_COLOR_FOLDER ? darkGray : accent);

    C2D_DrawText(&colorFolderText, C2D_AlignCenter | C2D_WithColor, NEW_FOLDER_X + width / 2,
                 ROW_BTN_Y + 2 * TEXT_VPAD,
                 0.0f, 0.5f, 0.5f, touchState->item == TOUCH_COLOR_FOLDER ? white : black);
}

u32 hsvToRgb(float h, float s, float v) {
    float c = v * s;
    float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
    float m = v - c;

    float r, g, b;
    if (h < 60) {
        r = c;
        g = x;
        b = 0;
    } else if (h < 120) {
        r = x;
        g = c;
        b = 0;
    } else if (h < 180) {
        r = 0;
        g = c;
        b = x;
    } else if (h < 240) {
        r = 0;
        g = x;
        b = c;
    } else if (h < 300) {
        r = x;
        g = 0;
        b = c;
    } else {
        r = c;
        g = 0;
        b = x;
    }

    return C2D_Color32(
            (u8)((r + m) * 0xFF),
            (u8)((g + m) * 0xFF),
            (u8)((b + m) * 0xFF),
            0xFF
    );
}

void rgbToHsv(u32 color, float *h, float *s, float *v) {
    float r = ((color) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = ((color >> 16) & 0xFF) / 255.0f;

    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));
    float delta = max - min;

    *v = max;

    if (max == 0) {
        *s = 0;
        *h = 0;
        return;
    }

    *s = delta / max;

    if (delta == 0) {
        *h = 0;
        return;
    }

    if (max == r) {
        *h = 60.0f * fmodf(((g - b) / delta), 6.0f);
    } else if (max == g) {
        *h = 60.0f * (((b - r) / delta) + 2.0f);
    } else { // max == b
        *h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    if (*h < 0) *h += 360.0f;
}

void drawColorBar(float x, float y, float w, float h, float hue) {
    int steps = (int) h;
    for (int i = 0; i < steps; i++) {
        float hue1 = ((float) i / steps) * 360.0f;
        float hue2 = ((float) (i + 1) / steps) * 360.0f;

        uint32_t c1 = hsvToRgb(hue1, 1.0f, 1.0f);
        uint32_t c2 = hsvToRgb(hue2, 1.0f, 1.0f);

        float y0 = y + i;
        float y1 = y + i + 1;

        C2D_DrawTriangle(x, y0, c1, x + w, y0, c1, x + w, y1, c2, 0.0f);
        C2D_DrawTriangle(x, y0, c1, x + w, y1, c2, x, y1, c2, 0.0f);
    }

    C2D_DrawRectSolid(x, y + h * hue / 360.0f - 1.0f, 0.0f, w, 3.0f, white);
}

void drawColorView(const TouchState *touchState) {
    const ColorState *color = &touchState->color;

    C2D_DrawRectSolid(COLOR_X - COLOR_PAD - BORDER, COLOR_Y - COLOR_PAD - BORDER, 0.0f,
                      COLOR_SIZE + COLOR_BAR_WIDTH + 3 * COLOR_PAD + 2 * BORDER,
                      COLOR_SIZE + 2 * COLOR_PAD + 2 * BORDER, accent);
    C2D_DrawRectSolid(COLOR_X - COLOR_PAD, COLOR_Y - COLOR_PAD, 0.0f, COLOR_SIZE + COLOR_BAR_WIDTH + 3 * COLOR_PAD,
                      COLOR_SIZE + 2 * COLOR_PAD, white);
    C2D_DrawRectangle(COLOR_X, COLOR_Y, 0.0f, COLOR_SIZE, COLOR_SIZE, white,
                      hsvToRgb(color->hue, 1.0f, 1.0f), black, black);
    drawColorBar(COLOR_X + COLOR_SIZE + COLOR_PAD, COLOR_Y, COLOR_BAR_WIDTH, COLOR_SIZE, color->hue);

    C2D_DrawRectSolid(COLOR_X + color->saturation * COLOR_SIZE - 5.0f,
                      COLOR_Y + (1.0f - color->value) * COLOR_SIZE - 1.0f, 0.0f, 10.0f, 2.0f, white);
    C2D_DrawRectSolid(COLOR_X + color->saturation * COLOR_SIZE - 1.0f,
                      COLOR_Y + (1.0f - color->value) * COLOR_SIZE - 5.0f, 0.0f, 2.0f, 10.0f, white);

    C2D_DrawText(&cancelText, C2D_AlignCenter, COLOR_CANCEL_X, COLOR_OPT_Y, 0.0f, 0.5f, 0.5f, black);
    C2D_DrawText(&confirmText, C2D_AlignCenter, COLOR_CONFIRM_X, COLOR_OPT_Y, 0.0f, 0.5f, 0.5f, black);
}

void drawOptionsView(DisplayMode display) {
    C2D_DrawText(&backText, 0, TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawRectSolid(0, 2 * TEXT_VPAD + 15.0f, 0.0f, BOTTOM_WIDTH, 1.0f, black);

    C2D_DrawRectSolid(OPTIONS_HPAD, OPTIONS_VPAD, 0.0f, OPTION_BOX_SIZE, OPTION_BOX_SIZE, black);
    if (display == DISPLAY_LIST) {
        C2D_DrawRectSolid(OPTIONS_HPAD + 2.0f, OPTIONS_VPAD + 2.0f, 0.0f, OPTION_BOX_SIZE - 4.0f,
                          OPTION_BOX_SIZE - 4.0f, bgColor);
    }

    C2D_DrawText(&gridOptionText, 0, OPTIONS_HPAD + OPTION_BOX_SIZE + OPTION_TEXT_PAD,
                 OPTIONS_VPAD + OPTION_BOX_SIZE / 2 - 10.0f, 0.0f, 0.6f, 0.6f, black);
}

void drawScrollBar(Scroll scroll, float width, float maxHeight, float heightPortion, float x, float y) {
    float height = heightPortion / (scroll.max + heightPortion) * maxHeight;
    float offset = y + scroll.offset / scroll.max * (maxHeight - height);

    C2D_DrawRectSolid(x + width - 6.0f, offset, 0.0f, 2.0f, height, scrollGray);
}

void drawBatteryIndicator() {
    static u8 prevLevel = 100;
    static char levelString[5] = "100%";
    static C2D_TextBuf textBuf;
    static C2D_Text text;
    static bool init = false;

    if (!init) {
        mcuHwcInit();
        textBuf = C2D_TextBufNew(4);
        C2D_TextParse(&text, textBuf, levelString);
        C2D_TextOptimize(&text);
        init = true;
    }

    u8 level = 0;
    MCUHWC_GetBatteryLevel(&level);

    if (level != prevLevel) {
        snprintf(levelString, sizeof(levelString), "%3d%%", level);
        C2D_TextBufClear(textBuf);
        C2D_TextParse(&text, textBuf, levelString);
        C2D_TextOptimize(&text);
        prevLevel = level;
    }

    const float width = 32.0f, height = 12.0f;
    C2D_DrawRectSolid(TOP_WIDTH - width - 1, 1.0f, 0.0f, width, height, black);
    C2D_DrawRectSolid(TOP_WIDTH - width, 2.0f, 0.0f, width - 2, height - 2, level > 10 ? bgColor : lowBatteryColor);
    C2D_DrawRectSolid(TOP_WIDTH - width - 3, 4.0f, 0.0f, 2.0f, height - 6.0f, black);
    C2D_DrawText(&text, C2D_WithColor | C2D_AlignRight, TOP_WIDTH - 3.0f, 1.0f, 0.0f, 0.4f, 0.4f,
                 level > 10 ? black : white);
}

// -----------------------------------------------------------------------------

static void
drawTopScreen(C3D_RenderTarget *top, const Inventory *inv, Screen screen, DisplayMode display, Scroll listScroll,
              Scroll gridScroll, const FolderView *view, const TouchState *touchState) {
#ifndef CONSOLE_TOP
    C2D_TargetClear(top, bgColor);

    C2D_SceneBegin(top);
    {
        if (screen == SCREEN_FOLDER || screen == SCREEN_DELETE_FOLDER || (screen == SCREEN_COLOR && touchState->prevScreen == SCREEN_FOLDER)) {
            drawFolderList(view/*, scroll.offset*/);
        } else {
            if (display == DISPLAY_LIST) {
                drawItemList(inv, view, listScroll.offset);
                drawHeaders(bgColor);
                if (listScroll.max > 0) {
                    drawScrollBar(listScroll, TOP_WIDTH,
                                  SCREEN_HEIGHT - INV_TOP_PAD - 2 * TEXT_VPAD - showFilterBar(inv, screen) * BAR_HEIGHT,
                                  SCREEN_HEIGHT - INV_TOP_PAD - showFilterBar(inv, screen) * BAR_HEIGHT, 0.0f,
                                  INV_TOP_PAD + TEXT_VPAD);
                }
            } else {
                drawGrid(inv, view, gridScroll.offset);
                if (gridScroll.max > 0) {
                    drawScrollBar(gridScroll, TOP_WIDTH, SCREEN_HEIGHT - GRID_VPAD - 2 * TEXT_VPAD - showFilterBar(inv, screen) * BAR_HEIGHT,
                                  SCREEN_HEIGHT - GRID_VPAD, 0.0f, GRID_VPAD);
                }
            }
        }


        drawFilterBar(inv, screen);

        drawBatteryIndicator();

        if (screen == SCREEN_DELETE || screen == SCREEN_DELETE_FOLDER) {
            C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, SCREEN_HEIGHT, darkenScreen);
        }
    }
#endif
}

void drawBottomScreen(C3D_RenderTarget *bottom, const Inventory *inv, Screen screen, DisplayMode display,
                      bool optionScreen, const TouchState *touchState, Scroll filterScroll,
                      ButtonPresses *presses, const FolderView *view) {
#ifndef CONSOLE
    C2D_TargetClear(bottom, bgColor);

    C2D_SceneBegin(bottom);
    {
        TouchState state = *touchState;
        if (presses->incrFrames > 0) state.item = TOUCH_INCR;
        if (presses->decrFrames > 0) state.item = TOUCH_DECR;

        if (optionScreen) {
            drawOptionsView(display);
            return;
        }

        switch (screen) {
            case SCREEN_FOLDER:
            case SCREEN_DELETE_FOLDER:
                if (isFolderEmpty(view)) {
                    drawEmptyFolderButtons(&state);
                } else {
                    drawFolderButtons(&state);
                }
                break;
            case SCREEN_FILTER:
            case SCREEN_FILTER_FOLDER:
                drawFilterView(inv, presses, filterScroll.offset);
                drawSortView(inv);
                if (filterScroll.max > 0.0f) {
                    drawScrollBar(filterScroll, BOX_AREA_WIDTH - BORDER, BOX_AREA_HEIGHT - 0.6 * 30.0f - 2 * TEXT_VPAD,
                                  BOX_AREA_HEIGHT, BOX_AREA_X, VIEW_TOP_PAD + 0.6 * 30.0f + 2 * TEXT_VPAD);
                }
                drawSearchBar(inv);
                break;
            case SCREEN_VIEW:
            case SCREEN_EDIT:
            case SCREEN_DELETE:
                if (numShownItems(inv) > 0) {
                    drawItemView(inv, screen == SCREEN_EDIT || screen == SCREEN_DELETE, &state);
                }
                break;
            case SCREEN_COLOR:
                drawColorView(touchState);
                return;
            default:
                return;
        }

        if (screen == SCREEN_VIEW) {
            drawViewButton(&state);
        }

        if (!isEmptyRoot(view)) {
            drawHintText(screen, isFolderEmpty(view));
        }

        if (screen == SCREEN_EDIT || screen == SCREEN_DELETE) {
            drawRowButtons(&state);
        }

        drawHamburger(touchState);

        if (screen == SCREEN_DELETE) {
            drawDeleteModal(&getSelectedItem(inv)->nameText);
        }
        if (screen == SCREEN_DELETE_FOLDER) {
            drawDeleteModal(&getSelectedFolder(view)->text);
        }
    }
#endif
}

void updateColors(void) {
    static u32 prevBgColor = 0xFFFFFF;
    if (bgColor != prevBgColor) {
        accent = calculateAccentColor(bgColor, black, 0x30);
        darkAccent = calculateAccentColor(bgColor, black, 0x50);
        lightAccent = calculateAccentColor(bgColor, lightGray, 0x30);
        prevBgColor = bgColor;
    }
}

void
render(C3D_RenderTarget *top, C3D_RenderTarget *bottom, const Inventory *inv, Screen screen, DisplayMode display,
       bool optionScreen, Scroll listScroll,
       Scroll gridScroll, const TouchState *touchState, ButtonPresses *presses, Scroll filterScroll,
       const FolderView *folderView) {
    bgColor = folderView->currentFolder->color;
    updateColors();

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

    drawTopScreen(top, inv, screen, display, listScroll, gridScroll, folderView, touchState);
    drawBottomScreen(bottom, inv, screen, display, optionScreen, touchState, filterScroll, presses, folderView);

    C3D_FrameEnd(0);
}