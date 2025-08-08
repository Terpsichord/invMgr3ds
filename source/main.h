#pragma once

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define NUM_SORTS 5

extern C2D_Text qtyText, tagsText, outText, sortText, filterText, searchText, commaText, quotesText, sortTexts[NUM_SORTS];
extern u32 white, lightGray, gray, darkGray, black, columnGray;

#define TOP_WIDTH 400.0f
#define BOTTOM_WIDTH 320.0f
#define SCREEN_HEIGHT 240.0f


#define MAX_SEARCH_LEN 64
#define BTN_FRAMES 5

typedef enum {
    SORT_NONE,
    SORT_QTY_ASC,
    SORT_QTY_DESC,
    SORT_NAME_AZ,
    SORT_NAME_ZA,
} SortOrder;

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
} Screen;