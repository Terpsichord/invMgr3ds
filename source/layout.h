#include "main.h"

#define ITEM_HEIGHT      26.0f
#define CROSS_PAD        4.0f
#define TEXT_HPAD        3.0f
#define TEXT_VPAD        2.0f

#define INV_TOP_PAD      20.0f
#define NAME_WIDTH       150.0f
#define QUANTITY_WIDTH   40.0f
#define TAG_WIDTH        100.0f
#define TAG_SPACING      5.0f


#define VIEW_TOP_PAD     30.0f
#define VIEW_HPAD        10.0f
#define VIEW_VPAD        8.0f
#define BORDER           3.0f

#define QTY_HEIGHT       35.0f
#define QTY_WIDTH        70.0f
#define QTY_BTN_HEIGHT   30.0f
#define QTY_PADDING      2.0f

#define QTY_HPAD         3.0f
#define QTY_VPAD         5.0f

#define QTY_X            ((BOTTOM_WIDTH) - (VIEW_HPAD) - (BORDER) - 2.0f * (TEXT_HPAD) - (QTY_WIDTH))

#define QTY_LABEL_Y      ((VIEW_TOP_PAD) + (VIEW_VPAD) + (BORDER) + 20.0f)
#define INCR_Y           ((QTY_LABEL_Y) + 20.0f)
#define QTY_Y            ((INCR_Y) + (QTY_BTN_HEIGHT) + (QTY_PADDING))
#define DECR_Y           ((QTY_Y) + (QTY_HEIGHT) + (QTY_PADDING))

#define DESC_X           ((VIEW_HPAD) + (BORDER) + 4.0f * (TEXT_HPAD))
#define DESC_Y           ((VIEW_TOP_PAD) + 25.0f)
#define DESC_WIDTH       ((QTY_X) - (DESC_X) - 10.0f)

#define TAGS_X           (DESC_X)
#define TAGS_Y           ((DESC_Y) + 80.0f)
#define TAG_ROWS         3
#define TAG_HEIGHT       20.0f

#define ROW_BTN_Y        200.0f
#define ROW_BTN_HEIGHT   25.0f
#define NUM_ROW_BTNS     4
#define ROW_BTN_WIDTH    (((BOTTOM_WIDTH) - 2.0f * (VIEW_HPAD) - ((NUM_ROW_BTNS) + 1) * (BORDER) - (NUM_ROW_BTNS) * (TEXT_HPAD)) / (NUM_ROW_BTNS))

#define RENAME_X         ((VIEW_HPAD) + (BORDER) + 2.0f * (TEXT_HPAD))
#define EDIT_TAGS_X      ((RENAME_X)    + (ROW_BTN_WIDTH) + (BORDER))
#define EDIT_DESC_X      ((EDIT_TAGS_X) + (ROW_BTN_WIDTH) + (BORDER))
#define DELETE_X         ((EDIT_DESC_X) + (ROW_BTN_WIDTH) + (BORDER))

#define FILTER_HPAD      3.0f
#define FILTER_VPAD      3.0f

#define SORT_X           ((VIEW_HPAD) + (BORDER) + (FILTER_HPAD))
#define SORT_Y           ((VIEW_TOP_PAD) + (BORDER) + 2 * (VIEW_VPAD) + (FILTER_VPAD))
#define SORT_WIDTH       90.0f
#define SORT_HEIGHT      160.0f
#define SORT_SPACING     ((SORT_HEIGHT - 2 * (BORDER) - 2 * (FILTER_VPAD) - (VIEW_VPAD)) / (NUM_SORTS))

#define BOX_AREA_X       ((SORT_X) + (SORT_WIDTH))
#define BOX_X            ((BOX_AREA_X) + (BORDER) + (FILTER_HPAD))
#define BOX_Y            (SORT_Y)
#define BOX_SIZE         12.0f
#define BOX_SPACING      ((BOX_SIZE) + (2 * (FILTER_VPAD)))
#define BOX_AREA_WIDTH   ((BOTTOM_WIDTH) - (VIEW_HPAD) - (BOX_AREA_X))
#define BOX_AREA_HEIGHT  (SORT_HEIGHT)

#define SEARCH_X         (VIEW_HPAD)
#define SEARCH_Y         ((VIEW_TOP_PAD) + (VIEW_VPAD) + (SORT_HEIGHT) + 2 * (FILTER_VPAD))
#define SEARCH_WIDTH     ((BOTTOM_WIDTH) - 2 * (SEARCH_X))
#define SEARCH_HEIGHT    30.0f
#define SEARCH_PAD       2.0f

#define BAR_HEIGHT       22.0f
#define BAR_BORDER       1.0f
#define BAR_X            ((TOP_WIDTH) - 80.0f)


#define FOLDER_SPACING   (0.6f * 30.0f + 2 * (TEXT_VPAD))

#define NUM_FOLDER_BTNS  3
#define FOLDER_BTN_WIDTH (((BOTTOM_WIDTH) - 2.0f * (VIEW_HPAD) - ((NUM_FOLDER_BTNS) + 1) * (BORDER) - (NUM_FOLDER_BTNS) * (TEXT_HPAD)) / (NUM_FOLDER_BTNS))
#define NEW_FOLDER_X     ((VIEW_HPAD) + (BORDER) + 2.0f * (TEXT_HPAD))
#define RENAME_FOLDER_X  ((NEW_FOLDER_X) + (FOLDER_BTN_WIDTH) + (BORDER))
#define DELETE_FOLDER_X  ((RENAME_FOLDER_X) + (FOLDER_BTN_WIDTH) + (BORDER))