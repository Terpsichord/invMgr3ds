#include <3ds.h>
#include <citro2d.h>
#include <stdlib.h>
#include <time.h>

#include "inventory.h"
#include "layout.h"
#include "main.h"

//#define CONSOLE 1
//#define CONSOLE_TOP 1
//#define BORDERS 1

u32 white, lightGray, gray, darkGray, black, darken, scrollGray;

C2D_TextBuf staticTextBuf, searchTextBuf;
C2D_Text nameText, qtyText, tagsText, descText, viewHintText, editHintText, filterHintText, confirmationText,
        deleteText, cancelText, renameText, editTagsText, editDescText, outText, sortText, filterText,
        searchText, commaText, quotesText, sortTexts[NUM_SORTS];

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

void initColors(void) {
    white = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    lightGray = C2D_Color32(0xF0, 0xF0, 0xF0, 0xFF);
    gray = C2D_Color32(0xE0, 0xE0, 0xE0, 0xFF);
    darkGray = C2D_Color32(0x80, 0x80, 0x80, 0xFF);
    black = C2D_Color32(0x00, 0x00, 0x00, 0xFF);
    darken = C2D_Color32(0x00, 0x00, 0x00, 0x60);
    scrollGray = C2D_Color32(0x40, 0x40, 0x40, 0x80);
}

void initText(void) {
    staticTextBuf = C2D_TextBufNew(1024);
    searchTextBuf = C2D_TextBufNew(MAX_SEARCH_LEN);

    C2D_TextParse(&nameText, staticTextBuf, "Name");
    C2D_TextParse(&qtyText, staticTextBuf, "Qty.");
    C2D_TextParse(&tagsText, staticTextBuf, "Tags");
    C2D_TextParse(&descText, staticTextBuf, "Description");
    C2D_TextParse(&viewHintText, staticTextBuf, "\uE07D Navigate | \uE000 Edit item | \uE003 Add item | \uE002 Filter");
    C2D_TextParse(&editHintText, staticTextBuf, "\uE07D Adjust quantity | \uE002 Delete | \uE000/\uE001 Back");
    C2D_TextParse(&filterHintText, staticTextBuf, "\uE07D Change sort order | \uE001 Back");
    C2D_TextParse(&confirmationText, staticTextBuf, "Are you sure you want to delete\nthis item?");
    C2D_TextParse(&deleteText, staticTextBuf, "\uE002 Delete");
    C2D_TextParse(&cancelText, staticTextBuf, "\uE001 Cancel");
    C2D_TextParse(&renameText, staticTextBuf, "Rename");
    C2D_TextParse(&editTagsText, staticTextBuf, "Edit tags");
    C2D_TextParse(&editDescText, staticTextBuf, "Edit desc.");
    C2D_TextParse(&outText, staticTextBuf, "Out of stock");
    C2D_TextParse(&sortText, staticTextBuf, "Sort by");
    C2D_TextParse(&filterText, staticTextBuf, "Filter by");
    C2D_TextParse(&searchText, staticTextBuf, "Enter search term...");
    C2D_TextParse(&commaText, staticTextBuf, ",");
    C2D_TextParse(&quotesText, staticTextBuf, "\"");

    C2D_TextParse(&sortTexts[SORT_NONE], staticTextBuf, "None");
    C2D_TextParse(&sortTexts[SORT_QTY_ASC], staticTextBuf, "Qty. asc.");
    C2D_TextParse(&sortTexts[SORT_QTY_DESC], staticTextBuf, "Qty. desc.");
    C2D_TextParse(&sortTexts[SORT_NAME_AZ], staticTextBuf, "Name A-Z");
    C2D_TextParse(&sortTexts[SORT_NAME_ZA], staticTextBuf, "Name Z-A");

    C2D_TextOptimize(&nameText);
    C2D_TextOptimize(&qtyText);
    C2D_TextOptimize(&tagsText);
    C2D_TextOptimize(&descText);
    C2D_TextOptimize(&viewHintText);
    C2D_TextOptimize(&editHintText);
    C2D_TextOptimize(&filterHintText);
    C2D_TextOptimize(&confirmationText);
    C2D_TextOptimize(&deleteText);
    C2D_TextOptimize(&cancelText);
    C2D_TextOptimize(&renameText);
    C2D_TextOptimize(&editTagsText);
    C2D_TextOptimize(&editDescText);
    C2D_TextOptimize(&outText);
    C2D_TextOptimize(&sortText);
    C2D_TextOptimize(&filterText);
    C2D_TextOptimize(&searchText);
    C2D_TextOptimize(&commaText);
    C2D_TextOptimize(&quotesText);
    for (int i = 0; i < NUM_SORTS; i++) {
        C2D_TextOptimize(&sortTexts[i]);
    }
}

void finish(void) {
    C2D_TextBufDelete(staticTextBuf);

    C2D_Fini();
    C3D_Fini();
    gfxExit();
}

void drawHeaders(void) {
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, INV_TOP_PAD, white);

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

void drawHintText(Screen screen) {
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
    }

    C2D_DrawText(text, 0, TEXT_HPAD, TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    C2D_DrawRectSolid(0, 2 * TEXT_VPAD + 15.0f, 0.0f, BOTTOM_WIDTH, 1.0f, black);
}

void drawDeleteModal(C2D_Text *name) {
    C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, BOTTOM_WIDTH, SCREEN_HEIGHT, darken);
    C2D_DrawRectSolid(30.0f, 30.0f, 0.0f, BOTTOM_WIDTH - 60.0f, SCREEN_HEIGHT - 60.0f, white);

    C2D_DrawText(&confirmationText, C2D_AlignCenter, BOTTOM_WIDTH / 2, 60.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&deleteText, 0, 70.0f, 185.0f, 0.0f, 0.5f, 0.5f);
    C2D_DrawText(&cancelText, 0, 190.0f, 185.0f, 0.0f, 0.5f, 0.5f);

    C2D_DrawText(name, C2D_AlignCenter, BOTTOM_WIDTH / 2, 120.0f, 0.0f, 0.9f, 0.9f);
}

void drawRowButtons(TouchState *touchState) {
    C2D_DrawRectSolid(RENAME_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_RENAME ? darkGray : gray);
    C2D_DrawRectSolid(EDIT_TAGS_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_TAGS ? darkGray : gray);
    C2D_DrawRectSolid(EDIT_DESC_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_DESC ? darkGray : gray);
    C2D_DrawRectSolid(DELETE_X, ROW_BTN_Y, 0.0f, ROW_BTN_WIDTH, ROW_BTN_HEIGHT,
                      touchState->item == TOUCH_DELETE ? darkGray : gray);

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

void drawScrollBar(float scroll, float maxScroll, gfxScreen_t screen, bool filterBar) {
    float width, maxHeight, heightPortion, x, y;
    if (screen == GFX_TOP) {
        width = TOP_WIDTH;
        maxHeight = SCREEN_HEIGHT - INV_TOP_PAD - 2 * TEXT_VPAD - filterBar * BAR_HEIGHT;
        heightPortion = SCREEN_HEIGHT - filterBar * BAR_HEIGHT;
        x = 0.0f;
        y = INV_TOP_PAD;
    } else {
        width = BOX_AREA_WIDTH - BORDER;
        maxHeight = BOX_AREA_HEIGHT - 0.6 * 30.0f - 2 * TEXT_VPAD;
        heightPortion = BOX_AREA_HEIGHT;
        x = BOX_AREA_X;
        y = VIEW_TOP_PAD + 0.6 * 30.0f + 2 * TEXT_VPAD;
    }

    float height = heightPortion / (maxScroll + heightPortion) * maxHeight;
    float offset = y + scroll / maxScroll * (maxHeight - height);

    C2D_DrawRectSolid(x + width - 6.0f, offset, 0.0f, 2.0f, height, scrollGray);
}

void
render(C3D_RenderTarget *top, C3D_RenderTarget *bottom, Inventory *inv, Screen screen, float scroll, float maxScroll,
       TouchState *touchState, bool incrPressed, bool decrPressed, bool pressedFilters[], float filterScroll,
       float filterMaxScroll) {
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);

#ifndef CONSOLE_TOP
    C2D_TargetClear(top, white);
#endif

#ifndef CONSOLE
    C2D_TargetClear(bottom, white);
#endif

#ifndef CONSOLE_TOP
    C2D_SceneBegin(top);
    {
        drawItemList(inv, scroll);
        drawHeaders();

        if (maxScroll > 0.0f) {
            drawScrollBar(scroll, maxScroll, GFX_TOP, isFiltered(inv));
        }

        drawFiltering(inv);

        if (screen == SCREEN_DELETE) {
            C2D_DrawRectSolid(0.0f, 0.0f, 0.0f, TOP_WIDTH, SCREEN_HEIGHT, darken);
        }
    }
#endif

#ifndef CONSOLE
    C2D_SceneBegin(bottom);
    {
        TouchState state = *touchState;
        if (incrPressed) state.item = TOUCH_INCR;
        if (decrPressed) state.item = TOUCH_DECR;

        if (screen == SCREEN_FILTER) {
            drawSortView(inv);
            drawFilterView(inv, pressedFilters, filterScroll);
            if (filterMaxScroll > 0.0f) {
                drawScrollBar(filterScroll, filterMaxScroll, GFX_BOTTOM, false);
            }
            drawSearchBar(inv);
        } else if (numShownItems(inv) > 0) {
            drawItemView(inv, screen == SCREEN_EDIT || screen == SCREEN_DELETE, &state);
        }

        drawHintText(screen);

        if (screen == SCREEN_EDIT || screen == SCREEN_DELETE) {
            drawRowButtons(&state);
        }

        if (screen == SCREEN_DELETE) {
            drawDeleteModal(&getSelectedItem(inv)->nameText);
        }
    }
#endif

    C3D_FrameEnd(0);
}

void updateTouchState(TouchState *state, Screen screen, float *filterScroll, bool hasQuery) {
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();
    u32 KUp = hidKeysUp();

    if (kDown & KEY_TOUCH) {
        state->stage = STAGE_DOWN;
    } else if (kHeld & KEY_TOUCH) {
        state->stage = STAGE_HELD;
    } else if (KUp & KEY_TOUCH) {
        state->stage = STAGE_UP;
    } else {
        state->stage = STAGE_NONE;
        state->item = TOUCH_NONE;
    }

    if (state->stage != STAGE_NONE && state->stage != STAGE_UP) {
        state->item = TOUCH_NONE;

        touchPosition touchPos;
        hidTouchRead(&touchPos);

        if (screen == SCREEN_VIEW || screen == SCREEN_EDIT) {
            if (touchPos.px >= QTY_X && touchPos.px < QTY_X + QTY_WIDTH) {
                if (touchPos.py >= INCR_Y && touchPos.py < INCR_Y + QTY_BTN_HEIGHT) {
                    state->item = TOUCH_INCR;
                } else if (touchPos.py >= QTY_Y && touchPos.py < QTY_Y + QTY_HEIGHT) {
                    state->item = TOUCH_QTY;
                } else if (touchPos.py >= DECR_Y && touchPos.py < DECR_Y + QTY_BTN_HEIGHT) {
                    state->item = TOUCH_DECR;
                }
            }
        }
        if (screen == SCREEN_EDIT) {
            if (touchPos.py >= ROW_BTN_Y && touchPos.py < ROW_BTN_Y + ROW_BTN_HEIGHT) {
                if (touchPos.px >= RENAME_X && touchPos.px < RENAME_X + ROW_BTN_WIDTH) {
                    state->item = TOUCH_RENAME;
                } else if (touchPos.px >= EDIT_TAGS_X && touchPos.px < EDIT_TAGS_X + ROW_BTN_WIDTH) {
                    state->item = TOUCH_TAGS;
                } else if (touchPos.px >= EDIT_DESC_X && touchPos.px < EDIT_DESC_X + ROW_BTN_WIDTH) {
                    state->item = TOUCH_DESC;
                } else if (touchPos.px >= DELETE_X && touchPos.px < DELETE_X + ROW_BTN_WIDTH) {
                    state->item = TOUCH_DELETE;
                }
            }
        }
        if (screen == SCREEN_FILTER) {
            bool touchFilter = false;
            if (touchPos.px >= BOX_X && touchPos.px < BOX_X + BOX_SIZE) {
                if (touchPos.py >= BOX_Y && touchPos.py < BOX_Y + BOX_AREA_HEIGHT) {
                    if (fmodf(touchPos.py - BOX_Y + *filterScroll, BOX_SPACING) <= BOX_SIZE) {
                        touchFilter = true;
                        state->item = TOUCH_FILTER;
                        state->itemIdx = (touchPos.py - BOX_Y + *filterScroll) / BOX_SPACING;
                    }
                }
            }

            if (touchPos.px >= BOX_AREA_X && touchPos.px < BOX_AREA_X + BOX_AREA_WIDTH) {
                if (touchPos.py >= VIEW_TOP_PAD + VIEW_VPAD &&
                    touchPos.py < VIEW_TOP_PAD + VIEW_VPAD + BOX_AREA_HEIGHT) {
                    if (!touchFilter) {
                        state->item = TOUCH_FILTER_BOX;
                    }
                }
            }

            if (touchPos.px >= SORT_X && touchPos.px < SORT_X + SORT_WIDTH) {
                if (touchPos.py >= SORT_Y && touchPos.py < SORT_Y + SORT_SPACING * NUM_SORTS) {
                    state->item = TOUCH_SORT;
                    state->itemIdx = (touchPos.py - SORT_Y) / SORT_SPACING;
                }
            }

            if (touchPos.py >= SEARCH_Y && touchPos.py < SEARCH_Y + SEARCH_HEIGHT && touchPos.px >= SEARCH_X) {
                if (touchPos.px < SEARCH_X + SEARCH_WIDTH - SEARCH_HEIGHT - BORDER) {
                    state->item = TOUCH_SEARCH;
                } else if (touchPos.px < SEARCH_X + SEARCH_WIDTH - BORDER) {
                    state->item = hasQuery ? TOUCH_SEARCH_CLEAR : TOUCH_SEARCH;
                }
            }
        }
    }
}

void normalizeSpaces(char *str) {
    int read = 0, write = 0;
    int inSpace = 1;

    str[write++] = ' ';

    while (str[read] && str[read] == ' ') {
        read++;
    }

    while (str[read]) {
        if (str[read] == ' ') {
            if (!inSpace) {
                str[write++] = ' ';
                inSpace = 1;
            }
        } else {
            str[write++] = str[read];
            inSpace = 0;
        }
        read++;
    }

    if (write > 1 && str[write - 1] == ' ')
        write--;

    str[write] = '\0';
}

void updateUiTouch(TouchState *state, Screen *screen, Inventory *inv, bool incrPressed, bool decrPressed,
                   bool pressedFilters[], bool *filterHeld, float *filterScroll) {
    static touchPosition prevTouch;

    TouchItem item = state->item;
    if (state->stage == STAGE_DOWN) {
        if (item == TOUCH_FILTER || item == TOUCH_FILTER_BOX) {
            *filterHeld = true;
            hidTouchRead(&prevTouch);
        }

        if (item == TOUCH_FILTER && state->itemIdx <= inv->numTags) {
            pressedFilters[state->itemIdx] = !pressedFilters[state->itemIdx];
            updateFilteredIndices(inv, pressedFilters);
        } else if (item == TOUCH_SORT) {
            if (inv->sortOrder != state->itemIdx) {
                inv->sortOrder = state->itemIdx;
                updateSortOrder(inv, true);
            }
        } else if (item == TOUCH_SEARCH) {
            static char searchBuf[MAX_QUERY];

            SwkbdState swkbd;
            swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_QUERY - 1);
            swkbdSetHintText(&swkbd, "Enter search term...");

            SwkbdButton button = swkbdInputText(&swkbd, searchBuf, MAX_QUERY);
            if (button == SWKBD_BUTTON_CONFIRM) {
                inventorySearch(inv, searchBuf);
            }
        } else if (item == TOUCH_SEARCH_CLEAR) {
            inventorySearch(inv, "");
        }
    } else if (state->stage == STAGE_HELD) {
        if (*filterHeld) {
            touchPosition curTouch;
            hidTouchRead(&curTouch);

            *filterScroll -= curTouch.py - prevTouch.py;
            prevTouch = curTouch;
        }
    }

    if (state->stage != STAGE_UP) return;
    *filterHeld = false;

    if (item == TOUCH_INCR && !incrPressed) {
        inventoryChangeQuantity(inv, +1);
    } else if (item == TOUCH_QTY) {
        char newQtyBuf[4];

        SwkbdState qtyKb;
        swkbdInit(&qtyKb, SWKBD_TYPE_NUMPAD, 2, 4);
        swkbdSetValidation(&qtyKb, SWKBD_NOTBLANK, SWKBD_FILTER_DIGITS, 3);
        snprintf(newQtyBuf, 4, "%d", getSelectedItem(inv)->quantity);
        swkbdSetInitialText(&qtyKb, newQtyBuf);

        SwkbdButton button = swkbdInputText(&qtyKb, newQtyBuf, 4);
        if (button == SWKBD_BUTTON_CONFIRM) {
            inventorySetQuantity(inv, atoi(newQtyBuf));
        }
    } else if (item == TOUCH_DECR && !decrPressed) {
        inventoryChangeQuantity(inv, -1);
    } else if (item == TOUCH_RENAME) {
        static char nameBuf[MAX_ITEM_LEN];

        SwkbdState swkbd;
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_ITEM_LEN - 1);
        swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
        swkbdSetHintText(&swkbd, "Enter item name");
        swkbdSetInitialText(&swkbd, getSelectedItem(inv)->name);

        SwkbdButton button = swkbdInputText(&swkbd, nameBuf, MAX_ITEM_LEN);
        if (button == SWKBD_BUTTON_CONFIRM) {
            inventoryRename(inv, nameBuf);
        }
    } else if (item == TOUCH_TAGS) {
        static char tagBuf[MAX_TAG_LEN * MAX_ITEM_TAGS + 1] = " ";
        SwkbdState swkbd;
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_TAG_LEN * MAX_ITEM_TAGS - 1);
        swkbdSetHintText(&swkbd, "Enter tags (separated by spaces, e.g. \"#tag1 #tag2\")");
        swkbdSetFilterCallback(&swkbd, validateTagInput, NULL);

        static char currTagBuf[MAX_TAG_LEN * MAX_ITEM_TAGS];
        currTagBuf[0] = '\0';
        if (getSelectedItem(inv)->numTags > 0) {
            strcpy(currTagBuf, "#");
            strncat(currTagBuf, getSelectedItem(inv)->tags[0], MAX_TAG_LEN);

            for (int i = 1; i < getSelectedItem(inv)->numTags; i++) {
                strcat(currTagBuf, " #");
                strncat(currTagBuf, getSelectedItem(inv)->tags[i], MAX_TAG_LEN);
            }

            swkbdSetInitialText(&swkbd, currTagBuf);
        }

        SwkbdButton button = swkbdInputText(&swkbd, tagBuf + 1, MAX_TAG_LEN * MAX_ITEM_TAGS);

        if (button == SWKBD_BUTTON_CONFIRM) {
            normalizeSpaces(tagBuf);
            strncat(getSelectedItem(inv)->desc, tagBuf, MAX_ITEM_LEN - 1);
            refreshItemTags(inv, getSelectedItem(inv));
        }
    } else if (item == TOUCH_DESC) {
        static char descBuf[MAX_ITEM_LEN];

        SwkbdState swkbd;
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_ITEM_LEN - 1);
        swkbdSetHintText(&swkbd, "Enter item description (enter tags as \"#tag1 #tag2\")");
        swkbdSetInitialText(&swkbd, getSelectedItem(inv)->desc);

        SwkbdButton button = swkbdInputText(&swkbd, descBuf, MAX_ITEM_LEN);
        if (button == SWKBD_BUTTON_CONFIRM) {
            static char currTagBuf[MAX_TAG_LEN * MAX_ITEM_TAGS];
            currTagBuf[0] = '\0';
            if (getSelectedItem(inv)->numTags > 0) {
                strcpy(currTagBuf, "#");
                strncat(currTagBuf, getSelectedItem(inv)->tags[0], MAX_TAG_LEN);

                for (int i = 1; i < getSelectedItem(inv)->numTags; i++) {
                    strcat(currTagBuf, " #");
                    strncat(currTagBuf, getSelectedItem(inv)->tags[i], MAX_TAG_LEN);
                }
            }

            static char buf[MAX_ITEM_LEN + MAX_TAG_LEN * MAX_ITEM_TAGS];
            strncpy(buf, descBuf, MAX_ITEM_LEN - 1);
            strcat(buf, " ");
            strncat(buf, currTagBuf, (MAX_TAG_LEN * MAX_ITEM_TAGS) - 1);

            inventorySetDesc(inv, buf);
        }
    } else if (item == TOUCH_DELETE) {
        *screen = SCREEN_DELETE;
    }
}

void addItem(Inventory *inv) {
    static char nameBuf[MAX_ITEM_LEN];
    static char descBuf[MAX_ITEM_LEN];
    descBuf[0] = '\0';

    SwkbdState nameKb;
    swkbdInit(&nameKb, SWKBD_TYPE_NORMAL, 3, MAX_ITEM_LEN - 1);
    swkbdSetValidation(&nameKb, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    swkbdSetButton(&nameKb, SWKBD_BUTTON_MIDDLE, "Add desc.", true);
    swkbdSetHintText(&nameKb, "Enter item name");

    SwkbdButton button = swkbdInputText(&nameKb, nameBuf, MAX_ITEM_LEN);
    if (button == SWKBD_BUTTON_MIDDLE) {
        SwkbdState descKb;
        swkbdInit(&descKb, SWKBD_TYPE_NORMAL, 2, MAX_ITEM_LEN - 1);
        swkbdSetHintText(&descKb, "Enter item description (enter tags as \"#tag1 #tag2\")");
        swkbdInputText(&descKb, descBuf, MAX_ITEM_LEN);
    }
    if (button == SWKBD_BUTTON_MIDDLE || button == SWKBD_BUTTON_CONFIRM) {
        addInventoryItem(inv, nameBuf, descBuf, 0);
        inv->selectedIdx = numShownItems(inv) - 1;
    }

}

void addExampleData(Inventory *inv) {
    addInventoryItem(inv, "apple", "fresh and juicy #fruit #food", 25);
    addInventoryItem(inv, "banana",
                     "ripe yellow goodness. #fruit These bananas are sourced from small farms, ripened naturally, and delivered fresh every morning, making them perfect for breakfast, smoothies, or a quick snack.",
                     0);
    addInventoryItem(inv, "stainless steel water bottle",
                     "keeps drinks cold. #eco #hydration Built with double-wall insulation technology, capable of keeping liquids cold for up to 24 hours and hot for up to 12 hours. Ideal for outdoor adventures, gym sessions, or daily commutes.",
                     7);
    addInventoryItem(inv, "gaming mouse", "RGB lights and fast clicks #tech #gaming", 4);
    addInventoryItem(inv, "milk 2%", "organic farm fresh #dairy #breakfast", 0);
    addInventoryItem(inv, "wireless headphones",
                     "noise cancelling. #music #audio Experience rich, deep bass and crystal-clear treble with these premium headphones designed for long listening sessions. Bluetooth 5.0 connectivity ensures a stable connection up to 10 meters away.",
                     6);
    addInventoryItem(inv, "pack of pencils", "HB grade #stationery #school", 50);
    addInventoryItem(inv, "instant ramen",
                     "spicy chicken flavor. #food #quickmeal Each pack contains a blend of spices and seasonings, ready to serve in under 5 minutes. Perfect for students, busy professionals, or anyone craving a late-night snack.",
                     0);
    addInventoryItem(inv, "LED light bulb", "energy saving #home #lighting", 15);
    addInventoryItem(inv, "cooking oil",
                     "sunflower oil 1L. #kitchen #food This pure sunflower oil is great for frying, baking, and salad dressings, offering a light taste that won't overpower your dishes.",
                     8);
    addInventoryItem(inv, "mystery box", "contents unknown #fun #surprise", 1);
    addInventoryItem(inv, "dog food", "beef flavor kibble #pet #dog", 0);
    addInventoryItem(inv, "catnip toy", "fun for cats #pet #cat", 12);
    addInventoryItem(inv, "spaghetti pasta",
                     "500g pack. #italian #food Crafted from high-quality durum wheat semolina, this pasta cooks to a perfect al dente texture, pairing beautifully with sauces, vegetables, and meats.",
                     14);
    addInventoryItem(inv, "tomato sauce", "classic marinara #food #kitchen", 0);
    addInventoryItem(inv, "coffee beans",
                     "dark roast. #morning #caffeine Sourced from Colombian highlands, roasted in small batches for maximum flavor, and packed immediately to lock in freshness.",
                     11);
    addInventoryItem(inv, "yoga mat", "non-slip surface #fitness #health", 0);
    addInventoryItem(inv, "flashlight", "LED ultra bright #emergency #tools", 3);
    addInventoryItem(inv, "batteries AA", "pack of 24 #power #electronics", 13);
    addInventoryItem(inv, "chocolate bar",
                     "milk chocolate. #snack #sweet Smooth and creamy texture with just the right amount of sweetness, perfect for a quick treat or sharing with friends.",
                     0);
    addInventoryItem(inv, "USB-C cable", "fast charging #tech #mobile", 17);
    addInventoryItem(inv, "shampoo", "anti-dandruff formula #bath #haircare", 9);
    addInventoryItem(inv, "toothpaste",
                     "mint flavor. #oralcare Freshen your breath while fighting cavities with this fluoride-rich, dentist-approved formula.",
                     16);
    addInventoryItem(inv, "laundry detergent", "liquid 2L #cleaning #home", 0);
    addInventoryItem(inv, "socks", "cotton black pair #clothing", 40);
    addInventoryItem(inv, "t-shirt", "graphic print #clothing #fashion", 12);
    addInventoryItem(inv, "backpack", "waterproof #bag #travel", 4);
    addInventoryItem(inv, "paper towels", "2-roll pack #home #cleaning", 15);
    addInventoryItem(inv, "hand sanitizer", "70% alcohol #health #hygiene", 19);
    addInventoryItem(inv, "candle",
                     "lavender scent. #home #relax Provides a calming aroma for up to 40 hours of burn time, ideal for bedrooms, bathrooms, or meditation spaces.",
                     8);
    addInventoryItem(inv, "board game",
                     "family fun. #games #entertainment Includes colorful game pieces, cards, and a foldable board — designed to bring friends and family together for hours of laughter.",
                     0);
    addInventoryItem(inv, "laptop stand", "ergonomic design #office #tech", 2);
    addInventoryItem(inv, "notebook", "lined pages #stationery #writing", 28);
    addInventoryItem(inv, "glass jar", "500ml with lid #kitchen", 14);
    addInventoryItem(inv, "umbrella", "windproof #travel #weather", 6);
    addInventoryItem(inv, "rice", "5kg bag #food #staple", 7);
    addInventoryItem(inv, "olive oil",
                     "extra virgin. #kitchen #food Cold-pressed to preserve flavor and nutrients, perfect for salad dressings, drizzling, or light cooking.",
                     0);
    addInventoryItem(inv, "whisk", "stainless steel #kitchen #baking", 5);
    addInventoryItem(inv, "cutting board", "bamboo #kitchen", 4);
    addInventoryItem(inv, "mug", "ceramic #drinkware #kitchen", 10);
    addInventoryItem(inv, "throw blanket",
                     "cozy fleece. #home #decor Ultra-soft and machine washable, ideal for chilly evenings on the couch or adding a decorative touch to your bed.",
                     3);
    addInventoryItem(inv, "garden hose", "15m length #outdoor #tools", 0);
    addInventoryItem(inv, "paint brush", "medium size #art #DIY", 20);
    addInventoryItem(inv, "screwdriver set", "8 pieces #tools", 4);
    addInventoryItem(inv, "earplugs", "noise reduction #sleep #travel", 12);
    addInventoryItem(inv, "first aid kit",
                     "emergency essentials. #health #safety Includes bandages, antiseptic wipes, scissors, and more to handle minor injuries at home or on the go.",
                     3);
    addInventoryItem(inv, "beanie", "warm wool #winter #clothing", 8);
    addInventoryItem(inv, "skipping rope", "adjustable length #fitness", 5);
}

// TODO: also remember to deduplicate item tags when adding items or editing tags/description

int main() {
    C3D_RenderTarget *top, *bottom;
    initGfx(&top, &bottom);
    initColors();
    initText();
    atexit(finish);

    Inventory inv;
    initInventory(&inv);
    loadInventory(&inv);

    Screen screen = SCREEN_VIEW;

    TouchState touchState;
    int incrPressed = 0, decrPressed = 0;
    float scroll = 0.0f, maxScroll = 0.0f, filterScroll = 0.0f, filterMaxScroll = 0.0f;

    bool pressedFilters[MAX_FILTERS] = {false}, filterHeld = false;

    while (aptMainLoop()) {
        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();

        if (kDown & KEY_START) break;

        updateTouchState(&touchState, screen, &filterScroll, hasQuery(&inv));
        updateUiTouch(&touchState, &screen, &inv, incrPressed, decrPressed, pressedFilters, &filterHeld, &filterScroll);

        if (decrPressed > 0) {
            decrPressed++;
            if (decrPressed > BTN_FRAMES) {
                decrPressed = 0;
            }
        }
        if (incrPressed > 0) {
            incrPressed++;
            if (incrPressed > BTN_FRAMES) {
                incrPressed = 0;
            }
        }

        if (screen == SCREEN_VIEW) {
            if (numShownItems(&inv) > 0) {
                if (kDown & KEY_DDOWN) {
                    inv.selectedIdx = (inv.selectedIdx + 1) % numShownItems(&inv);
                }
                if (kDown & KEY_DUP) {
                    inv.selectedIdx = (inv.selectedIdx + numShownItems(&inv) - 1) % numShownItems(&inv);
                }

                if (kDown & (KEY_DUP | KEY_DDOWN)) {
                    scroll = MIN(scroll, ITEM_HEIGHT * inv.selectedIdx);
                    scroll = MAX(scroll, ITEM_HEIGHT * (inv.selectedIdx + 1) - SCREEN_HEIGHT + INV_TOP_PAD + isFiltered(&inv) * BAR_HEIGHT);
                }

                if (kDown & KEY_A) {
                    screen = SCREEN_EDIT;
                }
            }
            if (kDown & KEY_X) {
                screen = SCREEN_FILTER;
            }
            if (kDown & KEY_Y) {
                addItem(&inv);
            }

        } else if (screen == SCREEN_EDIT) {
            if (kDown & KEY_DDOWN) {
                inventoryChangeQuantity(&inv, -1);
                decrPressed++;
            }
            if (kDown & KEY_DUP) {
                inventoryChangeQuantity(&inv, +1);
                incrPressed++;
            }
            if (kDown & (KEY_A | KEY_B)) {
                screen = SCREEN_VIEW;
            }
            if (kDown & KEY_X) {
                screen = SCREEN_DELETE;
            }
        } else if (screen == SCREEN_FILTER) {
            if (kDown & KEY_B) {
                screen = SCREEN_VIEW;
            }
            if (kDown & KEY_DDOWN) {
                inv.sortOrder = (inv.sortOrder + 1) % NUM_SORTS;
                updateSortOrder(&inv, true);
            }
            if (kDown & KEY_DUP) {
                inv.sortOrder = (inv.sortOrder + NUM_SORTS - 1) % NUM_SORTS;
                updateSortOrder(&inv, true);
            }
        } else if (screen == SCREEN_DELETE) {
            if (kDown & KEY_B) {
                screen = SCREEN_EDIT;
            }
            if (kDown & KEY_X) {
                removeInventoryItem(&inv, inv.selectedIdx);
                screen = SCREEN_VIEW;
            }
        }

        if (screen != SCREEN_DELETE && kHeld & (KEY_CPAD_UP | KEY_CPAD_DOWN)) {
            circlePosition circlePos;
            hidCircleRead(&circlePos);

            scroll -= circlePos.dy / 20.0f;
        }

        maxScroll = numShownItems(&inv) * ITEM_HEIGHT - SCREEN_HEIGHT + INV_TOP_PAD + isFiltered(&inv) * BAR_HEIGHT;
        scroll = MIN(scroll, maxScroll);
        scroll = MAX(0.0f, scroll);

        filterMaxScroll = (inv.numTags + 2) * BOX_SPACING - BOX_AREA_HEIGHT;
        filterScroll = MIN(filterScroll, filterMaxScroll);
        filterScroll = MAX(0.0f, filterScroll);

        render(top, bottom, &inv, screen, scroll, maxScroll, &touchState, incrPressed, decrPressed, pressedFilters,
               filterScroll, filterMaxScroll);
    }

    saveInventory(&inv);
    freeInventory(&inv);

    return 0;
}
