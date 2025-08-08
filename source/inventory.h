#include "citro2d.h"
#include "main.h"

#define MAX_ITEMS       512
#define MAX_ITEM_LEN    64
#define MAX_ITEM_TAGS   8
#define MAX_TAGS        32
#define MAX_FILTERS     ((MAX_TAGS) + 1)
#define MAX_TAG_LEN     32
#define MAX_QUERY       32
#define MAX_QUANTITY    999

#define FILE_PATH       "sdmc:/3ds/invMgr3ds/"

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
} Item;

typedef struct {
    Item *items;
    int numItems;
    int selectedIdx;

    C2D_TextBuf textBuf;

    char tags[MAX_TAGS][MAX_TAG_LEN];
    C2D_Text tagsText[MAX_TAGS];
    int numTags;

    int filters[MAX_FILTERS];
    int numFilters;
    SortOrder sortOrder;
    char searchQuery[MAX_QUERY];
    C2D_Text searchQueryText;

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

int addInventoryItem(Inventory *inv, const char *name, const char *desc, int quantity);
void removeInventoryItem(Inventory *inv, int idx);

void inventorySearch(Inventory *inv, const char *query);
bool hasQuery(const Inventory *inv);
bool isFiltered(const Inventory *inv);

Item *getSelectedItem(const Inventory *inv);

void inventorySetQuantity(Inventory *inv, int quantity);
void inventoryChangeQuantity(Inventory *inv, int change);

void inventoryRename(Inventory *inv, const char *name);
void inventorySetDesc(Inventory *inv, const char *desc);

void refreshItemTags(Inventory *inv, Item *item);

void updateFilteredIndices(Inventory *inv, bool pressedFilters[MAX_FILTERS]);
void updateSortOrder(Inventory *inv, bool changedOrder);
void addInventoryFilter(Inventory *inv, int filter);

void drawItemList(const Inventory *inv, float scrollOffset);
void drawItemView(const Inventory *inv, bool editing, const TouchState *touchState);
void drawSortView(const Inventory *inv);
void drawFilterView(const Inventory *inv, bool pressedFilters[], float scroll);
void drawSearchBar(const Inventory *inv);
void drawFiltering(const Inventory *inv);

SwkbdCallbackResult validateTagInput(void *user, const char **message, const char *text, size_t textLen);

void loadInventory(Inventory *inv);
void saveInventory(const Inventory *inv);