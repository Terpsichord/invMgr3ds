#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "layout.h"
#include "file.h"

int readItem(char *buf, char *name, int *quantity, char *desc, char *path) {
    int ret = sscanf(buf, "%63[^\t\n]\t%d\t%63[^\t\n]\t%4111[^\n]", name, quantity, desc, path); // todo: update 63 and 4111 if needed
    if (ret < 4) {
        // scan without description
        ret = sscanf(buf, "%63[^\t\n]\t%d\t\t%4111[^\n]", name, quantity, path);
        if (ret == 3) {
            desc[0] = '\0';
            ret = 4;
        }
    }

    return ret;
}

void loadItemFolders(Item *item, FolderView *view, char *path) {
    char *name, *remaining = path;
    Folder *parent = view->rootFolder;
    item->numFolders = 0;
    while ((name = strtok_r(remaining, "/", &remaining))) {
        if (item->numFolders >= MAX_FOLDERS) return;
        Folder *folder = findOrMakeFolder(view, name, parent);
        item->folders[item->numFolders++] = folder;
        parent = folder;
    }
}

void loadInventory(Inventory *inv, FolderView* view) {
    FILE *f = fopen(FILE_PATH "items.txt", "r");
    if (!f) return;

    char line[2 * MAX_ITEM_LEN + 6];
    char name[MAX_ITEM_LEN];
    int quantity = 0;
    char desc[MAX_ITEM_LEN];
    char path[(MAX_FOLDER_LEN + 1) * MAX_FOLDERS];

    while (fgets(line, sizeof(line), f)) {
        int ret = readItem(line, name, &quantity, desc, path);
        if (ret < 4) continue;

        addInventoryItem(inv, name, desc, quantity, NULL, 0);

        Item *item = &inv->items[inv->numItems - 1];
        for (int i = 0; i < item->numTags; i++) {
        }

        loadItemFolders(&inv->items[inv->numItems - 1], view, path);
        inventoryAddFolders(inv, inv->items[inv->numItems - 1].folders, inv->items[inv->numItems - 1].numFolders);
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
    char descBuf[MAX_ITEM_LEN + (MAX_TAG_LEN + 1) * MAX_ITEM_TAGS];

    strncpy(descBuf, item->desc, MAX_ITEM_LEN - 1);
    for (int i = 0; i < item->numTags; i++) {
        strcat(descBuf, " #");
        strncat(descBuf, item->tags[i], MAX_TAG_LEN - 1);
    }

    char pathBuf[(MAX_FOLDER_LEN + 1) * MAX_FOLDERS] = "";
    for (int i = 0; i < item->numFolders; i++) {
        strncat(pathBuf, item->folders[i]->name, MAX_FOLDER_LEN - 1);
        strcat(pathBuf, "/");
    }
    fprintf(f, "%s\t%d\t%s\t%s\n", item->name, item->quantity, descBuf, pathBuf);
}

void saveInventory(const Inventory *inv) {
    makeMissingDir(FILE_PATH);

    FILE *f = fopen(FILE_PATH "items.txt", "w");
    if (!f) return;

    for (int i = 0; i < inv->numItems; i++) {
        writeItem(f, &inv->items[i]);
    }

    fclose(f);
}

// todo: error handle
int loadFolderView(FolderView *view) {
    FILE *f = fopen(FILE_PATH "folders.txt", "r");
    if (!f) return -1;

    Folder *stack[MAX_FOLDERS] = { view->rootFolder };

    char name[MAX_FOLDER_LEN];
    int depth;
    u32 color;

    if (fscanf(f, "root %lX\n", &color) == 1) {
        view->rootFolder->color = color;
    }

    while (fscanf(f, "%d\t%255[^\t\n]\t%lX\n", &depth, name, &color) == 3) {
        if (depth >= MAX_FOLDERS) {
            continue;
        }

        Folder *parent = stack[depth];
        if (!parent) {
            continue;
        }

        if (parent->numChildren < MAX_FOLDERS) {
            Folder *folder = newFolder(name, parent, &color, view->textBuf);
            if (!folder) continue;
            stack[depth + 1] = folder;
        }
    }

    fclose(f);

    return 0;
}

static void saveFolders(const Folder *folder, FILE *f, int depth) {
    if (depth > MAX_FOLDERS) return;

    fprintf(f, "%d\t%s\t%lX\n", depth, folder->name, folder->color);
    for (int i = 0; i < folder->numChildren; i++) {
        saveFolders(folder->children[i], f, depth + 1);
    }
}

void saveFolderView(const FolderView *folderView) {
    makeMissingDir(FILE_PATH);

    FILE *f = fopen(FILE_PATH "folders.txt", "w");
    if (!f) return;

    fprintf(f, "root %lX\n", folderView->rootFolder->color);
    for (int i = 0; i < folderView->rootFolder->numChildren; i++) {
        saveFolders(folderView->rootFolder->children[i], f, 0);
    }

    fclose(f);
}
