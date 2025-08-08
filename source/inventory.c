#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "inventory.h"
#include "layout.h"

static Item items[MAX_ITEMS];

void initInventory(Inventory *inv) {
    inv->items = items;
    inv->textBuf = C2D_TextBufNew(4096);
    inv->numItems = 0;
    inv->numTags = 0;
    inv->selectedIdx = 0;

    memset(inv->filters, 0, sizeof(inv->filters));
    inv->numFilters = 0;
    inv->sortOrder = SORT_NONE;

    inv->numFiltered = 0;
}

void freeInventory(Inventory *inv) {
    C2D_TextBufDelete(inv->textBuf);
}

static void parseItemText(Inventory *inv, Item *item) {
    char *qtyStr = malloc(snprintf(NULL, 0, "%d", item->quantity) + 1);
    sprintf(qtyStr, "%d", item->quantity);
    C2D_TextParse(&item->qtyText, inv->textBuf, qtyStr);
    C2D_TextOptimize(&item->qtyText);
    free(qtyStr);

    static char tagBuf[MAX_TAG_LEN + 1];
    for (int i = 0; i < item->numTags; i++) {
        snprintf(tagBuf, sizeof(tagBuf), "#%s", item->tags[i]);
        C2D_TextParse(&item->tagsText[i], inv->textBuf, tagBuf);
        C2D_TextOptimize(&item->tagsText[i]);
    }

    C2D_TextParse(&item->nameText, inv->textBuf, item->name);
    C2D_TextOptimize(&item->nameText);

    C2D_TextParse(&item->descText, inv->textBuf, item->desc);
    C2D_TextOptimize(&item->descText);
}

// only needs to be called when removing/modifying an item as this clears and rebuilds the whole text buffer
static void updateInventoryText(Inventory *inv) {
    C2D_TextBufClear(inv->textBuf);

    for (int i = 0; i < inv->numItems; i++) {
        parseItemText(inv, &inv->items[i]);
    }

    static char tagBuf[MAX_TAG_LEN + 1];
    for (int i = 0; i < inv->numTags; i++) {
        snprintf(tagBuf, sizeof(tagBuf), "#%s", inv->tags[i]);
        C2D_TextParse(&inv->tagsText[i], inv->textBuf, tagBuf);
        C2D_TextOptimize(&inv->tagsText[i]);
    }

}

int parseItemTags(Item *item) {
    int i = 0;
    item->numTags = 0;
    while (item->desc[i] != '\0') {
        if (item->desc[i] == '#' && (i == 0 || item->desc[i - 1] == ' ')) {
            int hashIdx = i++, j = 0;

            while (item->desc[i] != '\0' && item->desc[i] != ' ') {
                if (j == MAX_TAG_LEN - 1) break;
                item->tags[item->numTags][j++] = item->desc[i++];
            }
            if (j == 0) continue;
            if (item->numTags == MAX_ITEM_TAGS) return -1;

            item->tags[item->numTags][j] = '\0';


            int l = MIN(hashIdx, 1);
            for (int k = -l; item->desc[hashIdx + k + j] != '\0'; k++) {
                item->desc[hashIdx + k] = item->desc[hashIdx + k + j + l + 1];
            }
            i = hashIdx;

            for (int j = 0; j < item->numTags; j++) {
                if (strcmp(item->tags[j], item->tags[item->numTags]) == 0) {
                    item->numTags--;
                    break;
                }
            }

            item->numTags++;
        } else {
            i++;
        }
    }

    return 0;
}

int inventoryAddTags(Inventory *inv, const char tags[MAX_ITEM_TAGS][MAX_TAG_LEN], C2D_Text tagsText[MAX_ITEM_TAGS],
                     int numTags) {
    for (int i = 0; i < numTags; i++) {
        if (inv->numTags >= MAX_TAGS) return -1;

        bool alreadyExists = false;
        for (int j = 0; j < inv->numTags; j++) {
            if (strcmp(tags[i], inv->tags[j]) == 0) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            inv->tagsText[inv->numTags] = tagsText[i];
            strncpy(inv->tags[inv->numTags++], tags[i], MAX_TAG_LEN - 1);
        }
    }

    return 0;
}

int cmpQtyAsc(const void *a, const void *b) {
    SortItem *item1 = (SortItem *) a, *item2 = (SortItem *) b;
    return item1->quantity - item2->quantity;
}

int cmpQtyDesc(const void *a, const void *b) {
    SortItem *item1 = (SortItem *) a, *item2 = (SortItem *) b;
    return item2->quantity - item1->quantity;
}

int cmpNameAZ(const void *a, const void *b) {
    SortItem *item1 = (SortItem *) a, *item2 = (SortItem *) b;
    return strcasecmp(item1->name, item2->name);
}

int cmpNameZA(const void *a, const void *b) {
    SortItem *item1 = (SortItem *) a, *item2 = (SortItem *) b;
    return strcasecmp(item2->name, item1->name);
}

void sortFilteredIndices(Inventory *inv) {
    int (*cmp)(const void *, const void *);
    switch (inv->sortOrder) {
        case SORT_QTY_ASC:
            cmp = cmpQtyAsc;
            break;
        case SORT_QTY_DESC:
            cmp = cmpQtyDesc;
            break;
        case SORT_NAME_AZ:
            cmp = cmpNameAZ;
            break;
        case SORT_NAME_ZA:
            cmp = cmpNameZA;
            break;
        default:
            return;
    }

    SortItem items[MAX_ITEMS];
    for (int i = 0; i < inv->numFiltered; i++) {
        items[i] = (SortItem) {
                .idx = inv->filteredIndices[i],
                .name = inv->items[inv->filteredIndices[i]].name,
                .quantity = inv->items[inv->filteredIndices[i]].quantity,
        };
    }
    qsort(items, inv->numFiltered, sizeof(SortItem), cmp);

    for (int i = 0; i < inv->numFiltered; i++) {
        inv->filteredIndices[i] = items[i].idx;
    }
}

bool checkItemFilter(const Inventory *inv, const Item *item, int filter) {
    if (filter == 0) {
        return item->quantity == 0;
    }

    for (int i = 0; i < item->numTags; i++) {
        if (strcmp(inv->tags[filter - 1], item->tags[i]) == 0) {
            return true;
        }
    }

    return false;
}

void updateFilteredIndices(Inventory *inv, bool pressedFilters[MAX_FILTERS]) {
    if (pressedFilters != NULL) {
        inv->numFilters = 0;
        for (int i = 0; i <= inv->numTags; i++) {
            if (pressedFilters[i]) {
                inv->filters[inv->numFilters++] = i;
            }
        }
    }

    inv->numFiltered = 0;
    for (int i = 0; i < inv->numItems; i++) {
        bool matches = true;
        for (int j = 0; j < inv->numFilters; j++) {
            if (!checkItemFilter(inv, &inv->items[i], inv->filters[j])) {
                matches = false;
                break;
            }
        }
        if (matches && hasQuery(inv) && strcasestr(inv->items[i].name, inv->searchQuery) == NULL) {
            matches = false;
        }

        if (matches) {
            inv->filteredIndices[inv->numFiltered++] = i;
        }
    }

    sortFilteredIndices(inv);

    if (pressedFilters != NULL) {
        inv->selectedIdx = 0;
    }
}

void updateSortOrder(Inventory *inv, bool changedOrder) {
    sortFilteredIndices(inv);
    if (changedOrder) {
        inv->selectedIdx = 0;
    }
}

void addInventoryFilter(Inventory *inv, int filter) {
    if (inv->numFilters >= MAX_FILTERS) return;

    inv->filters[inv->numFilters++] = filter;
}

int updateInventoryTags(Inventory *inv) {
    inv->numTags = 0;

    for (int i = 0; i < inv->numItems; i++) {
        int res = inventoryAddTags(inv, inv->items[i].tags, inv->items[i].tagsText, inv->items[i].numTags);
        if (res < 0) return res;
    }

    updateFilteredIndices(inv, NULL);

    return 0;
}

bool isFiltered(const Inventory *inv) {
    return inv->numFilters > 0 || inv->sortOrder != SORT_NONE || hasQuery(inv);
}

int getItemIdx(const Inventory *inv, int i) {
    if (isFiltered(inv)) {
        return inv->filteredIndices[i];
    } else {
        return i;
    }
}

Item *getItem(const Inventory *inv, int i) {
    return &inv->items[getItemIdx(inv, i)];
}

Item *getSelectedItem(const Inventory *inv) {
    return getItem(inv, inv->selectedIdx);
}


int numShownItems(const Inventory *inv) {
    if (isFiltered(inv)) {
        return inv->numFiltered;
    } else {
        return inv->numItems;
    }
}

// returns -1 if max items reached, returns -2 if item has too many tags (excess tags are ignored)
// todo: error handle this function
int addInventoryItem(Inventory *inv, const char *name, const char *desc, int quantity) {
    if (inv->numItems >= MAX_ITEMS) return -1;

    Item item = {
            .quantity = quantity,
            .numTags = 0,
    };

    strncpy(item.name, name, MAX_ITEM_LEN - 1);
    strncpy(item.desc, desc, MAX_ITEM_LEN - 1);

    int tagRes = parseItemTags(&item);
    parseItemText(inv, &item);

    inv->items[inv->numItems] = item;
    inv->numItems++;

    inventoryAddTags(inv, item.tags, item.tagsText, item.numTags);
    updateFilteredIndices(inv, NULL);

    return tagRes < 0 ? -2 : 0;
}

void removeInventoryItem(Inventory *inv, int idx) {
    inv->numItems--;
    for (int i = idx; i < inv->numItems; i++) {
        inv->items[i] = inv->items[i + 1];
    }
    if (inv->selectedIdx >= numShownItems(inv)) {
        inv->selectedIdx = numShownItems(inv) - 1;
    }

    updateInventoryTags(inv);
    updateInventoryText(inv);
}

void inventorySearch(Inventory *inv, const char *query) {
    strncpy(inv->searchQuery, query, MAX_QUERY - 1);
    C2D_TextParse(&inv->searchQueryText, inv->textBuf, inv->searchQuery);
    C2D_TextOptimize(&inv->searchQueryText);

    updateFilteredIndices(inv, NULL);
}

bool hasQuery(const Inventory *inv) {
    return inv->searchQuery[0] != '\0';
}

void drawItemList(const Inventory *inv, float scrollOffset) {
    if (numShownItems(inv) == 0) return;

    for (int i = 0; i < numShownItems(inv); i++) {
        if (INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset > SCREEN_HEIGHT || INV_TOP_PAD + ITEM_HEIGHT * (i + 1) - scrollOffset < 0) continue;

        u32 bgColor = white;

        if (inv->selectedIdx == i) {
            bgColor = gray;
            C2D_DrawRectSolid(0.0f, INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f, TOP_WIDTH, ITEM_HEIGHT, gray);
        }

        C2D_DrawText(&getItem(inv, i)->nameText, 0, TEXT_HPAD, INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                     0.0f, 0.6f,
                     0.6f);

        C2D_DrawRectSolid(NAME_WIDTH - 2 * TEXT_HPAD, INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f,
                          TOP_WIDTH, ITEM_HEIGHT, bgColor);

        C2D_DrawText(&getItem(inv, i)->qtyText, 0, NAME_WIDTH + TEXT_HPAD,
                     INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                     0.0f, 0.6f, 0.6f);

        if (getItem(inv, i)->numTags > 0) {
            float width, indent = NAME_WIDTH + QUANTITY_WIDTH + TAG_SPACING;
            for (int j = 0; j < getItem(inv, i)->numTags; j++) {
                C2D_TextGetDimensions(&getItem(inv, i)->tagsText[j], 0.55f, 0.55f, &width, NULL);
                C2D_DrawText(&getItem(inv, i)->tagsText[j], 0, indent,
                             INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset,
                             0.0f, 0.55f, 0.55f);
                indent += width + TAG_SPACING;
            }
        }

        C2D_DrawRectSolid(NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH - TEXT_HPAD,
                          INV_TOP_PAD + ITEM_HEIGHT * i - scrollOffset, 0.0f,
                          TOP_WIDTH, ITEM_HEIGHT, bgColor);

        C2D_DrawText(&getItem(inv, i)->descText, 0, NAME_WIDTH + QUANTITY_WIDTH + TAG_WIDTH + TEXT_HPAD,
                     INV_TOP_PAD + ITEM_HEIGHT * i + TEXT_VPAD - scrollOffset, 0.0f, 0.55f, 0.55f);
    }
}

static void drawQuantity(const Item *item, const TouchState *touchState) {
    C2D_DrawText(&qtyText, 0, QTY_X, QTY_LABEL_Y, 0.0f, 0.5f, 0.5f);

    C2D_DrawRectSolid(QTY_X, INCR_Y, 0.0f, QTY_WIDTH, QTY_BTN_HEIGHT, touchState->item == TOUCH_INCR ? darkGray : gray);
    C2D_DrawRectSolid(QTY_X, QTY_Y, 0.0f, QTY_WIDTH, QTY_HEIGHT, lightGray);
    C2D_DrawRectSolid(QTY_X, DECR_Y, 0.0f, QTY_WIDTH, QTY_BTN_HEIGHT, touchState->item == TOUCH_DECR ? darkGray : gray);

    u32 incrColor = touchState->item == TOUCH_INCR ? white : darkGray;
    C2D_DrawTriangle(QTY_X + QTY_WIDTH / 2 - 8, INCR_Y + QTY_BTN_HEIGHT / 2 + 3, incrColor,
                     QTY_X + QTY_WIDTH / 2 + 8, INCR_Y + QTY_BTN_HEIGHT / 2 + 3, incrColor,
                     QTY_X + QTY_WIDTH / 2, INCR_Y + QTY_BTN_HEIGHT / 2 - 6, incrColor, 0);

    u32 decrColor = touchState->item == TOUCH_DECR ? white : darkGray;
    C2D_DrawTriangle(QTY_X + QTY_WIDTH / 2 - 8, DECR_Y + QTY_BTN_HEIGHT / 2 - 3, decrColor,
                     QTY_X + QTY_WIDTH / 2 + 8, DECR_Y + QTY_BTN_HEIGHT / 2 - 3, decrColor,
                     QTY_X + QTY_WIDTH / 2, DECR_Y + QTY_BTN_HEIGHT / 2 + 6, decrColor, 0);

    C2D_DrawText(&item->qtyText, 0, QTY_X + QTY_HPAD + TEXT_HPAD, QTY_Y + QTY_VPAD + TEXT_VPAD, 0.0f, 0.6f, 0.6f);
}

static void drawTags(const Item *item) {
    float indent = TAGS_X;
    for (int i = 0; i < (item->numTags + TAG_ROWS - 1) / TAG_ROWS; i++) {
        float width, maxWidth = 0.0f;
        for (int j = 0; j < TAG_ROWS; j++) {
            int idx = i * TAG_ROWS + j;
            if (idx < item->numTags) {
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
                          SCREEN_HEIGHT - 2 * VIEW_VPAD - VIEW_TOP_PAD, gray);
        C2D_DrawRectSolid(VIEW_HPAD + BORDER, VIEW_VPAD + VIEW_TOP_PAD + BORDER, 0.0f,
                          BOTTOM_WIDTH - 2 * VIEW_HPAD - 2 * BORDER,
                          SCREEN_HEIGHT - 2 * VIEW_VPAD - 2 * BORDER - VIEW_TOP_PAD, white);

        float w, h;
        C2D_TextGetDimensions(&item->nameText, 0.6f, 0.6f, &w, &h);
        C2D_DrawRectSolid(VIEW_HPAD + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD,
                          white);
    }

    C2D_DrawText(&item->nameText, 0, VIEW_HPAD + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);
    C2D_DrawText(&item->descText, C2D_WordWrap, DESC_X, DESC_Y, 0.0f, 0.55f, 0.55f, DESC_WIDTH);

    drawTags(item);

    // hacky coverup to stop tags and long words in description going outside their area
    C2D_DrawRectSolid(DESC_X + DESC_WIDTH, DESC_Y, 0.0f, BOTTOM_WIDTH - VIEW_HPAD - BORDER - DESC_X - DESC_WIDTH,
                      SCREEN_HEIGHT - VIEW_VPAD - DESC_Y - BORDER, white);
    C2D_DrawRectSolid(BOTTOM_WIDTH - VIEW_HPAD, DESC_Y, 0.0f, VIEW_HPAD, SCREEN_HEIGHT - VIEW_VPAD - DESC_Y - BORDER,
                      white);
    if (editing) {
        C2D_DrawRectSolid(BOTTOM_WIDTH - VIEW_HPAD - BORDER, DESC_Y, 0.0f, BORDER, SCREEN_HEIGHT - VIEW_VPAD - DESC_Y,
                          gray);
    }

    drawQuantity(item, touchState);
}

void drawSortView(const Inventory *inv) {
    C2D_DrawRectSolid(VIEW_HPAD, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, SORT_WIDTH, SORT_HEIGHT, gray);
    C2D_DrawRectSolid(VIEW_HPAD + BORDER, VIEW_TOP_PAD + VIEW_VPAD + BORDER, 0.0f, SORT_WIDTH - 2 * BORDER,
                      SORT_HEIGHT - 2 * BORDER, white);

    float w, h;
    C2D_TextGetDimensions(&sortText, 0.6f, 0.6f, &w, &h);
    C2D_DrawRectSolid(VIEW_HPAD + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD, white);
    C2D_DrawText(&sortText, 0, VIEW_HPAD + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);

    for (int i = 0; i < NUM_SORTS; i++) {
        if (i == inv->sortOrder) {
            C2D_DrawRectSolid(SORT_X, SORT_Y + SORT_SPACING * i, 0.0f, SORT_WIDTH - 2 * BORDER - 2 * FILTER_HPAD,
                              SORT_SPACING, gray);
        }
        C2D_DrawText(&sortTexts[i], 0, SORT_X + TEXT_HPAD, SORT_Y + SORT_SPACING * i + 2 * TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }
}

const C2D_Text *getFilterText(const Inventory *inv, int filter) {
    if (filter == 0) {
        return &outText;
    } else {
        return &inv->tagsText[filter - 1];
    }
}

void drawFilterView(const Inventory *inv, bool pressedFilters[], float scroll) {
    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BOX_AREA_WIDTH, BOX_AREA_HEIGHT, gray);
    C2D_DrawRectSolid(BOX_AREA_X + BORDER, VIEW_TOP_PAD + VIEW_VPAD + BORDER, 0.0f, BOX_AREA_WIDTH - 2 * BORDER,
                      BOX_AREA_HEIGHT - 2 * BORDER, white);

    for (int i = 0; i <= inv->numTags; i++) {
        if (BOX_SPACING * (i + 1) - VIEW_VPAD - scroll + 2 * FILTER_VPAD < 0 || BOX_SPACING * i - scroll > BOX_AREA_HEIGHT - VIEW_VPAD - 2 * FILTER_VPAD - BORDER) continue;
        C2D_DrawRectSolid(BOX_X, BOX_Y + BOX_SPACING * i - scroll, 0.0f, BOX_SIZE, BOX_SIZE, black);

        if (!pressedFilters[i]) {
            C2D_DrawRectSolid(BOX_X + 1, BOX_Y + BOX_SPACING * i + 1 - scroll, 0.0f, BOX_SIZE - 2, BOX_SIZE - 2, white);
        }

        C2D_DrawText(getFilterText(inv, i), 0, BOX_X + BOX_SIZE + 2 * FILTER_HPAD, BOX_Y + BOX_SPACING * i - FILTER_VPAD + TEXT_VPAD - scroll,
                     0.0f, 0.5f, 0.5f);
    }

    float w, h;
    C2D_TextGetDimensions(&filterText, 0.6f, 0.6f, &w, &h);
    C2D_DrawRectSolid(BOX_AREA_X + BORDER + TEXT_HPAD, VIEW_TOP_PAD, 0.0f, w + 2 * TEXT_HPAD, h + 2 * TEXT_VPAD, white);
    C2D_DrawText(&filterText, 0, BOX_AREA_X + BORDER + 2 * TEXT_HPAD, VIEW_TOP_PAD, 0.0f, 0.6f, 0.6f);

    C2D_DrawRectSolid(BOX_AREA_X + BORDER + 3 * TEXT_HPAD + w, VIEW_TOP_PAD + VIEW_VPAD, 0.0f, BOX_AREA_WIDTH - BORDER - 3 * TEXT_HPAD - w, BORDER, gray);
    C2D_DrawRectSolid(BOX_AREA_X + BORDER + 3 * TEXT_HPAD + w, VIEW_TOP_PAD + VIEW_VPAD + BORDER, 0.0f, BOX_AREA_WIDTH - 2 * BORDER - 3 * TEXT_HPAD - w, h + 2 * TEXT_VPAD - VIEW_VPAD - BORDER, white);
    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD + BOX_AREA_HEIGHT - BORDER, 0.0f, BOX_AREA_WIDTH, BORDER, gray);
    C2D_DrawRectSolid(BOX_AREA_X, VIEW_TOP_PAD + VIEW_VPAD + BOX_AREA_HEIGHT, 0.0f, BOX_AREA_WIDTH, SCREEN_HEIGHT, white);
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
    C2D_DrawRectSolid(SEARCH_X, SEARCH_Y, 0.0f, SEARCH_WIDTH, SEARCH_HEIGHT, gray);
    C2D_DrawRectSolid(SEARCH_X + BORDER, SEARCH_Y + BORDER, 0.0f, SEARCH_WIDTH - 2 * BORDER, SEARCH_HEIGHT - 2 * BORDER,
                      white);

    const C2D_Text *text = hasQuery(inv) ? &inv->searchQueryText : &searchText;
    C2D_DrawText(text, 0, SEARCH_X + BORDER + TEXT_HPAD, SEARCH_Y + BORDER + TEXT_VPAD, 0.0f, 0.6f, 0.6f);

    if (!hasQuery(inv)) return;
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH - SEARCH_HEIGHT + BORDER - TEXT_HPAD, SEARCH_Y + BORDER, 0.0f, SEARCH_HEIGHT - 2 * BORDER + TEXT_HPAD, SEARCH_HEIGHT - 2 * BORDER, white);
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH - BORDER, SEARCH_Y, 0.0f, BORDER, SEARCH_HEIGHT, gray);
    C2D_DrawRectSolid(SEARCH_X + SEARCH_WIDTH, SEARCH_Y, 0.0f, SEARCH_X, SEARCH_HEIGHT, white);
    drawCross(SEARCH_X + SEARCH_WIDTH - SEARCH_HEIGHT + BORDER + SEARCH_PAD, SEARCH_Y + BORDER + SEARCH_PAD,
              SEARCH_HEIGHT - 2 * BORDER - 2 * SEARCH_PAD - 1.0f, 1.0f, black);
}

void drawItemView(const Inventory *inv, bool editing, const TouchState *touchState) {
    drawSelected(getSelectedItem(inv), editing, touchState);
}

void drawFiltering(const Inventory *inv) {
    if (!isFiltered(inv)) return;

    C2D_DrawRectSolid(0.0f, SCREEN_HEIGHT - BAR_HEIGHT, 0.0f, TOP_WIDTH, BAR_BORDER, black);
    C2D_DrawRectSolid(0.0f, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER, 0.0f, TOP_WIDTH, BAR_HEIGHT - BAR_BORDER, white);

    float commaWidth, quotesWidth;
    C2D_TextGetDimensions(&commaText, 0.5f, 0.5f, &commaWidth, NULL);
    C2D_TextGetDimensions(&quotesText, 0.5f, 0.5f, &quotesWidth, NULL);

    float width, indent = width + 2 * TEXT_HPAD;
    if (inv->numFilters > 0) {
        for (int i = 0; i < inv->numFilters - 1; i++) {
            C2D_TextGetDimensions(getFilterText(inv, inv->filters[i]), 0.5f, 0.5f, &width, NULL);
            C2D_DrawText(getFilterText(inv, inv->filters[i]), 0, indent, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f,
                         0.5f);
            indent += width;
            C2D_DrawText(&commaText, 0, indent, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
            indent += commaWidth + 2 * TEXT_HPAD;
        }
        C2D_TextGetDimensions(getFilterText(inv, inv->filters[inv->numFilters - 1]), 0.5f, 0.5f, &width, NULL);
        C2D_DrawText(getFilterText(inv, inv->filters[inv->numFilters - 1]), 0, indent, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f,
                     0.5f);
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


    C2D_DrawRectSolid(coverStart, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER, 0.0f, TOP_WIDTH, BAR_HEIGHT - BAR_BORDER, white);

    if (inv->sortOrder != SORT_NONE) {
        C2D_DrawText(&sortTexts[inv->sortOrder], 0, sortEnd - sortWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }

    if (hasQuery(inv)) {
        C2D_DrawText(&quotesText, 0, queryEnd - queryWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
        C2D_DrawText(&inv->searchQueryText, 0, queryEnd - queryWidth + quotesWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
        C2D_DrawText(&quotesText, 0, queryEnd - quotesWidth, SCREEN_HEIGHT - BAR_HEIGHT + BAR_BORDER + TEXT_VPAD, 0.0f, 0.5f, 0.5f);
    }
}

void inventorySetQuantity(Inventory *inv, int quantity) {
    if (quantity > MAX_QUANTITY) quantity = MAX_QUANTITY;

    getSelectedItem(inv)->quantity = MAX(quantity, 0);

    updateInventoryText(inv);
    updateFilteredIndices(inv, NULL);
}

void inventoryChangeQuantity(Inventory *inv, int change) {
    inventorySetQuantity(inv, getSelectedItem(inv)->quantity + change);
}

void inventoryRename(Inventory *inv, const char *name) {
    strncpy(getSelectedItem(inv)->name, name, MAX_ITEM_LEN);
    updateInventoryText(inv);
    updateSortOrder(inv, false);
}

void refreshItemTags(Inventory *inv, Item *item) {
    parseItemTags(item);
    updateInventoryTags(inv);
    updateInventoryText(inv);
}

void inventorySetDesc(Inventory *inv, const char *desc) {
    strncpy(getSelectedItem(inv)->desc, desc, MAX_ITEM_LEN - 1);
    refreshItemTags(inv, getSelectedItem(inv));
}

SwkbdCallbackResult validateTagInput(void *user, const char **message, const char *text, size_t textLen) {
    size_t i = 0;
    size_t tagCount = 0;
    while (i < textLen) {
        if (text[i] == ' ') {
            i++;
            continue;
        } else if (text[i] != '#') {
            *message = "Tags must start with #";
            return SWKBD_CALLBACK_CONTINUE;
        }
        size_t j = ++i;
        while (j < textLen && text[j] != ' ') {
            j++;
        }
        if (j == i) {
            *message = "Tag cannot be empty";
            return SWKBD_CALLBACK_CONTINUE;
        }
        if (j - i > MAX_TAG_LEN) {
            *message = "Tag length too long";
            return SWKBD_CALLBACK_CONTINUE;
        }
        tagCount++;
        if (tagCount > MAX_ITEM_TAGS) {
            *message = "Too many tags";
            return SWKBD_CALLBACK_CONTINUE;
        }
        i = j;
    }
    return SWKBD_CALLBACK_OK;
}

int readItem(char *buf, char *name, int *quantity, char *desc) {
    return sscanf(buf, "%63[^\t]\t%d\t%63[^\n]", name, quantity, desc); // todo: update 63 with MAX_ITEM_LEN - 1 if it changes
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}

void loadInventory(Inventory *inv) {
    FILE *f = fopen(FILE_PATH "items.txt", "r");
    if (!f) return;

    char line[2 * MAX_ITEM_LEN + 6];
    char name[MAX_ITEM_LEN];
    int quantity = 0;
    char desc[MAX_ITEM_LEN];

    while (fgets(line, sizeof(line), f)) {
        name[0] = '\0';
        desc[0] = '\0';

        int res = readItem(line, name, &quantity, desc);
        if (res > 3 || res < 1) continue;

        addInventoryItem(inv, name, desc, quantity);
    }
    fclose(f);
}


void makeMissingDir(const char *path) {
    struct stat st;

    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
        // already exists
        return;
    }

    mkdir(path, 0777);
}

void writeItem(FILE *f, const Item *item) {
    static char descBuf[MAX_ITEM_LEN];

    strncpy(descBuf, item->desc, MAX_ITEM_LEN - 1);
    for (int j = 0; j < item->numTags; j++) {
        strcat(descBuf, " #");
        strncat(descBuf, item->tags[j], MAX_TAG_LEN - 1);
    }

    fprintf(f, "%s\t%d\t%s\n", item->name, item->quantity, descBuf);
}

void saveInventory(const Inventory *inv) {
    makeMissingDir(FILE_PATH);

    FILE *f = fopen(FILE_PATH "items.txt", "w");
    if (f == NULL) return;

    for (int i = 0; i < inv->numItems; i++) {
        writeItem(f, &inv->items[i]);
    }

    fclose(f);
}