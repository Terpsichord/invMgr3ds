#pragma once

#include "citro2d.h"

#include "main.h"
#include "folder.h"

#define MAX_ITEMS       512
#define MAX_ITEM_LEN    64
#define MAX_ITEM_TAGS   8
#define MAX_TAGS        32
#define MAX_FILTERS     ((MAX_TAGS) + (MAX_TOTAL_FOLDERS) + 1)
#define MAX_TAG_LEN     32
#define MAX_QUERY       32
#define MAX_QUANTITY    999

typedef struct {
    char name[MAX_ITEM_LEN];
    C2D_Text nameText;

    int quantity;
    C2D_Text qtyText;

    char desc[MAX_ITEM_LEN];
    C2D_Text descText;

    char tags[MAX_ITEM_TAGS][MAX_TAG_LEN];
    C2D_Text tagsText[MAX_ITEM_TAGS];
    int numTags;

    Folder *folders[MAX_FOLDERS];
    int numFolders;
} Item;

typedef struct {
    Item *items;
    int numItems;
    int selectedIdx;

    C2D_TextBuf textBuf;

    char tags[MAX_TAGS][MAX_TAG_LEN];
    C2D_Text tagsText[MAX_TAGS];
    int numTags;

    Folder *folders[MAX_TOTAL_FOLDERS];
    int numFolders;

    int filters[MAX_FILTERS];
    int numSelectedFilters;
    SortOrder sortOrder;
    char searchQuery[MAX_QUERY];
    C2D_Text searchQueryText;

    Folder *folderFilters[MAX_FOLDERS];
    int numFolderFilters;

    int filteredIndices[MAX_ITEMS];
    int numFiltered;
} Inventory;

typedef struct {
    int idx;
    char *name;
    int quantity;
} SortItem;

void initInventory(Inventory *inv);
void freeInventory(Inventory *inv);

int numShownItems(const Inventory *inv);

int addInventoryItem(Inventory *inv, const char *name, const char *desc, int quantity, const Folder *folders[MAX_FOLDERS], int numFolders);
void removeInventoryItem(Inventory *inv, int idx);

int inventoryAddFolders(Inventory *inv, Folder *folders[MAX_FOLDERS], int numFolders);

void inventorySearch(Inventory *inv, const char *query);
bool hasQuery(const Inventory *inv);
bool folderHasItems(const Inventory *inv, const Folder *folder);
void deleteSelectedFolder(FolderView *view, Inventory *inv);
bool isFiltered(const Inventory *inv);

int getItemIdx(const Inventory *inv, int i);
Item *getItem(const Inventory *inv, int i);
Item *getSelectedItem(const Inventory *inv);

int inventoryNumFilters(const Inventory *inv);

void inventorySetQuantity(Inventory *inv, int quantity);
void inventoryChangeQuantity(Inventory *inv, int change);

void inventoryRename(Inventory *inv, const char *name);
void inventorySetDesc(Inventory *inv, const char *desc);

void refreshItemTags(Inventory *inv, Item *item);


void updateFilteredIndices(Inventory *inv, bool pressedFilters[MAX_FILTERS]);
void updateSortOrder(Inventory *inv, bool changedOrder);
void addInventoryFilter(Inventory *inv, int filter);
void inventorySetFolderFilter(Inventory *inv, Folder *folder);

SwkbdCallbackResult validateTagInput(void *user, const char **message, const char *text, size_t textLen);