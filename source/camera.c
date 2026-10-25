#include <string.h>
#include <stdlib.h>

#include "citro3d.h"

#include "camera.h"
#include "file.h"
#include "render.h"

typedef enum {
    EVENT_INTERRUPT,
    EVENT_RECV,
} CameraEvent;

static u8 *cameraBuffer = NULL;
static Handle receiveEvents[2] = {0};
static bool captureInterrupted = false;
static u32 bufSize = 0;

static float cameraBrightness = 10.0f;

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

    if (index == EVENT_INTERRUPT) {
        captureInterrupted = true;
        return false;
    }

    return true;
}

u32 adjustBrightness(u32 rgb) {
    float h, s, v;
    rgbToHsv(rgb, &h, &s, &v);

    v = fmaxf(1.0f, v * cameraBrightness);

    return hsvToRgb(h, s, v);
}


static void writePictureToFramebufferRGB565(void *fb, void *img, u16 x, u16 y, u16 width, u16 height) {
    u8 *fb_8 = (u8 *) fb;
    u16 *img_16 = (u16 *) img;
    int i, j, draw_x, draw_y;
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            draw_y = y + height - j;
            draw_x = x + i;
            u32 v = (draw_y + draw_x * height) * 3;
            u16 data = img_16[j * width + i];

            uint8_t b = ((data >> 11) & 0x1F) << 3;
            uint8_t g = ((data >> 5) & 0x3F) << 2;
            uint8_t r = (data & 0x1F) << 3;

            u32 rgb = C2D_Color32(r, g, b, 0xFF);
            u32 brightRgb = adjustBrightness(rgb);

            r = (brightRgb) & 0xFF;
            g = (brightRgb >> 8) & 0xFF;
            b = (brightRgb >> 16) & 0xFF;

            fb_8[v] = r;
            fb_8[v + 1] = g;
            fb_8[v + 2] = b;
        }
    }
}

void cameraDrawBuffer(void *fb) {
    writePictureToFramebufferRGB565(fb, cameraBuffer, CAMERA_X, CAMERA_Y, CAMERA_WIDTH, CAMERA_HEIGHT);
}

const u32 next_pow2(u32 n) {
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

const u32 clamp(u32 n, u32 lower, u32 upper) {
    if (n < lower)
        return lower;
    if (n > upper)
        return upper;
    return n;
}

const u32 rgba_to_abgr(u32 px) {
    u8 r = (px & 0xff000000) >> 24;
    u8 g = (px & 0x00ff0000) >> 16;
    u8 b = (px & 0x0000ff00) >> 8;
    u8 a = px & 0x000000ff;
    return (a << 24) | (b << 16) | (g << 8) | r;
}

void rgb565_to_rgba8888(uint8_t *src, uint8_t *dst, size_t width, size_t height) {
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            uint16_t rgb565 = 0;
            // little endian read
            rgb565 |= (uint16_t)(*(src + (y * width + x) * 2));
            rgb565 |= (uint16_t)(*(src + (y * width + x) * 2 + 1)) << 8;

            uint8_t r = (rgb565 >> 11) & 0x1F;
            uint8_t g = (rgb565 >> 5) & 0x3F;
            uint8_t b = rgb565 & 0x1F;

            size_t dst_index = (y * width + x) * 4;
            dst[dst_index]     = r << 3;  // red
            dst[dst_index + 1] = g << 2;  // green
            dst[dst_index + 2] = b << 3;  // blue
            dst[dst_index + 3] = 255;     // alpha
        }
    }
}

void cameraSaveImage(Item *item) {
    u32 *rgba_buf = (u32 *) malloc(CAMERA_WIDTH * CAMERA_HEIGHT * sizeof(u32));
    rgb565_to_rgba8888(cameraBuffer, (u8 *) rgba_buf, CAMERA_WIDTH, CAMERA_HEIGHT);

    saveImage(rgba_buf, item);

    item->image = cameraGetImage(rgba_buf);
    item->hasImage = true;
}

C2D_Image cameraGetImage(u32 *rgba_raw) {
    C2D_Image image;
    const u32 image_width = CAMERA_WIDTH, image_height = CAMERA_HEIGHT;

    // Base texture
    C3D_Tex *tex = malloc(sizeof(C3D_Tex));
    image.tex = tex;
    // Texture dimensions must be square powers of two between 64x64 and 1024x1024
    tex->width = clamp(next_pow2(image_width), 64, 1024);
    tex->height = clamp(next_pow2(image_height), 64, 1024);

    // Subtexture
    Tex3DS_SubTexture *subtex = malloc(sizeof(Tex3DS_SubTexture));
    image.subtex = subtex;
    subtex->width = image_width;
    subtex->height = image_height;
    // (U, V) coordinates
    subtex->left = 0.0f;
    subtex->top = 1.0f;
    subtex->right = (float)image_width / (float)tex->width;
    subtex->bottom = 1.0 - ((float)image_height / (float)tex->height);

    C3D_TexInit(tex, tex->width, tex->height, GPU_RGBA8);
    C3D_TexSetFilter(tex, GPU_LINEAR, GPU_NEAREST);

    memset(tex->data, 0, image_width * image_height * 4);
    for (u8 i = 0; i < image_height; i++) {
        for (u8 j = 0; j < image_width; j++) {
            u32 src_idx = (j * image_width) + i;
            u32 rgba_px = rgba_raw[src_idx];
            u32 abgr_px = rgba_to_abgr(rgba_px);

            // Swizzle magic to convert into a t3x format
            u32 dst_ptr_offset = ((((j >> 3) * (tex->width >> 3) + (i >> 3)) << 6) +
                                  ((i & 1) | ((j & 1) << 1) | ((i & 2) << 1) |
                                   ((j & 2) << 2) | ((i & 4) << 2) | ((j & 4) << 3)));
            ((u32 *)tex->data)[dst_ptr_offset] = abgr_px;
        }
    }

    return image;
}