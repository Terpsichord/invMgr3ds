#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "layout.h"
#include "file.h"
#include "camera.h"

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

static void itemPath(const Item *item, char *buf) {
    buf[0] = '\0';
    for (int i = 0; i < item->numFolders; i++) {
        strncat(buf, item->folders[i]->name, MAX_FOLDER_LEN - 1);
        strcat(buf, "/");
    }
}

static bool loadImage(const Item *item, C2D_Image *img) {
    char pathBuf[(MAX_FOLDER_LEN + 1) * MAX_FOLDERS + MAX_ITEM_LEN + sizeof(IMAGE_FOLDER) + 4];
    itemPath(item, pathBuf);

    char fullPath[sizeof(FILE_PATH) + sizeof(pathBuf) + sizeof(IMAGE_FOLDER) + MAX_ITEM_LEN + 4];
    snprintf(fullPath, sizeof(fullPath), "%s%s%s%s.raw", FILE_PATH, IMAGE_FOLDER, pathBuf, item->name);

    FILE *f = fopen(fullPath, "rb");
    if (!f) return false;

    u32 *buffer = malloc(CAMERA_WIDTH * CAMERA_HEIGHT * sizeof(u32));

    fread(buffer, sizeof(u32), CAMERA_WIDTH * CAMERA_HEIGHT, f);
    *img = cameraGetImage(buffer);

    free(buffer);

    fclose(f);

    return true;
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

        // fixme: i have genuinely no idea what i was planning on doing with the line below
        for (int i = 0; i < item->numTags; i++) {
        }

        loadItemFolders(item, view, path);
        inventoryAddFolders(inv, item->folders, item->numFolders);

        C2D_Image img;
        if (loadImage(item, &img)) {
            item->hasImage = true;
            item->image = img;
        }
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

static void makeMissingNestedDir(const char *path) {
    char tmp[512];
    char *p;
    size_t len;

    strncpy(tmp, path, sizeof(tmp));
    tmp[sizeof(tmp)-1] = '\0';

    len = strlen(tmp);
    if (len == 0) return;

    if (tmp[len - 1] == '/') tmp[len - 1] = '\0';

    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            struct stat st;
            if (stat(tmp, &st) != 0) {
                if (mkdir(tmp, 0777) != 0 && errno != EEXIST) return;
            } else if (!S_ISDIR(st.st_mode)) {
                return;
            }
            *p = '/';
        }
    }

    struct stat st;
    if (stat(tmp, &st) != 0) {
        mkdir(tmp, 0777);
    }
}


void writeItem(FILE *f, const Item *item) {
    char descBuf[MAX_ITEM_LEN + (MAX_TAG_LEN + 1) * MAX_ITEM_TAGS];

    strncpy(descBuf, item->desc, MAX_ITEM_LEN - 1);
    for (int i = 0; i < item->numTags; i++) {
        strcat(descBuf, " #");
        strncat(descBuf, item->tags[i], MAX_TAG_LEN - 1);
    }

    char pathBuf[(MAX_FOLDER_LEN + 1) * MAX_FOLDERS + 1];
    itemPath(item, pathBuf);
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
    char gridCode;
    int depth;
    u32 color;

    if (fscanf(f, "root\t%lX\t%c\n", &color, &gridCode) == 2) {
        view->rootFolder->color = color;
        view->rootFolder->alwaysGrid = gridCode == 'y';
    }

    while (fscanf(f, "%d\t%255[^\t\n]\t%lX\t%c\n", &depth, name, &color, &gridCode) == 4) {
        if (depth >= MAX_FOLDERS) {
            continue;
        }

        Folder *parent = stack[depth];
        if (!parent) {
            continue;
        }

        if (parent->numChildren < MAX_FOLDERS) {
            Folder *folder = newFolder(name, parent, &color, gridCode == 'y', view->textBuf);
            if (!folder) continue;
            stack[depth + 1] = folder;
        }
    }

    fclose(f);

    return 0;
}

static char alwaysGridEncode(const Folder *folder) {
    return folder->alwaysGrid ? 'y' : 'n';
}

static void saveFolders(const Folder *folder, FILE *f, int depth) {
    if (depth > MAX_FOLDERS) return;

    fprintf(f, "%d\t%s\t%lX\t%c\n", depth, folder->name, folder->color, alwaysGridEncode(folder));
    for (int i = 0; i < folder->numChildren; i++) {
        saveFolders(folder->children[i], f, depth + 1);
    }
}

void saveFolderView(const FolderView *folderView) {
    makeMissingDir(FILE_PATH);

    FILE *f = fopen(FILE_PATH "folders.txt", "w");
    if (!f) return;

    fprintf(f, "root\t%lX\t%c\n", folderView->rootFolder->color, alwaysGridEncode(folderView->rootFolder));
    for (int i = 0; i < folderView->rootFolder->numChildren; i++) {
        saveFolders(folderView->rootFolder->children[i], f, 0);
    }

    fclose(f);
}

void saveImage(u32 *buffer, const Item *item) {
    char pathBuf[(MAX_FOLDER_LEN + 1) * MAX_FOLDERS + MAX_ITEM_LEN + sizeof(IMAGE_FOLDER) + 4];
    itemPath(item, pathBuf);

    char fullPath[sizeof(FILE_PATH) + (MAX_FOLDER_LEN + 1) * MAX_FOLDERS + sizeof(IMAGE_FOLDER) + MAX_ITEM_LEN + 4] = FILE_PATH;
    strncat(fullPath, IMAGE_FOLDER, sizeof(fullPath) - strlen(FILE_PATH) - 1);
    strncat(fullPath, pathBuf, sizeof(fullPath) - strlen(FILE_PATH) - 1);

    makeMissingNestedDir(fullPath);

    strncat(fullPath, item->name, sizeof(fullPath) - strlen(FILE_PATH) - 1);
    strncat(fullPath, ".raw", sizeof(fullPath) - strlen(FILE_PATH) - 1);

    FILE *f = fopen(fullPath, "wb");
    if (!f) return;

    fwrite(buffer, sizeof(u32), CAMERA_WIDTH * CAMERA_HEIGHT, f);

    fclose(f);
}