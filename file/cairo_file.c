#include <cairo.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

#define DEF_CAIRO_WIDTH     640
#define DEF_CAIRO_HEIGHT    480
#define DEF_CAIRO_HELLO     "Hello, MA35D1"

static   cairo_surface_t *png_surface;


void cairo_file_png(void)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, DEF_CAIRO_WIDTH, DEF_CAIRO_HEIGHT);
    cr = cairo_create(surface);

    cairo_set_source_surface(cr, png_surface, 20, DEF_CAIRO_HEIGHT / 2);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40.0);

    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, DEF_CAIRO_HELLO);

    cairo_surface_write_to_png(surface, "cairo.png");

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}

void cairo_file_pdf(void)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_pdf_surface_create("cairo.pdf", DEF_CAIRO_WIDTH, DEF_CAIRO_HEIGHT);
    cr = cairo_create(surface);

    cairo_set_source_surface(cr, png_surface, 20, DEF_CAIRO_HEIGHT / 2);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40.0);

    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, DEF_CAIRO_HELLO);

    cairo_show_page(cr);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);
}

void cairo_file_svg(void)
{
    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_svg_surface_create("cairo.svg", DEF_CAIRO_WIDTH, DEF_CAIRO_HEIGHT);
    cr = cairo_create(surface);

    cairo_set_source_surface(cr, png_surface, 20, DEF_CAIRO_HEIGHT / 2);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 40.0);

    cairo_move_to(cr, 10.0, 50.0);
    cairo_show_text(cr, DEF_CAIRO_HELLO);

    cairo_surface_destroy(surface);
    cairo_destroy(cr);

}


int main(void)
{
    png_surface = cairo_image_surface_create_from_png("nuvoton.png");

    cairo_file_png();
    cairo_file_pdf();
    cairo_file_svg();

    cairo_surface_destroy(png_surface);

    return 0;
}

