#include <stdlib.h>

#include "ui.h"
#include "layout.h"
#include "render.h"

void updateTouchState(TouchState *state, Screen screen, bool optionScreen, float *filterScroll, bool hasQuery, bool folderEmpty) {
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

        if (touchPos.py <= BURGER_SIZE && touchPos.px >= BOTTOM_WIDTH - BURGER_SIZE) {
            state->item = TOUCH_BURGER;
        }

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

        if (optionScreen) {
            if (touchPos.px >= OPTIONS_HPAD && touchPos.px < OPTIONS_HPAD + OPTION_BOX_SIZE) {
                if (touchPos.py >= OPTIONS_VPAD && touchPos.py < OPTIONS_VPAD + OPTION_BOX_SIZE) {
                    state->item = TOUCH_GRID_OPTION;
                }
            }
            return;
        }
        if (screen == SCREEN_VIEW) {
            if (touchPos.px >= NEW_FOLDER_X && touchPos.px < DELETE_FOLDER_X + FOLDER_BTN_WIDTH
                && touchPos.py >= ROW_BTN_Y && touchPos.py < ROW_BTN_Y + ROW_BTN_HEIGHT) {
                state->prevScreen = screen;
                state->item = TOUCH_COLOR_FOLDER;
            }
        } else if (screen == SCREEN_EDIT) {
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
        } else if (screen == SCREEN_FILTER || screen == SCREEN_FILTER_FOLDER) {
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
        } else if (screen == SCREEN_FOLDER) {
            if (folderEmpty) {
                if (touchPos.py >= ROW_BTN_Y && touchPos.py < ROW_BTN_Y + ROW_BTN_HEIGHT) {
                    if (touchPos.px >= NEW_EMPTY_X && touchPos.px < NEW_EMPTY_X + EMPTY_BTN_WIDTH) {
                        state->item = TOUCH_NEW_FOLDER;
                    } else if (touchPos.px >= COLOR_EMPTY_X && touchPos.px < COLOR_EMPTY_X + EMPTY_BTN_WIDTH) {
                        state->prevScreen = screen;
                        state->item = TOUCH_COLOR_FOLDER;
                    }
                }
            } else {
                if (touchPos.py >= ROW_BTN_Y && touchPos.py < ROW_BTN_Y + ROW_BTN_HEIGHT) {
                    if (touchPos.px >= NEW_FOLDER_X && touchPos.px < NEW_FOLDER_X + FOLDER_BTN_WIDTH) {
                        state->item = TOUCH_NEW_FOLDER;
                    } else if (touchPos.px >= RENAME_FOLDER_X && touchPos.px < RENAME_FOLDER_X + FOLDER_BTN_WIDTH) {
                        state->item = TOUCH_RENAME_FOLDER;
                    } else if (touchPos.px >= COLOR_FOLDER_X && touchPos.px < COLOR_FOLDER_X + FOLDER_BTN_WIDTH) {
                        state->prevScreen = screen;
                        state->item = TOUCH_COLOR_FOLDER;
                    } else if (touchPos.px >= DELETE_FOLDER_X && touchPos.px < DELETE_FOLDER_X + FOLDER_BTN_WIDTH) {
                        state->item = TOUCH_DELETE_FOLDER;
                    }
                }
            }
        } else if (screen == SCREEN_COLOR) {
            if (state->color.held) {
                state->color.saturation = (touchPos.px - COLOR_X) / COLOR_SIZE;
                state->color.saturation = MIN(1.0f, MAX(0.0f, state->color.saturation));
                state->color.value = 1.0f - (touchPos.py - COLOR_Y) / COLOR_SIZE;
                state->color.value = MIN(1.0f, MAX(0.0f, state->color.value));
            } else if (touchPos.py >= COLOR_Y && touchPos.py < COLOR_Y + COLOR_SIZE) {
                if (touchPos.px >= COLOR_X && touchPos.px < COLOR_X + COLOR_SIZE) {
                    state->color.saturation = (touchPos.px - COLOR_X) / COLOR_SIZE;
                    state->color.value = 1.0f - (touchPos.py - COLOR_Y) / COLOR_SIZE;
                    state->item = TOUCH_COLOR;
                } else if (touchPos.px >= COLOR_X + COLOR_SIZE + COLOR_PAD && touchPos.px < COLOR_X + COLOR_SIZE + COLOR_PAD + COLOR_BAR_WIDTH) {
                    state->color.hue = (touchPos.py - COLOR_Y) * 360.0f / COLOR_SIZE;
                }
            }
        }
    }
}

static void normalizeSpaces(char *str) {
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

void updateUiTouch(TouchState *state, Screen *screen, DisplayMode *display, bool *optionScreen, Inventory *inv, FolderView *view, ButtonPresses *presses,
                   float *filterScroll) {
    static touchPosition prevTouch;

    TouchItem item = state->item;
    if (state->stage == STAGE_DOWN) {
        if (item == TOUCH_FILTER || item == TOUCH_FILTER_BOX) {
            presses->filterHeld = true;
            hidTouchRead(&prevTouch);
        }
        if (item == TOUCH_COLOR) {
            state->color.held = true;
        }

        if (item == TOUCH_FILTER && state->itemIdx < inventoryAvailableFilters(inv)) {
            presses->filters[state->itemIdx] = !presses->filters[state->itemIdx];
            updateFilteredIndices(inv, presses->filters);
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
        } else if (item == TOUCH_GRID_OPTION) {
            *display = *display == DISPLAY_LIST ? DISPLAY_GRID : DISPLAY_LIST;
        }
    } else if (state->stage == STAGE_HELD) {
        if (presses->filterHeld) {
            touchPosition curTouch;
            hidTouchRead(&curTouch);

            *filterScroll -= curTouch.py - prevTouch.py;
            prevTouch = curTouch;
        }
        if (item == TOUCH_COLOR) {
            state->color.held = true;
        }
    }

    if (presses->decrFrames > 0) {
        (presses->decrFrames)++;
        if (presses->decrFrames > BTN_FRAMES) {
            presses->decrFrames = 0;
        }
    }
    if (presses->incrFrames > 0) {
        presses->incrFrames++;
        if (presses->incrFrames > BTN_FRAMES) {
            presses->incrFrames = 0;
        }
    }

    if (*screen == SCREEN_COLOR) {
        view->currentFolder->color = hsvToRgb(state->color.hue, state->color.saturation, state->color.value);
    } else {
        state->color.held = false;
    }

    if (state->stage != STAGE_UP) return;
    presses->filterHeld = false;
    if (*screen == SCREEN_COLOR) state->color.held = false;

    if (item == TOUCH_INCR && presses->incrFrames == 0) {
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
    } else if (item == TOUCH_DECR && presses->decrFrames == 0) {
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
        swkbdSetFeatures(&swkbd, SWKBD_MULTILINE);
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
    } else if (item == TOUCH_BURGER) {
        *optionScreen = true;
    } else if ((item == TOUCH_NEW_FOLDER && view->currentFolder->numChildren < MAX_FOLDERS) ||
               item == TOUCH_RENAME_FOLDER) {
        static char nameBuf[MAX_FOLDER_LEN];
        SwkbdState swkbd;
        swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 2, MAX_FOLDER_LEN - 1);
        swkbdSetValidation(&swkbd, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
        swkbdSetHintText(&swkbd, "Enter folder name");
        if (item == TOUCH_RENAME_FOLDER) {
            swkbdSetInitialText(&swkbd, getSelectedFolder(view)->name);
        }

        errorConf err;
        errorInit(&err, ERROR_TEXT, CFG_LANGUAGE_EN);
        errorCode(&err, 0);
        errorText(&err, "Folder with name already exists");

        SwkbdButton button;
        Folder *existing;
        do {
            existing = NULL;
            button = swkbdInputText(&swkbd, nameBuf, MAX_FOLDER_LEN);
            if (button == SWKBD_BUTTON_CONFIRM) {
                if ((existing = findFolder(view->rootFolder, nameBuf)) != NULL && existing != getSelectedFolder(view)) {
                    errorDisp(&err);
                    swkbdSetInitialText(&swkbd, nameBuf);
                }
            }
        } while (existing != NULL);

        if (button == SWKBD_BUTTON_CONFIRM) {
            if (item == TOUCH_NEW_FOLDER) {
                newFolder(nameBuf, view->currentFolder, NULL, view->textBuf);
            } else {
                strncpy(getSelectedFolder(view)->name, nameBuf, MAX_FOLDER_LEN - 1);
                updateFolderViewText(view);
            }
            sortFolders(view->currentFolder);
            if (item == TOUCH_NEW_FOLDER) {
                for (int i = 0; i < view->currentFolder->numChildren; i++) {
                    if (strcmp(view->currentFolder->children[i]->name, nameBuf) == 0) {
                        view->selectedIdx = i;
                    }
                }
            }
        }
    } else if (item == TOUCH_DELETE) {
        *screen = SCREEN_DELETE;
    } else if (item == TOUCH_DELETE_FOLDER) {
        *screen = SCREEN_DELETE_FOLDER;
    } else if (item == TOUCH_COLOR_FOLDER) {
        view->oldColor = view->currentFolder->color;
        rgbToHsv(view->oldColor, &state->color.hue, &state->color.saturation, &state->color.value);
        *screen = SCREEN_COLOR;
    }
}

static SwkbdButton addItem(Inventory *inv, Folder *folder) {
    static char nameBuf[MAX_ITEM_LEN];
    static char descBuf[MAX_ITEM_LEN];
    descBuf[0] = '\0';

    SwkbdState nameKb;
    swkbdInit(&nameKb, SWKBD_TYPE_NORMAL, 3, MAX_ITEM_LEN - 1);
    swkbdSetValidation(&nameKb, SWKBD_NOTEMPTY_NOTBLANK, 0, 0);
    swkbdSetButton(&nameKb, SWKBD_BUTTON_MIDDLE, "Add desc.", true);
    swkbdSetHintText(&nameKb, "Enter item name");

    errorConf err;
    errorInit(&err, ERROR_TEXT, CFG_LANGUAGE_EN);
    errorCode(&err, 0);
    errorText(&err, "Item with name already exists");


    SwkbdButton button;
    bool exists;
    do {
        exists = false;
        button = swkbdInputText(&nameKb, nameBuf, MAX_ITEM_LEN);
        if (button != SWKBD_BUTTON_LEFT) {
            for (int i = 0; i < inv->numItems; i++) {
                if (inv->items[i].folders[0] == folder && strcmp(inv->items[i].name, nameBuf) == 0) {
                    errorDisp(&err);
                    swkbdSetInitialText(&nameKb, nameBuf);
                    exists = true;
                }
            }
        }
    } while (exists);

    if (button == SWKBD_BUTTON_MIDDLE) {
        SwkbdState descKb;
        swkbdInit(&descKb, SWKBD_TYPE_NORMAL, 2, MAX_ITEM_LEN - 1);
        swkbdSetHintText(&descKb, "Enter item description (enter tags as \"#tag1 #tag2\")");
        swkbdSetFeatures(&descKb, SWKBD_MULTILINE);
        if (swkbdInputText(&descKb, descBuf, MAX_ITEM_LEN) == SWKBD_BUTTON_LEFT) {
            return SWKBD_BUTTON_LEFT;
        }
    }

    if (button != SWKBD_BUTTON_LEFT) {
        const Folder *folders[MAX_FOLDERS];

        int numFolders = 0;
        while (folder->parent != NULL) {
            folders[numFolders++] = folder;
            folder = folder->parent;
        }

        addInventoryItem(inv, nameBuf, descBuf, 0, folders, numFolders);
        inv->selectedIdx = numShownItems(inv) - 1;
    }

    return button;
}


bool itemsEqual(const Item *item1, const Item *item2) {
    if (item1->numFolders != item2->numFolders) return false;
    for (int i = 0; i < item1->numFolders; i++) {
        if (strcmp(item1->folders[i]->name, item2->folders[i]->name) != 0) {
            return false;
        }
    }
    return strcmp(item1->name, item2->name) == 0;
}

void editItem(FolderView *view, Inventory *inv, Screen *screen, Item *item) {
    view->currentFolder = item->folders[0];
    inventorySetFolderFilter(inv, view->currentFolder);

    for (int i = 0; i < inv->numFiltered; i++) {
        if (itemsEqual(&inv->items[i], item)) {
            inv->selectedIdx = i;
        }
    }
    *screen = SCREEN_EDIT;
}

static void updateListScroll(Scroll *listScroll, Inventory *inv, Screen screen) {
    listScroll->offset = MIN(listScroll->offset, ITEM_HEIGHT * inv->selectedIdx);
    listScroll->offset = MAX(listScroll->offset,
                             ITEM_HEIGHT * (inv->selectedIdx + 1) - SCREEN_HEIGHT + INV_TOP_PAD +
                             showFilterBar(inv, screen) * BAR_HEIGHT);
}

static void updateGridScroll(Scroll *gridScroll, Inventory *inv) {
    int row = inv->selectedIdx / GRID_COLUMNS;
    gridScroll->offset = MIN(gridScroll->offset, GRID_SPACING * row);
    gridScroll->offset = MAX(gridScroll->offset,
                             GRID_SPACING * (row + 1) - SCREEN_HEIGHT + GRID_VPAD);
}

void updateUiButtons(Screen *screen, DisplayMode display, Inventory *inv, FolderView *view, bool *optionScreen, Scroll *listScroll,
                     Scroll *gridScroll, ButtonPresses *presses, Screen prevScreen) {
    u32 kDown = hidKeysDown();
    u32 kHeld = hidKeysHeld();

    if (*optionScreen) {
        if (kDown & KEY_B) {
            *optionScreen = false;
        }
        return;
    }

    if (*screen == SCREEN_VIEW || *screen == SCREEN_FILTER_FOLDER) {
        if (numShownItems(inv) > 0) {
            if (display == DISPLAY_LIST) {
                if (kDown & KEY_DDOWN) {
                    inv->selectedIdx = (inv->selectedIdx + 1) % numShownItems(inv);
                }
                if (kDown & KEY_DUP) {
                    inv->selectedIdx = (inv->selectedIdx + numShownItems(inv) - 1) % numShownItems(inv);
                }

                if (kDown & (KEY_DUP | KEY_DDOWN)) {
                    updateListScroll(listScroll, inv, *screen);
                }
            } else if (display == DISPLAY_GRID) {
                if (kDown & KEY_DRIGHT) {
                    inv->selectedIdx = (inv->selectedIdx + 1) % numShownItems(inv);
                }
                if (kDown & KEY_DLEFT) {
                    inv->selectedIdx = (inv->selectedIdx + numShownItems(inv) - 1) % numShownItems(inv);
                }

                int numRows = (numShownItems(inv) + GRID_COLUMNS - 1) / GRID_COLUMNS;
                int row = inv->selectedIdx / GRID_COLUMNS, col = inv->selectedIdx % GRID_COLUMNS;

                if (kDown & KEY_DDOWN) {
                    if (row < numRows - 2) {
                        inv->selectedIdx += GRID_COLUMNS;
                    } else if (row == numRows - 2) {
                        inv->selectedIdx = MIN(inv->selectedIdx + GRID_COLUMNS, numShownItems(inv) - 1);
                    } else {
                        inv->selectedIdx = col;
                    }
                }
                if (kDown & KEY_DUP) {
                    if (row > 0) {
                        inv->selectedIdx -= GRID_COLUMNS;
                    } else {
                        inv->selectedIdx = MIN((numRows - 1) * GRID_COLUMNS + col, numShownItems(inv) - 1);
                    }
                }

                if (kDown & (KEY_DLEFT | KEY_DRIGHT | KEY_DUP | KEY_DDOWN)) {
                    updateGridScroll(gridScroll, inv);
                }
            }


            if (*screen == SCREEN_VIEW && kDown & KEY_A) {
                *screen = SCREEN_EDIT;
            }
        }
        if (*screen == SCREEN_VIEW) {
            if (kDown & KEY_R) {
                *screen = SCREEN_FILTER;
            }
            if (kDown & KEY_X) {
                int prevIdx = inv->selectedIdx;
                SwkbdButton button = addItem(inv, view->currentFolder);
                if (button != SWKBD_BUTTON_LEFT) {
                    inventorySetQuantity(inv, getItem(inv, prevIdx)->quantity);

                    getSelectedItem(inv)->numTags = getItem(inv, prevIdx)->numTags;
                    for (int i = 0; i < getItem(inv, prevIdx)->numTags; i++) {
                        strncpy(getSelectedItem(inv)->tags[i], getItem(inv, prevIdx)->tags[i], MAX_TAG_LEN - 1);
                    }
                    if (getSelectedItem(inv)->desc[0] == '\0') {
                        strncpy(getSelectedItem(inv)->desc, getItem(inv, prevIdx)->desc, MAX_ITEM_LEN - 1);
                    }
                    refreshItemTags(inv, getSelectedItem(inv));

                    updateListScroll(listScroll, inv, *screen);
                    updateGridScroll(gridScroll, inv);
                }
            }
            if (kDown & KEY_Y) {
                addItem(inv, view->currentFolder);
                updateListScroll(listScroll, inv, *screen);
                updateGridScroll(gridScroll, inv);
            }
            if (kDown & KEY_B) {
                folderViewNavigateParent(view);
                *screen = SCREEN_FOLDER;
            }
        } else {
            if (kDown & KEY_A && numShownItems(inv) > 0) {
                editItem(view, inv, screen, getSelectedItem(inv));
            }
            if (kDown & (KEY_R | KEY_B)) {
                *screen = SCREEN_FOLDER;
            }
        }
    } else if (*screen == SCREEN_EDIT) {
        if (kDown & KEY_DDOWN) {
            inventoryChangeQuantity(inv, -1);
            presses->decrFrames++;
        }
        if (kDown & KEY_DUP) {
            inventoryChangeQuantity(inv, +1);
            presses->incrFrames++;
        }
        if (kDown & (KEY_A | KEY_B)) {
            *screen = SCREEN_VIEW;
        }
        if (kDown & KEY_X) {
            *screen = SCREEN_DELETE;
        }
    } else if (*screen == SCREEN_FILTER) {
        if (kDown & (KEY_R | KEY_B)) {
            *screen = SCREEN_VIEW;
        }
        if (kDown & KEY_DDOWN) {
            inv->sortOrder = (inv->sortOrder + 1) % NUM_SORTS;
            updateSortOrder(inv, true);
        }
        if (kDown & KEY_DUP) {
            inv->sortOrder = (inv->sortOrder + NUM_SORTS - 1) % NUM_SORTS;
            updateSortOrder(inv, true);
        }
    } else if (*screen == SCREEN_DELETE) {
        if (kDown & KEY_B) {
            *screen = SCREEN_EDIT;
        }
        if (kDown & KEY_X) {
            removeInventoryItem(inv, getItemIdx(inv, inv->selectedIdx));
            *screen = SCREEN_VIEW;
        }
    } else if (*screen == SCREEN_DELETE_FOLDER) {
        if (kDown & KEY_B) {
            *screen = SCREEN_FOLDER;
        }
        if (kDown & KEY_X) {
            deleteSelectedFolder(view, inv);
            *screen = SCREEN_FOLDER;
        }
    } else if (*screen == SCREEN_FOLDER) {
        if (kDown & KEY_DDOWN) {
            view->selectedIdx = (view->selectedIdx + 1) % view->currentFolder->numChildren;
        }
        if (kDown & KEY_DUP) {
            view->selectedIdx =
                    (view->selectedIdx + view->currentFolder->numChildren - 1) % view->currentFolder->numChildren;
        }

        // todo: should probably add proper scrolling for folder view
//        if (kDown & (KEY_DUP | KEY_DDOWN)) {
//            listScroll->offset = MIN(listScroll->offset, FOLDER_SPACING * inv->selectedIdx);
//            listScroll->offset = MAX(listScroll->offset, FOLDER_SPACING * (inv->selectedIdx + 1) - SCREEN_HEIGHT + INV_TOP_PAD);
//        }

        if (kDown & KEY_A) {
            folderViewNavigateChild(view);
            if (folderHasItems(inv, view->currentFolder)) {
                inventorySetFolderFilter(inv, view->currentFolder);
                if (inv->selectedIdx >= numShownItems(inv)) {
                    inv->selectedIdx = numShownItems(inv) - 1;
                }
                *screen = SCREEN_VIEW;
            }
        }
        if (kDown & KEY_B) {
            folderViewNavigateParent(view);
        }

        if (!isFolderEmpty(view) && kDown & KEY_R) {
            inventorySetFolderFilter(inv, view->currentFolder);
            *screen = SCREEN_FILTER_FOLDER;
        }

        if (isFolderEmpty(view) && !isEmptyRoot(view) && kDown & KEY_Y) {
            SwkbdButton button = addItem(inv, view->currentFolder);
            if (button == SWKBD_BUTTON_CONFIRM) {
                inventorySetFolderFilter(inv, view->currentFolder);
                *screen = SCREEN_VIEW;
            }
        }
    } else if (*screen == SCREEN_COLOR) {
        if (kDown & KEY_B) {
            view->currentFolder->color = view->oldColor;
            *screen = prevScreen;
        } else if (kDown & KEY_A) {
            *screen = prevScreen;
        }
    }

    if (*screen == SCREEN_VIEW || *screen == SCREEN_EDIT || *screen == SCREEN_FILTER ||
        *screen == SCREEN_FILTER_FOLDER) {
        if (kHeld & (KEY_CPAD_UP | KEY_CPAD_DOWN)) {
            circlePosition circlePos;
            hidCircleRead(&circlePos);


            Scroll *scroll = display == DISPLAY_LIST ? listScroll : gridScroll;
            scroll->offset -= circlePos.dy / 20.0f;
        }
    }
}

void updateScroll(Scroll *listScroll, Scroll *gridScroll, Inventory *inv, Scroll *filterScroll) {
    listScroll->max = numShownItems(inv) * ITEM_HEIGHT - SCREEN_HEIGHT + INV_TOP_PAD +
                      showFilterBar(inv, SCREEN_VIEW) * BAR_HEIGHT;
    listScroll->offset = MIN(listScroll->offset, listScroll->max);
    listScroll->offset = MAX(0.0f, listScroll->offset);

    gridScroll->max = (numShownItems(inv) + GRID_COLUMNS - 1) / GRID_COLUMNS * GRID_SPACING - SCREEN_HEIGHT + GRID_VPAD;
    gridScroll->offset = MIN(gridScroll->offset, gridScroll->max);
    gridScroll->offset = MAX(0.0f, gridScroll->offset);

    filterScroll->max = (inventoryAvailableFilters(inv) + 1) * BOX_SPACING - BOX_AREA_HEIGHT;
    filterScroll->offset = MIN(filterScroll->offset, filterScroll->max);
    filterScroll->offset = MAX(0.0f, filterScroll->offset);

}

void updateUi(Screen *screen, DisplayMode *display, bool *optionScreen, TouchState *touchState, Inventory *inv, FolderView *view,
              Scroll *listScroll, Scroll *gridScroll, ButtonPresses *presses, Scroll *filterScroll) {
    updateTouchState(touchState, *screen, *optionScreen, &filterScroll->offset, hasQuery(inv), isFolderEmpty(view));
    updateUiTouch(touchState, screen, display, optionScreen, inv, view, presses, &filterScroll->offset);
    updateUiButtons(screen, *display, inv, view, optionScreen, listScroll, gridScroll, presses, touchState->prevScreen);
    updateScroll(listScroll, gridScroll, inv, filterScroll);
}