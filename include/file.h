#pragma once

#include "inventory.h"
#include "folder.h"

#define FILE_PATH "sdmc:/3ds/invMgr3ds/"
#define IMAGE_FOLDER "images/"

void loadInventory(Inventory *inv, FolderView *view);
void makeMissingDir(const char *path);
void saveInventory(const Inventory *inv);

int loadFolderView(FolderView *view);
void saveFolderView(const FolderView *folder);

void saveImage(u32 *buffer, const Item *item);
