#pragma once

#include "citro2d.h"

#define MAX_FOLDERS 16
#define MAX_TOTAL_FOLDERS 32
#define MAX_FOLDER_LEN 256
#define FOLDER_BUF_SIZE 1024

#define NUM_FOLDER_COLORS 20

extern u32 folderColors[NUM_FOLDER_COLORS];

typedef struct Folder {
    char name[MAX_FOLDER_LEN];
    C2D_Text text;
    u32 color;

    struct Folder *parent;
    struct Folder *children[MAX_FOLDERS];
    int numChildren;
} Folder;

typedef struct {
    Folder *rootFolder;
    Folder *currentFolder;
    int selectedIdx;
    C2D_TextBuf textBuf;
} FolderView;

Folder *newFolder(const char *name, Folder *parent, const u32 *color, C2D_TextBuf textBuf);
void freeFolder(Folder *folder);

void initFolderView(FolderView *view);
void updateFolderViewText(FolderView *view);
void sortFolders(Folder *folder);
void freeFolderView(FolderView *view);

Folder *findFolder(Folder *root, const char *name);
Folder *findOrMakeFolder(FolderView *view, const char *name, Folder *parent);

Folder *getSelectedFolder(const FolderView *folderView);
bool isFolderEmpty(const FolderView *folderView);
bool isEmptyRoot(const FolderView *folderView);

void folderViewNavigateChild(FolderView *view);
void folderViewNavigateParent(FolderView *view);
