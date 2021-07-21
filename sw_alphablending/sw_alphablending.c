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
#include <getopt.h>

typedef struct _cairo_linuxfb_device
{
    int fb_fd;
    unsigned char *fb_data;
    long fb_screensize;
    struct fb_var_screeninfo fb_vinfo;
    struct fb_fix_screeninfo fb_finfo;
} cairo_linuxfb_device_t;


/* Destroy a cairo surface */
void cairo_linuxfb_surface_destroy(void *dev)
{
    cairo_linuxfb_device_t *device = (cairo_linuxfb_device_t *)dev;

    if (device == NULL)
        return;

    munmap((void *)device->fb_data, device->fb_screensize);
    close(device->fb_fd);
    free(device);
}

/* Create a cairo surface using the specified framebuffer */
cairo_surface_t *cairo_linuxfb_surface_create(cairo_linuxfb_device_t *device, const char *fb_name)
{
    cairo_surface_t *surface;

    // Open the file for reading and writing
    device->fb_fd = open(fb_name, O_RDWR);
    if (device->fb_fd == -1)
    {
        perror("Error: cannot open framebuffer device");
        goto handle_allocate_error;
    }

    // Get variable screen information
    if (ioctl(device->fb_fd, FBIOGET_VSCREENINFO, &device->fb_vinfo) == -1)
    {
        perror("Error: reading variable information");
        goto handle_ioctl_error;
    }

    /* Set virtual display size double the width for double buffering */
    device->fb_vinfo.yoffset = 0;
    device->fb_vinfo.yres_virtual = device->fb_vinfo.yres * 3;
    if (ioctl(device->fb_fd, FBIOPUT_VSCREENINFO, &device->fb_vinfo))
    {
        perror("Error setting variable screen info from fb");
        goto handle_ioctl_error;
    }

    // Get fixed screen information
    if (ioctl(device->fb_fd, FBIOGET_FSCREENINFO, &device->fb_finfo) == -1)
    {
        perror("Error reading fixed information");
        goto handle_ioctl_error;
    }

    device->fb_screensize = device->fb_vinfo.xres_virtual * device->fb_vinfo.yres_virtual * device->fb_vinfo.bits_per_pixel / 8;

    // Map the device to memory
    device->fb_data = (unsigned char *)mmap(0, device->fb_screensize,
                                            PROT_READ | PROT_WRITE, MAP_SHARED,
                                            device->fb_fd, 0);
    if (device->fb_data == (unsigned char *) -1)
    {
        perror("Error: failed to map framebuffer device to memory");
        goto handle_ioctl_error;
    }

    /* Create the cairo surface which will be used to draw to */
    /*
        cairo_surface_t *cairo_image_surface_create_for_data(unsigned char *data,
            cairo_format_t format, int width, int height, int stride);

    cairo_format_stride_for_width(cairo_format_t format, int width);
    */
    surface = cairo_image_surface_create_for_data(device->fb_data,
              CAIRO_FORMAT_ARGB32,
              device->fb_vinfo.xres,
              device->fb_vinfo.yres_virtual,
              cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, device->fb_vinfo.xres));

    cairo_surface_set_user_data(surface, NULL, device, &cairo_linuxfb_surface_destroy);

    return surface;

handle_ioctl_error:
    close(device->fb_fd);
handle_allocate_error:
    free(device);
    exit(1);
}

static cairo_operator_t operators[] =
{
  CAIRO_OPERATOR_CLEAR,
  CAIRO_OPERATOR_SOURCE,
  CAIRO_OPERATOR_OVER,
  CAIRO_OPERATOR_IN,
  CAIRO_OPERATOR_OUT,
  CAIRO_OPERATOR_ATOP,
  CAIRO_OPERATOR_DEST,
  CAIRO_OPERATOR_DEST_OVER,
  CAIRO_OPERATOR_DEST_IN,
  CAIRO_OPERATOR_DEST_OUT,
  CAIRO_OPERATOR_DEST_ATOP,
  CAIRO_OPERATOR_XOR,
  CAIRO_OPERATOR_ADD,
  CAIRO_OPERATOR_SATURATE,
  CAIRO_OPERATOR_MULTIPLY
};
#define operator_num (sizeof(operators)/sizeof(cairo_operator_t))

static char* szoperators[] =
{
  "CAIRO_OPERATOR_CLEAR.png",
  "CAIRO_OPERATOR_SOURCE.png",
  "CAIRO_OPERATOR_OVER.png",
  "CAIRO_OPERATOR_IN.png",
  "CAIRO_OPERATOR_OUT.png",
  "CAIRO_OPERATOR_ATOP.png",
  "CAIRO_OPERATOR_DEST.png",
  "CAIRO_OPERATOR_DEST_OVER.png",
  "CAIRO_OPERATOR_DEST_IN.png",
  "CAIRO_OPERATOR_DEST_OUT.png",
  "CAIRO_OPERATOR_DEST_ATOP.png",
  "CAIRO_OPERATOR_XOR.png",
  "CAIRO_OPERATOR_ADD.png",
  "CAIRO_OPERATOR_SATURATE.png",
  "CAIRO_OPERATOR_MULTIPLY.png"
};

int main(int argc, char *argv[])
{
    int i;
    char fb_devnode_video[16] = "/dev/fb0";
    char fb_devnode_overlay[16] = "/dev/fb1";
    cairo_linuxfb_device_t *device_video, *device_overlay;
    cairo_surface_t *surface_video, *surface_overlay;

    device_video = malloc(sizeof(*device_video));

    if (!device_video)
    {
        perror("Error: cannot allocate memory\n");
        exit(1);
    }

    device_overlay = malloc(sizeof(*device_overlay));
    if (!device_overlay)
    {
        perror("Error: cannot allocate memory\n");
        exit(1);
    }

    surface_video = cairo_linuxfb_surface_create(device_video, fb_devnode_video);

    surface_overlay = cairo_linuxfb_surface_create(device_overlay, fb_devnode_overlay);

    for ( i=0; i<operator_num; i++)
    {
        cairo_surface_t * surface_tmp = cairo_image_surface_create (cairo_image_surface_get_format(surface_video),
                                                            cairo_image_surface_get_width(surface_video),
                                                            cairo_image_surface_get_height(surface_video) );
        cairo_t *fbcr_tmp =  cairo_create(surface_tmp);

        cairo_set_operator(fbcr_tmp, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_surface(fbcr_tmp, surface_video, 0, 0);
        cairo_paint(fbcr_tmp);

        cairo_set_operator(fbcr_tmp, operators[i]);
        cairo_set_source_surface(fbcr_tmp, surface_overlay, 0, 0);
        cairo_paint(fbcr_tmp);
        cairo_surface_write_to_png(surface_tmp, szoperators[i]);

        cairo_destroy(fbcr_tmp);

        cairo_surface_destroy(surface_tmp);
    }

    cairo_surface_write_to_png(surface_video, "video.png");

    cairo_surface_write_to_png(surface_overlay, "overlay.png");

    cairo_surface_destroy(surface_video);
    cairo_surface_destroy(surface_overlay);

    return 0;
}
