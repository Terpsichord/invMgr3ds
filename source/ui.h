#pragma once

#include <stdbool.h>

#include "main.h"
#include "inventory.h"
#include "folder.h"

typedef struct {
    float offset;
    float max;
} Scroll;

typedef enum {
    TOUCH_NONE,
    TOUCH_INCR,
    TOUCH_DECR,
    TOUCH_QTY,
    TOUCH_RENAME,
    TOUCH_TAGS,
    TOUCH_DESC,
    TOUCH_DELETE,
    TOUCH_SORT,
    TOUCH_FILTER,
    TOUCH_FILTER_BOX,
    TOUCH_SEARCH,
    TOUCH_SEARCH_CLEAR,
    TOUCH_NEW_FOLDER,
    TOUCH_RENAME_FOLDER,
    TOUCH_DELETE_FOLDER,
} TouchItem;

typedef enum {
    STAGE_NONE,
    STAGE_DOWN,
    STAGE_HELD,
    STAGE_UP,
} TouchStage;

typedef struct {
    TouchItem item;
    TouchStage stage;

    // only valid if item is TOUCH_SORT or TOUCH_FILTER
    int itemIdx;
} TouchState;

typedef enum {
    SCREEN_VIEW,
    SCREEN_EDIT,
    SCREEN_FILTER,
    SCREEN_DELETE,
    SCREEN_FOLDER,
    SCREEN_FILTER_FOLDER,
    SCREEN_DELETE_FOLDER,
} Screen;

typedef enum {
    DISPLAY_LIST,
    DISPLAY_GRID,
} DisplayMode;

typedef struct {
    int incrFrames;
    int decrFrames;
    bool filterHeld;
    bool filters[MAX_FILTERS];
} ButtonPresses;

void updateUi(Screen *screen, DisplayMode *display, TouchState *touchState, Inventory *inv, FolderView *view,
              Scroll *listScroll, Scroll *gridScroll, ButtonPresses *presses, Scroll *filterScroll);
