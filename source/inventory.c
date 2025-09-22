#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "inventory.h"
#include "layout.h"
#include "file.h"

static Item items[MAX_ITEMS];

void initInventory(Inventory *inv) {
    inv->items = items;
    inv->textBuf = C2D_TextBufNew(4096);
    inv->numItems = 0;
    inv->numTags = 0;
    inv->selectedIdx = 0;

    memset(inv->filters, 0, sizeof(inv->filters));
    inv->numSelectedFilters = 0;
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
            int hashIdx = i++;

            int tagLen = 0;
            while (item->desc[i] != '\0' && item->desc[i] != ' ') {
                if (tagLen == MAX_TAG_LEN - 1) break;
                item->tags[item->numTags][tagLen++] = item->desc[i++];
            }
            item->tags[item->numTags][tagLen] = '\0';

            if (tagLen == 0) continue;
            if (item->numTags == MAX_ITEM_TAGS) return -1;

            for (int j = 0; j < item->numTags; j++) {
                if (strcmp(item->tags[j], item->tags[item->numTags]) == 0) {
                    item->numTags--;
                    break;
                }
            }
            item->numTags++;

            int offset = MIN(hashIdx, 1);
            for (int j = -offset; item->desc[hashIdx + tagLen + j] != '\0'; j++) {
                item->desc[hashIdx + j] = item->desc[hashIdx + tagLen + j + offset + 1];
            }

            i = hashIdx;
        } else {
            i++;
        }
    }

    return 0;
}

int inventoryAddFolders(Inventory *inv, Folder *folders[MAX_FOLDERS], int numFolders) {
    for (int i = 0; i < numFolders; i++) {
        if (inv->numFolders >= MAX_TOTAL_FOLDERS) return -1;

        bool alreadyExists = false;
        for (int j = 0; j < inv->numFolders; j++) {
            if (folders[i] == inv->folders[j]) {
                alreadyExists = true;
                break;
            }
        }

        if (!alreadyExists) {
            inv->folders[inv->numFolders++] = folders[i];
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

int inventoryNumFilters(const Inventory *inv) {
    return inv->numTags + inv->numFolders + 1;
}

bool checkTagFilter(const Inventory *inv, const Item *item, int filter) {
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

static bool checkFolderFilter(const Inventory *inv, const Item *item, Folder *filter) {
    for (int i = 0; i < item->numFolders; i++) {
        if (item->folders[i] == filter) {
            return true;
        }
    }

    return false;
}

void updateFilteredIndices(Inventory *inv, bool pressedFilters[MAX_FILTERS]) {
    if (pressedFilters != NULL) {
        inv->numSelectedFilters = 0;
        for (int i = 0; i < inventoryNumFilters(inv); i++) {
            if (pressedFilters[i]) {
                inv->filters[inv->numSelectedFilters++] = i;
            }
        }
    }

    inv->numFiltered = 0;
    for (int i = 0; i < inv->numItems; i++) {
        for (int j = 0; j < inv->numSelectedFilters; j++) {
            if (inv->filters[j] <= inv->numTags) {
                if (!checkTagFilter(inv, &inv->items[i], inv->filters[j])) {
                    goto noMatch;
                }
            } else {
                if (!checkFolderFilter(inv, &inv->items[i], inv->folders[inv->filters[j] - inv->numTags - 1])) {
                    goto noMatch;
                }
            }
        }
        for (int j = 0; j < inv->numFolderFilters; j++) {
            if (!checkFolderFilter(inv, &inv->items[i], inv->folderFilters[j])) {
                goto noMatch;
            }
        }

        if (hasQuery(inv) && strcasestr(inv->items[i].name, inv->searchQuery) == NULL) {
            goto noMatch;
        }

        inv->filteredIndices[inv->numFiltered++] = i;

        noMatch:
    }

    sortFilteredIndices(inv);

    if (pressedFilters != NULL) {
        inv->selectedIdx = 0;
    }
}

void inventorySetFolderFilter(Inventory *inv, Folder *folder) {
    if (folder->parent == NULL) {
        inv->numFolderFilters = 0;
    } else {
        inv->numFolderFilters = 1;
        inv->folderFilters[0] = folder;
    }
    updateFilteredIndices(inv, NULL);
    if (inv->selectedIdx >= numShownItems(inv)) {
        inv->selectedIdx = numShownItems(inv) - 1;
    }
}

void updateSortOrder(Inventory *inv, bool changedOrder) {
    sortFilteredIndices(inv);
    if (changedOrder) {
        inv->selectedIdx = 0;
    }
}

void addInventoryFilter(Inventory *inv, int filter) {
    if (inv->numSelectedFilters >= MAX_FILTERS) return;

    inv->filters[inv->numSelectedFilters++] = filter;
}

int updateInventoryTags(Inventory *inv) {
    inv->numFolders = 0;
    inv->numTags = 0;

    for (int i = 0; i < inv->numItems; i++) {
        inventoryAddFolders(inv, inv->items[i].folders, inv->items[i].numFolders);
        inventoryAddTags(inv, inv->items[i].tags, inv->items[i].tagsText, inv->items[i].numTags);
    }


    updateFilteredIndices(inv, NULL);

    return 0;
}

bool isFiltered(const Inventory *inv) {
    return inv->numSelectedFilters > 0 || inv->sortOrder != SORT_NONE || hasQuery(inv) || inv->numFolderFilters > 0;
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
int addInventoryItem(Inventory *inv, const char *name, const char *desc, int quantity, const Folder *folders[MAX_FOLDERS],
                     int numFolders) {
    if (inv->numItems >= MAX_ITEMS) return -1;

    Item item = {
            .quantity = quantity,
            .numTags = 0,
            .numFolders = numFolders,
    };

    strncpy(item.name, name, MAX_ITEM_LEN - 1);
    strncpy(item.desc, desc, MAX_ITEM_LEN - 1);
    memcpy(item.folders, folders, sizeof(Folder *) * numFolders);

    int tagRes = parseItemTags(&item);
    parseItemText(inv, &item);

    inv->items[inv->numItems] = item;
    inv->numItems++;

    inventoryAddTags(inv, item.tags, item.tagsText, item.numTags);
    inventoryAddFolders(inv, item.folders, item.numFolders);
    updateFilteredIndices(inv, NULL);

    return tagRes < 0 ? -2 : 0;
}

void removeInventoryItem(Inventory *inv, int idx) {
    inv->numItems--;
    for (int i = idx; i < inv->numItems; i++) {
        inv->items[i] = inv->items[i + 1];
    }

    updateInventoryTags(inv);
    updateInventoryText(inv);
    updateFilteredIndices(inv, NULL);

    if (inv->selectedIdx >= numShownItems(inv)) {
        inv->selectedIdx = numShownItems(inv) - 1;
    }
}

void removeInventoryItemTag(Inventory *inv, int idx, int tagIdx) {
    Item *item = &inv->items[idx];
    item->numTags--;
    for (int i = tagIdx; i < item->numTags; i++) {
        strncpy(item->tags[i], item->tags[i + 1], MAX_TAG_LEN - 1);
    }
    updateInventoryTags(inv);
    updateInventoryText(inv);
}

void addInventoryItemTags(Inventory *inv, int idx, const char *tags[], int numTags) {
    Item *item = &inv->items[idx];
    for (int i = 0; i < numTags; i++) {
        if (item->numTags >= MAX_TAGS) return;
        strncpy(item->tags[item->numTags], tags[i], MAX_TAG_LEN - 1);
        item->numTags++;
    }

    updateInventoryTags(inv);
    updateInventoryText(inv);
}

void addNewTag(Inventory *inv, const char *tag) {
    addInventoryItemTags(inv, inv->selectedIdx, &tag, 1);
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


bool folderHasItems(const Inventory *inv, const Folder *folder) {
    for (int i = 0; i < inv->numItems; i++) {
        if (inv->items[i].folders[0] == folder) return true;
    }
    return false;
}


void deleteSelectedFolder(FolderView *view, Inventory *inv) {
    if (view->selectedIdx >= view->currentFolder->numChildren) return;

    for (int i = 0; i < inv->numItems; i++) {
        for (int j = 0; j < inv->items[i].numFolders; j++) {
            if (inv->items[i].folders[j] == getSelectedFolder(view)) {
                removeInventoryItem(inv, i--);
                break;
            }
        }
    }

    freeFolder(getSelectedFolder(view));

    view->currentFolder->numChildren--;
    for (int i = view->selectedIdx; i < view->currentFolder->numChildren; i++) {
        view->currentFolder->children[i] = view->currentFolder->children[i + 1];
    }
    if (view->selectedIdx >= view->currentFolder->numChildren) {
        view->selectedIdx = view->currentFolder->numChildren - 1;
    }

    updateFolderViewText(view);
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

SwkbdCallbackResult validateSingleTagInput(void *user, const char **message, const char *text, size_t textLen) {
    for (size_t i = 0; i < textLen; i++) {
        if (text[i] == ' ') {
            *message = "Tag cannot have spaces";
            return SWKBD_CALLBACK_CONTINUE;
        }

        if (text[i] == '#') {
            *message = "Tag cannot contain #";
            return SWKBD_CALLBACK_CONTINUE;
        }
    }

    return SWKBD_CALLBACK_OK;
}

