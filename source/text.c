#include "text.h"

C2D_Text nameText, qtyText, tagsText, descText, viewHintText, editHintText, filterHintText, folderHintText, emptyHintText, filterFolderHintText, gridOptionText, confirmationText,
        deleteText, cancelText, confirmText, backText, renameText, editTagsText, editDescText, newFolderText, deleteFolderText, colorFolderText, outText, sortText, filterText,
        searchText, emptyText, emptyRootText, commaText, quotesText, slashText, hashText, sortTexts[NUM_SORTS];
static C2D_TextBuf staticTextBuf;

static void addText(C2D_Text *text, const char *str) {
    C2D_TextParse(text, staticTextBuf, str);
    C2D_TextOptimize(text);
}

void initText(void) {
    staticTextBuf = C2D_TextBufNew(1024);

    addText(&nameText, "Name");
    addText(&qtyText, "Qty.");
    addText(&tagsText, "Tags");
    addText(&descText, "Description");
    addText(&viewHintText, "\uE000 Edit | \uE002 Copy | \uE003 Add | \uE005 Filter | \uE001 Back");
    addText(&editHintText, "\uE07D Adjust quantity | \uE002 Delete | \uE000/\uE001 Back");
    addText(&filterHintText, "\uE07D Change sort order | \uE005/\uE001 Back");
    addText(&folderHintText, "\uE000 Open folder | \uE005 Filter items | \uE001 Back");
    addText(&emptyHintText, "\uE003 Add new item | \uE001 Back");
    addText(&filterFolderHintText, "\uE000 Edit item | \uE005/\uE001 Back");
    addText(&gridOptionText, "Enable grid view");
    addText(&confirmationText, "Are you sure you want to delete\nthis item?");
    addText(&deleteText, "\uE002 Delete");
    addText(&cancelText, "\uE001 Cancel");
    addText(&confirmText, "\uE000 Confirm");
    addText(&backText, "\uE01A Back \uE001");
    addText(&renameText, "Rename");
    addText(&editTagsText, "Edit tags");
    addText(&editDescText, "Edit desc.");
    addText(&newFolderText, "New folder");
    addText(&deleteFolderText, "Delete");
    addText(&colorFolderText, "Edit color");
    addText(&outText, "Out of stock");
    addText(&sortText, "Sort by");
    addText(&filterText, "Filter by");
    addText(&searchText, "Enter search term...");
    addText(&emptyText, "No items...");
    addText(&emptyRootText, "Try getting started by adding a folder");
    addText(&commaText, ",");
    addText(&quotesText, "\"");
    addText(&slashText, "/");
    addText(&hashText, "#");

    addText(&sortTexts[SORT_NONE], "None");
    addText(&sortTexts[SORT_QTY_ASC], "Qty. asc.");
    addText(&sortTexts[SORT_QTY_DESC], "Qty. desc.");
    addText(&sortTexts[SORT_NAME_AZ], "Name A-Z");
    addText(&sortTexts[SORT_NAME_ZA], "Name Z-A");
}


void cleanupText(void) {
    C2D_TextBufDelete(staticTextBuf);

}