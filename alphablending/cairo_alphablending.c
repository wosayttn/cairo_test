/**************************************************************************//**
 *
 * Demo per-pixel alpha blending between video and overlay layer on screen using fbdev.
 *
 * @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ******************************************************************************/

#include <cairo/cairo.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <time.h>

#include "util.h"

static volatile sig_atomic_t cancel = 0;

typedef struct _cairo_linuxfb_device
{
    int fb_fd;
    unsigned char *fb_data;
    long fb_screensize;
    struct fb_var_screeninfo fb_vinfo;
    struct fb_fix_screeninfo fb_finfo;
} cairo_linuxfb_device_t;

void signal_handler(int signum)
{
    cancel = 1;
}

/* Destroy a cairo surface */
static void cairo_linuxfb_surface_destroy(void *dev)
{
    cairo_linuxfb_device_t *device = (cairo_linuxfb_device_t *)dev;

    if (device == NULL)
        return;

    munmap((void *)device->fb_data, device->fb_screensize);
    close(device->fb_fd);
    free(device);
}

/* Create a cairo surface using the specified framebuffer */
static cairo_surface_t *cairo_linuxfb_surface_create(cairo_linuxfb_device_t *device, const char *fb_name)
{
    cairo_surface_t *surface;

    /* Open the file for reading and writing */
    device->fb_fd = open(fb_name, O_RDWR);
    if (device->fb_fd == -1)
    {
        perror("Error: cannot open framebuffer device");
        goto handle_allocate_error;
    }

    /* Get variable screen information */
    if (ioctl(device->fb_fd, FBIOGET_VSCREENINFO, &device->fb_vinfo) == -1)
    {
        perror("Error: reading variable information");
        goto handle_ioctl_error;
    }

    /* Set virtual display size double the width for double buffering */
    device->fb_vinfo.yoffset = 0;
    device->fb_vinfo.yres_virtual = device->fb_vinfo.yres * 2;
    if (ioctl(device->fb_fd, FBIOPUT_VSCREENINFO, &device->fb_vinfo))
    {
        perror("Error setting variable screen info from fb");
        goto handle_ioctl_error;
    }

    /* Get fixed screen information */
    if (ioctl(device->fb_fd, FBIOGET_FSCREENINFO, &device->fb_finfo) == -1)
    {
        perror("Error reading fixed information");
        goto handle_ioctl_error;
    }

    device->fb_screensize = device->fb_vinfo.xres_virtual * device->fb_vinfo.yres_virtual * device->fb_vinfo.bits_per_pixel / 8;
    printf("-> device->fb_vinfo.xres_virtual=%d\n", device->fb_vinfo.xres_virtual);
    printf("-> device->fb_vinfo.yres_virtual=%d\n", device->fb_vinfo.yres_virtual);
    printf("-> device->fb_vinfo.bits_per_pixel=%d\n", device->fb_vinfo.bits_per_pixel);
    printf("-> device->fb_screensize=%d\n", device->fb_screensize);
    printf("-> device->fb_vinfo.yoffset=%d\n", device->fb_vinfo.yoffset);
    printf("-> device->fb_vinfo.yres=%d\n", device->fb_vinfo.yres);
    printf("-> device->fb_finfo.smem_len=%d\n", device->fb_finfo.smem_len);

    /* Map the device frame memory */
    device->fb_data = (unsigned char *)mmap(0, device->fb_screensize,
                                            PROT_READ | PROT_WRITE, MAP_SHARED,
                                            device->fb_fd, 0);
    if (device->fb_data == (unsigned char *) -1)
    {
        perror("Error: failed to map framebuffer device to memory");
        goto handle_ioctl_error;
    }

    /* Clean frame buffer. */
    memset((void *)device->fb_data, 0, device->fb_screensize);

    /* Create the cairo surface which will be used to draw to */
    surface = cairo_image_surface_create_for_data(device->fb_data,
              CAIRO_FORMAT_ARGB32,
              device->fb_vinfo.xres,
              device->fb_vinfo.yres_virtual,
              cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, device->fb_vinfo.xres));

    /* Register destroy callback function. */
    cairo_surface_set_user_data(surface, NULL, device, &cairo_linuxfb_surface_destroy);

    return surface;

handle_ioctl_error:
    close(device->fb_fd);

handle_allocate_error:
    free(device);
    exit(1);
}

static const char *szAlphaBlending[] =
{
    "DC_BLEND_MODE_CLEAR",
    "DC_BLEND_MODE_SRC",
    "DC_BLEND_MODE_DST",
    "DC_BLEND_MODE_SRC_OVER",
    "DC_BLEND_MODE_DST_OVER",
    "DC_BLEND_MODE_SRC_IN",
    "DC_BLEND_MODE_DST_IN",
    "DC_BLEND_MODE_SRC_OUT"
};

static void draw_alphablending(cairo_t *fbcr, cairo_linuxfb_device_t *device, int effect)
{
    int fbsizex = device->fb_vinfo.xres;
    int fbsizey = device->fb_vinfo.yres;
    cairo_surface_t *png_surface;
    dc_color_key myColKey;

    /* Disable Colorkey transparency effect. */
    myColKey.enable = 0;
    myColKey.colorkey_low = 0;
    myColKey.colorkey_high = 0;
    SetColorkey(device->fb_fd, myColKey);

    /* Load PNG resources. */
    png_surface = cairo_image_surface_create_from_png("nuvoton.png");
    printf("-> fbsizex=%d, fbsizey=%d\n", fbsizex, fbsizey);

    /* Draw surface on overlay. */
    cairo_set_operator(fbcr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_surface(fbcr, png_surface, 0, 0);
    cairo_paint(fbcr);

    while (!cancel)
    {
        dc_alpha_blending_mode eAlpha;
        for (eAlpha = DC_BLEND_MODE_CLEAR; eAlpha <= DC_BLEND_MODE_SRC_OUT; eAlpha++)
        {
            /* If the effect variable is specified, fixed the effect in this execution. */
            if (effect >= 0)
                eAlpha = effect;

            printf("%s\n", szAlphaBlending[eAlpha]);

            /* Set supporting alpha blending effect */
            SetBlendingMode(device->fb_fd, eAlpha);
            SetVarScreenInfo(device->fb_fd, &device->fb_vinfo);

            sleep(2);
            if (cancel)
                goto exit_draw_alphablending;
        }
    }

exit_draw_alphablending:

    cairo_surface_destroy(png_surface);

}

int main(int argc, char *argv[])
{
    char fb_node[16] = "/dev/fb1";
    char *tsdevice = NULL;
    struct tsdev *ts;
    struct sigaction action;
    cairo_linuxfb_device_t *device;
    cairo_surface_t *fbsurface;
    cairo_t *fbcr;
    int effect = -1;

    printf("Frame buffer node is: %s\n", fb_node);

    if (argc > 1)
    {
        effect = atoi(argv[1]);
    }

    device = malloc(sizeof(*device));
    if (!device)
    {
        perror("Error: cannot allocate memory\n");
        exit(1);
    }

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = signal_handler;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGINT, &action, NULL);

    fbsurface = cairo_linuxfb_surface_create(device, fb_node);
    fbcr = cairo_create(fbsurface);

    draw_alphablending(fbcr, device, effect);

    cairo_destroy(fbcr);
    cairo_surface_destroy(fbsurface);

    return 0;
}
