#include "3ds.h"
#include "citro2d.h"

#include "main.h"
#include "inventory.h"

#define CAMERA_WIDTH    240
#define CAMERA_HEIGHT   240
#define CAMERA_X        (TOP_WIDTH - CAMERA_WIDTH) / 2
#define CAMERA_Y        (SCREEN_HEIGHT - CAMERA_HEIGHT) / 2
#define CAMERA_BUFFER_SIZE ((CAMERA_WIDTH) * (CAMERA_HEIGHT) * sizeof(u16))

#define CAMERA_WAIT_TIMEOUT        1000000000ULL

void initCamera(void);
void cleanupCamera(void);

void cameraStartStream(void);
void cameraStopStream(void);

bool cameraUpdate(void);
void cameraDrawBuffer(void *fb);

void cameraSaveImage(Item *item);
C2D_Image cameraGetImage(u32 *rgba_raw);
