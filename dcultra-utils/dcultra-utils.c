/**************************************************************************//**
 *
 * dcultra-utils.
 *
 * @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ******************************************************************************/

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
#include <stdint.h>
#include <getopt.h>

#include "util.h"

static int g_i32FBdevfd = -1;
static struct fb_fix_screeninfo g_sFBFixInfo = {0};
static struct fb_var_screeninfo g_sFBVarInfo = {0};

static void help(void)
{
    fprintf(stdout,
            "Usage: dcultra-utils [OPTION]\n"
            "-h,--help           Help\n"
            "-d,--device         Frame buffer device node(/dev/fb0, /dev/fb1)\n"
            "-s,--SetPanDisplay  View window ID - 0, 1, 2\n"
            "-c,--SetColor       Fill Color before pan display\n"
            "\n");
    fflush(stdout);
}

/*
 * Flip framebuffer, return the next buffer id which will be used
 */
int flip_buffer(int vsync, int bufid)
{
    int dummy = 0;

    /* Pan the framebuffer */
    g_sFBVarInfo.yoffset = g_sFBVarInfo.yres * bufid;

    printf("fb_offset=%d, yres=%d, vsync=%d\n", g_sFBVarInfo.yoffset, g_sFBVarInfo.yres, vsync);

    if (ioctl(g_i32FBdevfd, FBIOPAN_DISPLAY, &g_sFBVarInfo))
    {
        perror("Error panning display");
        return -1;
    }

    if (vsync)
    {
        if (ioctl(g_i32FBdevfd, FBIO_WAITFORVSYNC, &dummy))
        {
            perror("Error waiting for VSYNC");
            return -1;
        }
    }

    return 0;
}

#define FB_DEVICE "/dev/fb0"
int main(int argc, char *argv[])
{
    char fb_node[16] = {0};
    uint32_t u32ViewWinIdx = 0, u32FillColor = 0;
    int c;

    struct option long_option[] =
    {
        {"help", 0, NULL, 'h'},
        {"device", 1, NULL, 'd'},
        {"SetPanDisplay", 1, NULL, 's'},
        {"SetColor", 1, NULL, 'c'},
        {NULL, 0, NULL, 0},
    };

    // Parsing
    while ((c = getopt_long(argc, argv, "d:s:c:h", long_option, NULL)) >= 0)
    {
        switch (c)
        {
        case 'd':
            if (optarg)
                strcpy(fb_node, optarg);
            else
            {
                help();
                return 0;
            }
            break;

        case 's':
            if (optarg)
                sscanf(optarg, "%d", &u32ViewWinIdx);
            else
            {
                help();
                return -1;
            }
            break;

        case 'c':
            if (optarg)
                sscanf(optarg, "%x", &u32FillColor);
            else
            {
                help();
                return -1;
            }
            break;

        case 'h':
        case '?':
            help();
            return 0;
        default:
            break;
        }
    }

    if ( (g_i32FBdevfd = open(fb_node, O_RDWR)) == -1)
    {
        perror("open");
    }
    else if ( (GetFixScreenInfo(g_i32FBdevfd, &g_sFBFixInfo)) < 0 )
    {
        perror("GetFixScreenInfo");
    }
    else if ( (GetVarScreenInfo(g_i32FBdevfd, &g_sFBVarInfo)) < 0 )
    {
        perror("GetVarScreenInfo");
    }
    else
    {
        int screensize;
        dump_fscreeninfo(&g_sFBFixInfo);
        dump_vscreeninfo(&g_sFBVarInfo);

        screensize = g_sFBVarInfo.xres * g_sFBVarInfo.yres * g_sFBVarInfo.bits_per_pixel / 8;
        if ( u32ViewWinIdx < (g_sFBFixInfo.smem_len/screensize) )
        {
            // Map the device to memory
            unsigned char *usermem = (unsigned char *)mmap(0, g_sFBFixInfo.smem_len,
                                                  PROT_READ | PROT_WRITE, MAP_SHARED,
                                                  g_i32FBdevfd, 0);
            if (usermem == (unsigned char *) -1)
            {
                perror("Error: failed to map framebuffer device to memory");
            }
            else
            {
                /* Pan the framebuffer */
                g_sFBVarInfo.yoffset = g_sFBVarInfo.yres * u32ViewWinIdx;

                printf("u32FillColor = %08x\n", u32FillColor);
                CleanScreen(u32FillColor, &g_sFBVarInfo, usermem, screensize);

                flip_buffer(1, u32ViewWinIdx);
                munmap((void *)usermem, g_sFBFixInfo.smem_len);
            }

        }
    }

    if ( g_i32FBdevfd >= 0 )
        close(g_i32FBdevfd);

    return 0;
}
