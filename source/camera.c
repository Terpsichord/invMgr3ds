#include <string.h>
#include <stdlib.h>

#include "citro3d.h"

#include "camera.h"

typedef enum {
    EVENT_INTERRUPT,
    EVENT_RECV,
} CameraEvent;

static u8 *cameraBuffer = NULL;
static Handle receiveEvents[2] = {0};
static bool captureInterrupted = false;
static u32 bufSize = 0;

static C3D_Tex *cameraTexture = NULL;
static Tex3DS_SubTexture *camSubTex = NULL;

static void initCameraTexture(void) {
    cameraTexture = malloc(sizeof(C3D_Tex));

    if (!C3D_TexInit(cameraTexture, CAMERA_WIDTH, CAMERA_HEIGHT, GPU_RGB565)) {
        free(cameraTexture);
    }

    Tex3DS_SubTexture* subtex = (Tex3DS_SubTexture*)malloc(sizeof(Tex3DS_SubTexture));
    subtex->left   = 0.0f;
    subtex->top    = 0.0f;
    subtex->right  = (float)CAMERA_WIDTH;
    subtex->bottom = (float)CAMERA_HEIGHT;
    subtex->width  = CAMERA_WIDTH;
    subtex->height = CAMERA_HEIGHT;
}

static void updateCameraTexture(void) {
    GSPGPU_FlushDataCache((u32*)cameraBuffer, CAMERA_BUFFER_SIZE);
    C3D_SyncDisplayTransfer((u32*)cameraBuffer, GX_BUFFER_DIM(CAMERA_WIDTH, CAMERA_HEIGHT),
                            (u32*)cameraTexture->data, GX_BUFFER_DIM(CAMERA_WIDTH, CAMERA_HEIGHT),
                            GX_TRANSFER_FLIP_VERT(0) |
                            GX_TRANSFER_OUT_TILED(1) |
                            GX_TRANSFER_RAW_COPY(0) |
                            GX_TRANSFER_IN_FORMAT(GX_TRANSFER_FMT_RGB565) |
                            GX_TRANSFER_OUT_FORMAT(GX_TRANSFER_FMT_RGB565));
    C3D_TexSetFilter(cameraTexture, GPU_LINEAR, GPU_LINEAR);
    C3D_TextSetWrap(cameraTexture, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
}

void initCamera(void) {
    camInit();

    CAMU_SetSize(SELECT_OUT1, SIZE_CTR_TOP_LCD, CONTEXT_A);
    CAMU_SetOutputFormat(SELECT_OUT1, OUTPUT_RGB_565, CONTEXT_A);

    CAMU_SetFrameRate(SELECT_OUT1, FRAME_RATE_30);

    CAMU_SetNoiseFilter(SELECT_OUT1, true);
    CAMU_SetAutoExposure(SELECT_OUT1, true);
    CAMU_SetAutoWhiteBalance(SELECT_OUT1, true);

    CAMU_SetTrimming(PORT_CAM1, true);
    CAMU_SetTrimmingParamsCenter(PORT_CAM1, CAMERA_WIDTH, CAMERA_HEIGHT, TOP_WIDTH, SCREEN_HEIGHT);

    initCameraTexture();
}

void cleanupCamera(void) {
    camExit();
}

void cameraStartStream(void) {
    cameraBuffer = calloc(1, CAMERA_BUFFER_SIZE);

    captureInterrupted = false;

    CAMU_Activate(SELECT_OUT1);

    CAMU_GetBufferErrorInterruptEvent(&receiveEvents[EVENT_INTERRUPT], PORT_CAM1);
    CAMU_GetMaxBytes(&bufSize, CAMERA_WIDTH, CAMERA_HEIGHT);
    CAMU_SetTransferBytes(PORT_CAM1, bufSize, CAMERA_WIDTH, CAMERA_HEIGHT);
    CAMU_ClearBuffer(PORT_CAM1);
    CAMU_StartCapture(PORT_CAM1);
}

void cameraStopStream(void) {
    free(cameraBuffer);
    CAMU_StopCapture(PORT_CAM1);
    CAMU_Activate(SELECT_NONE);
}

// returns whether the screen should be updated
bool cameraUpdate(void) {
    if (receiveEvents[EVENT_RECV] == 0) {
        CAMU_SetReceiving(&receiveEvents[EVENT_RECV], cameraBuffer, PORT_CAM1, CAMERA_BUFFER_SIZE, (s16) bufSize);
    }

    if (captureInterrupted) {
        CAMU_StartCapture(PORT_CAM1);
        captureInterrupted = false;
    }

    s32 index = 0;
    svcWaitSynchronizationN(&index, receiveEvents, 2, false, CAMERA_WAIT_TIMEOUT);

    svcCloseHandle(receiveEvents[EVENT_RECV]);
    receiveEvents[EVENT_RECV] = 0;

    updateCameraTexture();

    if (index == EVENT_INTERRUPT) {
        captureInterrupted = true;
        return false;
    }

    return true;
}

//static void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
//    u8 *fb_8 = (u8 *) fb;
//    u16 *img_16 = (u16 *) img;
//    int i, j, draw_x, draw_y;
//    for (j = 0; j < height; j++) {
//        for (i = 0; i < width; i++) {
//            draw_y = y + height - j;
//            draw_x = x + i;
//            u32 v = (draw_y + draw_x * height) * 3;
//            u16 data = img_16[j * width + i];
//            uint8_t b = ((data >> 11) & 0x1F) << 3;
//            uint8_t g = ((data >> 5) & 0x3F) << 2;
//            uint8_t r = (data & 0x1F) << 3;
//            fb_8[v] = r;
//            fb_8[v + 1] = g;
//            fb_8[v + 2] = b;
//        }
//    }
//}

C2D_Image cameraImage() {
    return (C2D_Image) {
        .tex = cameraTexture,
        .subtex = camSubTex,
    };
}