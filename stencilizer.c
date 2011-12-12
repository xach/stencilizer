/* Convert an 8-bit-per-channel RGBA PNG to a stencil image; all color
 * channels are converted to black, and the alpha channel is inverted.
 */

#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include <string.h>

void fail (char *why)
{
    fputs(why, stderr);
    fputs("\n", stderr);
    exit(1);
}

void *pmalloc (size_t size)
{
    void *result = malloc(size);
    
    if (result == NULL) {
	fail("out of memory");
    }
}

typedef struct {
    png_structp png;
    png_infop info;
    png_bytep row;
    unsigned long rowstride;
} InPng;

InPng *open_png (char *filename)
{
    InPng *in_png;
    FILE *fh;

    in_png = pmalloc(sizeof(InPng));

    if (!(fh = fopen(filename, "rb"))) {
	fail("could not open png input file");
    }

    in_png->png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
					 NULL, NULL, NULL);
    
    if (!in_png->png) {
	fail("png_create_read_struct failure");
    }

    in_png->info = png_create_info_struct(in_png->png);

    if (!in_png->info) {
	fail("png_create_info_struct failed");
    }

    png_init_io(in_png->png, fh);

    png_read_info(in_png->png, in_png->info);

    if (in_png->info->color_type != PNG_COLOR_TYPE_RGB_ALPHA) {
	fail("only RGBA pngs are supported");
    }

    if (in_png->info->bit_depth != 8) {
	fail("only 8-bit pngs are supported");
    }

    in_png->rowstride = in_png->info->width * 4;
    in_png->row = (png_bytep )pmalloc(in_png->rowstride * sizeof(png_byte));

    return in_png;
}

int stencilize (InPng *in_png, char *output_filename)
{
    FILE *fh;
    png_structp png;
    png_infop png_info;
    png_bytep row;
    int i, j;

    if (!(fh = fopen(output_filename, "wb"))) {
	fail("failed to open output file");
    }

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
				  NULL, NULL, NULL);
    
    if (!png) {
	fail("could not create png");
    }

    png_info = png_create_info_struct(png);

    if (!png_info) {
	fail("could not create png_info");
    }

    png_init_io(png, fh);

    png_set_IHDR(png, png_info,
		 in_png->info->width,
		 in_png->info->height,
		 in_png->info->bit_depth,
		 in_png->info->color_type,
		 in_png->info->interlace_type,
		 PNG_COMPRESSION_TYPE_DEFAULT,
		 PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png, png_info);

    row = in_png->row;
    for (i = 0; i < in_png->info->height; i++) {
	png_read_row(in_png->png, row, NULL);
	j = 0;
	while (j < in_png->rowstride) {
	    row[j + 0] = 0;
	    row[j + 1] = 0;
	    row[j + 2] = 0;
	    row[j + 3] = 255 - row[j + 3];
	    j += 4;
	}
	png_write_row(png, row);
    }

    png_write_end(png, png_info);
    fclose(fh);
    return 1;
}

int main (int argc, char *argv[])
{
    InPng *in_png;
    char *input_file;
    char *output_file;

    if (argc != 3) {
	fail("usage: stencilizer INPUT-PNG OUTPUT-PNG");
    }
    
    input_file = argv[1];
    output_file = argv[2];

    in_png = open_png(input_file);
    stencilize(in_png, output_file);

    return 0;
}
