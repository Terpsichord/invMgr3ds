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
    TOUCH_BURGER,
    TOUCH_GRID_OPTION,
    TOUCH_RENAME,
    TOUCH_TAGS,
    TOUCH_DESC,
    TOUCH_DELETE,
    TOUCH_SORT,
    TOUCH_FILTER,
    TOUCH_FILTER_BOX,
    TOUCH_SEARCH,
    TOUCH_SEARCH_CLEAR,
    TOUCH_COLOR,
    TOUCH_NEW_FOLDER,
    TOUCH_RENAME_FOLDER,
    TOUCH_COLOR_FOLDER,
    TOUCH_DELETE_FOLDER,
} TouchItem;

typedef enum {
    STAGE_NONE,
    STAGE_DOWN,
    STAGE_HELD,
    STAGE_UP,
} TouchStage;

typedef struct {
    float hue;
    float saturation;
    float value;
    bool held;
} ColorState;

typedef enum {
    SCREEN_VIEW,
    SCREEN_EDIT,
    SCREEN_FILTER,
    SCREEN_DELETE,
    SCREEN_FOLDER,
    SCREEN_COLOR,
    SCREEN_FILTER_FOLDER,
    SCREEN_DELETE_FOLDER,
} Screen;

typedef struct {
    TouchItem item;
    TouchStage stage;

    // only valid if item is TOUCH_SORT or TOUCH_FILTER
    int itemIdx;
    // only valid if screen is SCREEN_COLOR
    ColorState color;
    // only valid if screen is SCREEN_COLOR
    Screen prevScreen;
} TouchState;

typedef enum {
    DISPLAY_LIST,
    DISPLAY_GRID,
} DisplayMode;

typedef struct {
    int incrFrames;
    int decrFrames;
    bool filterHeld;
    bool colorHeld;
    bool filters[MAX_FILTERS];
} ButtonPresses;

void updateUi(Screen *screen, DisplayMode *display, bool *optionScreen, TouchState *touchState, Inventory *inv, FolderView *view,
              Scroll *listScroll, Scroll *gridScroll, ButtonPresses *presses, Scroll *filterScroll);
