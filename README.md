# libbmp2

A simple Bitmap (BMP) library written in ANSI C without dependencies.

This is a fork of the original [libbmp](https://github.com/marc-q/libbmp) by
Marc Volker Dickmann, with the following additions:

- **8-bit indexed** (256-color) BMP read/write support
- **1-bit monochrome** BMP read/write support
- Full **palette** API
- Strict **ANSI C (C89)** compliance — compiles cleanly with
  `gcc -Wall -Wextra -Wpedantic -std=c89`

### Example: 24-bit checkerboard

```c
#include <stdio.h>
#include "libbmp2.h"

int
main (int argc, char *argv[])
{
    bmp_img img;
    size_t y;
    size_t x;

    (void)argc;
    (void)argv;

    bmp_img_init_df (&img, 512, 512, 24);

    /* Draw a checkerboard pattern: */
    for (y = 0; y < 512; y++)
    {
        for (x = 0; x < 512; x++)
        {
            if ((y % 128 < 64 && x % 128 < 64) ||
                (y % 128 >= 64 && x % 128 >= 64))
            {
                bmp_pixel_init (&img.img_pixels[y][x], 250, 250, 250);
            }
            else
            {
                bmp_pixel_init (&img.img_pixels[y][x], 0, 0, 0);
            }
        }
    }

    bmp_img_write (&img, "test.bmp");
    bmp_img_free (&img);
    return 0;
}
```

### Example: 8-bit grayscale

```c
#include <stdio.h>
#include "libbmp2.h"

int
main (void)
{
    bmp_img img;
    unsigned int i;
    size_t y;
    size_t x;

    bmp_img_init_df (&img, 256, 256, 8);

    /* Fill palette with grayscale: */
    for (i = 0; i < 256; i++)
    {
        img.img_palette->entry[i].blue  = (unsigned char)i;
        img.img_palette->entry[i].green = (unsigned char)i;
        img.img_palette->entry[i].red   = (unsigned char)i;
        img.img_palette->entry[i].reserved = 0;
    }

    /* Fill pixels: */
    for (y = 0; y < 256; y++)
    {
        for (x = 0; x < 256; x++)
        {
            img.img_pixels_idx[y][x] = (unsigned char)((x + y) % 256);
        }
    }

    bmp_img_write (&img, "grayscale.bmp");
    bmp_img_free (&img);
    return 0;
}
```

### License

GNU General Public License v3.0 — see [LICENSE](LICENSE).

Original work Copyright (C) 2016-2017 Marc Volker Dickmann.  
Modifications Copyright (C) 2024 [llichlet](https://github.com/lllichlet).
