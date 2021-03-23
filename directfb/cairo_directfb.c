/* testimage.c
 * Copyright (C) 2005  Red Hat, Inc.
 * Based on cairo-demo/X11/cairo-knockout.c
 *
 * Author: Owen Taylor
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * compile with
 * gcc `pkg-config --cflags directfb` `pkg-config --cflags cairo` `pkg-config --libs directfb` `pkg-config --libs cairo` -o testcairo testcairo.c
 * Run example
 * export DFBARGS=system=sdl;./testcairo
 */

#include <stdio.h>
#include <math.h>
#include <cairo.h>
#include <directfb.h>
#include <cairo-directfb.h>

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
{                                                                \
    err = x;                                                    \
    if (err != DFB_OK) {                                        \
        fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
        DirectFBErrorFatal( #x, err );                         \
    }                                                           \
}

#define G_PI    3.1415926535897932384626433832795028841971693993751
#define G_PI_2  1.5707963267948966192313216916397514420985846996876
#define G_PI_4  0.78539816339744830961566084581987572104929234984378

#define where_am_i() fprintf(stderr,"%s # %i \n",__func__,__LINE__)


static void
oval_path(cairo_t *cr,
          double xc, double yc,
          double xr, double yr)
{
    cairo_save(cr);

    cairo_translate(cr, xc, yc);
    cairo_scale(cr, 1.0, yr / xr);
    cairo_move_to(cr, xr, 0.0);
    cairo_arc(cr,
              0, 0,
              xr,
              0, 2 * G_PI);
    cairo_close_path(cr);

    cairo_restore(cr);
}

/* Create a path that is a circular oval with radii xr, yr at xc,
 * yc.
 */
/* Fill the given area with checks in the standard style
 * for showing compositing effects.
 *
 * It would make sense to do this as a repeating surface,
 * but most implementations of RENDER currently have broken
 * implementations of repeat + transform, even when the
 * transform is a translation.
 */
static void
fill_checks(cairo_t *cr,
            int x,     int y,
            int width, int height)
{
    int i, j;

#define CHECK_SIZE 32

    cairo_rectangle(cr, x, y, width, height);
    cairo_set_source_rgb(cr, 0.4, 0.4, 0.4);
    cairo_fill(cr);

    /* Only works for CHECK_SIZE a power of 2 */
    j = x & (-CHECK_SIZE);

    for (; j < height; j += CHECK_SIZE)
    {
        i = y & (-CHECK_SIZE);
        for (; i < width; i += CHECK_SIZE)
            if ((i / CHECK_SIZE + j / CHECK_SIZE) % 2 == 0)
                cairo_rectangle(cr, i, j, CHECK_SIZE, CHECK_SIZE);
    }

    cairo_set_source_rgb(cr, 0.7, 0.7, 0.7);
    cairo_fill(cr);
}

/* Draw a red, green, and blue circle equally spaced inside
 * the larger circle of radius r at (xc, yc)
 */
static void
draw_3circles(cairo_t *cr,
              double xc, double yc,
              double radius,
              double alpha)
{
    double subradius = radius * (2 / 3. - 0.1);

    cairo_set_source_rgba(cr, 1., 0., 0., alpha);
    oval_path(cr,
              xc + radius / 3. * cos(G_PI * (0.5)),
              yc - radius / 3. * sin(G_PI * (0.5)),
              subradius, subradius);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0., 1., 0., alpha);
    oval_path(cr,
              xc + radius / 3. * cos(G_PI * (0.5 + 2 / .3)),
              yc - radius / 3. * sin(G_PI * (0.5 + 2 / .3)),
              subradius, subradius);
    cairo_fill(cr);

    cairo_set_source_rgba(cr, 0., 0., 1., alpha);
    oval_path(cr,
              xc + radius / 3. * cos(G_PI * (0.5 + 4 / .3)),
              yc - radius / 3. * sin(G_PI * (0.5 + 4 / .3)),
              subradius, subradius);
    cairo_fill(cr);
}

static void
draw(cairo_t *cr,
     int      width,
     int      height,
     cairo_surface_t *sur,
     IDirectFBSurface *surface)
{
    cairo_surface_t *overlay, *punch, *circles;
    cairo_t *overlay_cr, *punch_cr, *circles_cr;

    /* Fill the background */
    double radius = 0.5 * (width < height ? width : height) - 10;
    double xc = width / 2.;
    double yc = height / 2.;

    overlay = cairo_surface_create_similar(cairo_get_target(cr),
                                           CAIRO_CONTENT_COLOR_ALPHA,
                                           width, height);
    if (overlay == NULL)
        return;

    punch = cairo_surface_create_similar(cairo_get_target(cr),
                                         CAIRO_CONTENT_ALPHA,
                                         width, height);
    if (punch == NULL)
        return;

    circles = cairo_surface_create_similar(cairo_get_target(cr),
                                           CAIRO_CONTENT_COLOR_ALPHA,
                                           width, height);
    if (circles == NULL)
        return;

    fill_checks(cr, 0, 0, width, height);
    /* Draw a black circle on the overlay
     */
    overlay_cr = cairo_create(overlay);
    cairo_set_source_rgb(overlay_cr, 0., 0., 0.);
    oval_path(overlay_cr, xc, yc, radius, radius);
    cairo_fill(overlay_cr);

    /* Draw 3 circles to the punch surface, then cut
     * that out of the main circle in the overlay
     */
    punch_cr = cairo_create(punch);
    draw_3circles(punch_cr, xc, yc, radius, 1.0);
    cairo_destroy(punch_cr);

    cairo_set_operator(overlay_cr, CAIRO_OPERATOR_DEST_OUT);
    cairo_set_source_surface(overlay_cr, punch, 0, 0);
    cairo_paint(overlay_cr);

    /* Now draw the 3 circles in a subgroup again
     * at half intensity, and use OperatorAdd to join up
     * without seams.
     */
    circles_cr = cairo_create(circles);
    cairo_set_operator(circles_cr, CAIRO_OPERATOR_OVER);
    draw_3circles(circles_cr, xc, yc, radius, 0.5);
    cairo_destroy(circles_cr);

    cairo_set_operator(overlay_cr, CAIRO_OPERATOR_ADD);
    cairo_set_source_surface(overlay_cr, circles, 0, 0);
    cairo_paint(overlay_cr);

    cairo_destroy(overlay_cr);
    cairo_set_source_surface(cr, overlay, 0, 0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_rectangle(cr, 0, 0, 20, 20);
    cairo_fill(cr);
    cairo_surface_destroy(overlay);
    cairo_surface_destroy(punch);
    cairo_surface_destroy(circles);
    cairo_set_source_rgb(cr, 255, 0, 0);
    cairo_move_to(cr, 50, 50);
    cairo_set_font_size(cr, 20);
    cairo_show_text(cr, "Hello World.");
    cairo_rectangle(cr, 0, 0, 20, 20);
    cairo_fill(cr);
}

void cairo_file_png(cairo_t *fbcr, const char *filename)
{
    cairo_surface_t *surface = cairo_get_target(fbcr);
    cairo_surface_write_to_png(surface, filename);
}


static void draw_simple(IDirectFB *dfb, IDirectFBSurface *surface)
{
    int   ret_width = 0;
    int   ret_height = 0;
    static int counter = 1;
    char filename[16];

    surface->GetSize(surface, &ret_width, &ret_height);
    fprintf(stderr, "%s # %i %d %d\n", __func__, __LINE__, ret_width, ret_height);
    cairo_surface_t *csurface = cairo_directfb_surface_create(dfb, surface);
    cairo_t *cr = cairo_create(csurface);
    draw(cr, ret_width, ret_height, csurface, surface);

    snprintf(filename, sizeof(filename), "%d.png", counter++);
    //cairo_file_png(cr, filename);

    cairo_surface_destroy(csurface);
    cairo_destroy(cr);

    where_am_i();
    surface->Flip(surface, NULL, 0);
    where_am_i();
}

int main(int argc, char *argv[])
{
    IDirectFB              *dfb;
    IDirectFBDisplayLayer  *layer;
    IDirectFBWindow        *window1;
    IDirectFBSurface       *window_surface1;
    IDirectFBEventBuffer   *buffer;

    DFBDisplayLayerConfig        layer_config;
    DFBGraphicsDeviceDescription desc;
    DFBWindowID                  id1;
    int err;
    int quit = 0;

    DFBCHECK(DirectFBInit(&argc, &argv));
    DFBCHECK(DirectFBCreate(&dfb));
    dfb->GetDeviceDescription(dfb, &desc);

    DFBCHECK(dfb->GetDisplayLayer(dfb, DLID_PRIMARY, &layer));

    layer->SetCooperativeLevel(layer, DLSCL_ADMINISTRATIVE);
    if (!((desc.blitting_flags & DSBLIT_BLEND_ALPHACHANNEL) &&
            (desc.blitting_flags & DSBLIT_BLEND_COLORALPHA)))
    {
        layer_config.flags = DLCONF_BUFFERMODE;
        layer_config.buffermode = DLBM_BACKSYSTEM;
        layer->SetConfiguration(layer, &layer_config);
    }
    layer->GetConfiguration(layer, &layer_config);
    layer->EnableCursor(layer, 1);
    {
        DFBWindowDescription desc;
        desc.flags  = (DWDESC_POSX | DWDESC_POSY | DWDESC_WIDTH | DWDESC_HEIGHT);

        desc.posx   = 0;
        desc.posy   = 0;
        desc.width  = layer_config.width;
        desc.height = layer_config.height;
        {
            printf(" Main surface with alpha channel \n");
            desc.caps   = DWCAPS_ALPHACHANNEL;
            desc.flags  |= DWDESC_CAPS;
        }

        DFBCHECK(layer->CreateWindow(layer, &desc, &window1));
        window1->CreateEventBuffer(window1, &buffer);
        window1->GetSurface(window1, &window_surface1);
        window_surface1->SetColor(window_surface1, 0xFF, 0x20, 0x20, 0xFF);
        window_surface1->DrawRectangle(window_surface1, 0, 0, desc.width, desc.height);

        window_surface1->Flip(window_surface1, NULL, 0);
        window1->AttachEventBuffer(window1, buffer);
        if (desc.caps == DWCAPS_ALPHACHANNEL)
        {
            /*window1->SetOpacity( window1, 0x7f );*/
            window1->SetOpacity(window1, 0xFF);
        }
        else
        {
            window1->SetOpacity(window1, 0xFF);
        }

        window1->GetID(window1, &id1);
    }

    window1->RequestFocus(window1);
    window1->RaiseToTop(window1);

    for (;;)
    {
        draw_simple(dfb, window_surface1);
    }

    buffer->Release(buffer);
    window_surface1->Release(window_surface1);
    window1->Release(window1);
    layer->Release(layer);
    dfb->Release(dfb);

    return 0;
}
