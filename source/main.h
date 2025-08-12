#pragma once

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define NUM_SORTS 5

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