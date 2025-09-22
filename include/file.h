#pragma once

#include "inventory.h"
#include "folder.h"

#define FILE_PATH "sdmc:/3ds/invMgr3ds/"

void loadInventory(Inventory *inv, FolderView *view);
void makeMissingDir(const char *path);
void saveInventory(const Inventory *inv);

int loadFolderView(FolderView *view);
void saveFolderView(const FolderView *folder);