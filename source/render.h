#include "ui.h"

extern C2D_Text qtyText, tagsText, outText, sortText, filterText, searchText, commaText, quotesText, sortTexts[NUM_SORTS];
extern u32 white, lightGray, gray, darkGray, black, columnGray;

void initRender(C3D_RenderTarget **top, C3D_RenderTarget **bottom);
void cleanupRender(void);

bool showFilterBar(const Inventory *inv, Screen screen);

void
render(C3D_RenderTarget *top, C3D_RenderTarget *bottom, const Inventory *inv, Screen screen, DisplayMode display, bool optionScreen, Scroll listScroll,
       Scroll gridScroll, const TouchState *touchState, const ButtonPresses *presses, Scroll filterScroll, const FolderView *folderView);

u32 hsvToRgb(float h, float s, float v);
void rgbToHsv(u32 color, float *h, float *s, float *v);
