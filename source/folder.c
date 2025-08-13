#include <stdlib.h>

#include "folder.h"
#include "inventory.h"

u32 folderColors[];

static Folder *newRoot(void) {
    Folder *folder = calloc(1, sizeof(Folder));
    if (!folder) return NULL;
    folder->color = C2D_Color32(0xFF, 0xFF, 0xFF, 0xFF);
    return folder;
}

void initFolderView(FolderView *view) {
    view->rootFolder = newRoot();
    view->currentFolder = view->rootFolder;
    view->selectedIdx = 0;
    view->textBuf = C2D_TextBufNew(FOLDER_BUF_SIZE);
}

void freeFolderView(FolderView *view) {
    freeFolder(view->rootFolder);
    C2D_TextBufDelete(view->textBuf);
}

Folder *newFolder(const char *name, Folder *parent, const u32 *color, C2D_TextBuf textBuf) {
    if (parent->numChildren >= MAX_FOLDERS) return NULL;

    Folder *folder = calloc(1, sizeof(Folder));
    if (!folder) return NULL;

    strncpy(folder->name, name, MAX_FOLDER_LEN - 1);
    C2D_TextParse(&folder->text, textBuf, name);
    C2D_TextOptimize(&folder->text);

    folder->color = color != NULL ? *color : folderColors[rand() % NUM_FOLDER_COLORS];
    folder->parent = parent;
    parent->children[parent->numChildren++] = folder;

    return folder;
}

void freeFolder(Folder *folder) {
    for (int i = 0; i < folder->numChildren; i++) {
        freeFolder(folder->children[i]);
    }

    free(folder);
}

Folder *getSelectedFolder(const FolderView *folderView) {
    return folderView->currentFolder->children[folderView->selectedIdx];
}

bool isFolderEmpty(const FolderView *folderView) {
    return folderView->currentFolder->numChildren == 0;
}

bool isEmptyRoot(const FolderView *folderView) {
    return isFolderEmpty(folderView) && folderView->currentFolder == folderView->rootFolder;
}

static void updateFolderText(Folder *folder, C2D_TextBuf textBuf) {
    C2D_TextParse(&folder->text, textBuf, folder->name);
    C2D_TextOptimize(&folder->text);

    for (int i = 0; i < folder->numChildren; i++) {
        updateFolderText(folder->children[i], textBuf);
    }
}

void updateFolderViewText(FolderView *view) {
    C2D_TextBufClear(view->textBuf);
    updateFolderText(view->rootFolder, view->textBuf);
}

int cmpFolders(const void *a, const void *b) {
    return strcasecmp((*(Folder **)a)->name, (*(Folder **)b)->name);
}

void sortFolders(Folder *folder) {
    qsort(folder->children, folder->numChildren, sizeof(Folder *), cmpFolders);
}

Folder *findFolder(Folder *root, const char *name) {
    if (root == NULL) return NULL;
    if (strcmp(root->name, name) == 0) return root;

    for (int i = 0; i < root->numChildren; i++) {
        Folder *folder = findFolder(root->children[i], name);
        if (folder != NULL) return folder;
    }
    return NULL;
}

Folder *findOrMakeFolder(FolderView *view, const char *name, Folder *parent) {
    Folder *folder = findFolder(view->rootFolder, name);
    if (folder == NULL) {
        folder = newFolder(name, parent, NULL, view->textBuf);
        sortFolders(parent);
    }
    return folder;

}

void folderViewNavigateChild(FolderView *view) {
    if (view->selectedIdx < view->currentFolder->numChildren) {
        view->currentFolder = view->currentFolder->children[view->selectedIdx];
        view->selectedIdx = 0;
    }
}

void folderViewNavigateParent(FolderView *view) {
    if (view->currentFolder->parent != NULL) {
        for (int i = 0; i < view->currentFolder->parent->numChildren; i++) {
            if (view->currentFolder->parent->children[i] == view->currentFolder) {
                view->selectedIdx = i;
                break;
            }
        }
        view->currentFolder = view->currentFolder->parent;
    }
}
