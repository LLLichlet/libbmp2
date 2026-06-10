/* Copyright 2016 - 2017 Marc Volker Dickmann
 * Project: LibBMP
 */
#include <stdio.h>
#include "../libbmp2.h"

#define BMP_TEST_PASSED 1
#define BMP_TEST_FAILED 0

/* Utils */

static void
bmp_test_print_summary (const int points,
                        const int points_max)
{
	printf ("\n\nPoints\t%i/%i\n", points, points_max);
	printf ("Failed\t%i\n", points_max - points);
}

static void
bmp_test_print_passed (const char *name)
{
	printf ("%s\t\tPASSED!\n", name);
}

static void
bmp_test_print_failed (const char *name)
{
	printf ("%s\t\tFAILED!\n", name);
}

/* MACROS */

static int
bmp_test_get_padding (void)
{
	if (BMP_GET_PADDING (0) == 0 &&
	    BMP_GET_PADDING (1) == 3 &&
	    BMP_GET_PADDING (2) == 2 &&
	    BMP_GET_PADDING (3) == 1 &&
	    BMP_GET_PADDING (4) == 0 &&
	    BMP_GET_PADDING (5) == 3 &&
	    BMP_GET_PADDING (6) == 2 &&
	    BMP_GET_PADDING (7) == 1 &&
	    BMP_GET_PADDING (8) == 0)
	{
		bmp_test_print_passed ("BMP_GET_PADDING");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("BMP_GET_PADDING");
	return BMP_TEST_FAILED;
}

/* Header */

static int
bmp_test_header_size (void)
{
	/* Note: Its 52 Bytes because the header doesn't include the magic field! */
	if (sizeof (bmp_header) == 52)
	{
		bmp_test_print_passed ("header_size");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("header_size");
	return BMP_TEST_FAILED;
}

static int
bmp_test_header_init_df (void)
{
	int passed = BMP_TEST_PASSED;
	bmp_header header;

	/* Test positive height value (24-bit): */
	bmp_header_init_df (&header, 100, 100, 24);

	if (header.bfSize != 30054 ||
	    header.bfOffBits != 54 ||
	    header.biWidth != 100 ||
	    header.biHeight != 100)
	{
		passed = BMP_TEST_FAILED;
	}

	/* Test negative height value with padding (24-bit): */
	bmp_header_init_df (&header, 102, -100, 24);

	if (header.bfSize != 30854 ||
	    header.bfOffBits != 54 ||
	    header.biWidth != 102 ||
	    header.biHeight != -100)
	{
		passed = BMP_TEST_FAILED;
	}

	/* Test 8-bit header: */
	bmp_header_init_df (&header, 64, 64, 8);

	if (header.bfOffBits != 1078 ||
	    header.biBitCount != 8)
	{
		passed = BMP_TEST_FAILED;
	}

	/* Test 1-bit header: */
	bmp_header_init_df (&header, 16, 16, 1);

	if (header.bfOffBits != 62 ||
	    header.biBitCount != 1)
	{
		passed = BMP_TEST_FAILED;
	}

	/* Return the result: */
	if (passed == BMP_TEST_PASSED)
	{
		bmp_test_print_passed ("header_init_df");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("header_init_df");
	return BMP_TEST_FAILED;
}

/* Pixel */

static int
bmp_test_pixel_init (void)
{
	bmp_pixel pxl;

	bmp_pixel_init (&pxl, 1, 250, 4);

	if (pxl.red == 1 &&
	    pxl.green == 250 &&
	    pxl.blue == 4)
	{
		bmp_test_print_passed ("pixel_init");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("pixel_init");
	return BMP_TEST_FAILED;
}

static int
bmp_test_pixel_1bit (void)
{
	bmp_img img;
	int x;
	int passed = BMP_TEST_PASSED;

	bmp_img_init_df (&img, 16, 1, 1);

	/* Set a pattern: 1 0 1 0 1 0 1 0 | 0 1 0 1 0 1 0 1 */
	for (x = 0; x < 8; x++)
	{
		bmp_pixel_1bit_set (&img, x, 0, (unsigned char)((x + 1) % 2));
	}
	for (x = 8; x < 16; x++)
	{
		bmp_pixel_1bit_set (&img, x, 0, (unsigned char)(x % 2));
	}

	/* Verify: */
	if (bmp_pixel_1bit_get (&img, 0, 0) != 1 ||
	    bmp_pixel_1bit_get (&img, 1, 0) != 0 ||
	    bmp_pixel_1bit_get (&img, 7, 0) != 0 ||
	    bmp_pixel_1bit_get (&img, 8, 0) != 0 ||
	    bmp_pixel_1bit_get (&img, 9, 0) != 1 ||
	    bmp_pixel_1bit_get (&img, 15, 0) != 1)
	{
		passed = BMP_TEST_FAILED;
	}

	bmp_img_free (&img);

	if (passed == BMP_TEST_PASSED)
	{
		bmp_test_print_passed ("pixel_1bit");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("pixel_1bit");
	return BMP_TEST_FAILED;
}

/* Palette */

static int
bmp_test_palette (void)
{
	bmp_palette pal;
	int passed = BMP_TEST_PASSED;

	bmp_palette_init (&pal, 4);

	pal.entry[0].blue = 0;
	pal.entry[0].green = 0;
	pal.entry[0].red = 0;
	pal.entry[1].blue = 255;
	pal.entry[1].green = 255;
	pal.entry[1].red = 255;

	if (pal.n_entries != 4 ||
	    pal.entry[0].blue != 0 ||
	    pal.entry[1].red != 255)
	{
		passed = BMP_TEST_FAILED;
	}

	bmp_palette_free (&pal);

	if (passed == BMP_TEST_PASSED)
	{
		bmp_test_print_passed ("palette");
		return BMP_TEST_PASSED;
	}

	bmp_test_print_failed ("palette");
	return BMP_TEST_FAILED;
}

int
main (int argc, char *argv[])
{
	int points = 0;

	(void)argc;
	(void)argv;

	printf ("LibBMP-Test v. 0.1.0 A (C) 2016 - 2017 Marc Volker Dickmann\n\n");

	points += bmp_test_get_padding ();

	points += bmp_test_header_size ();
	points += bmp_test_header_init_df ();

	points += bmp_test_pixel_init ();
	points += bmp_test_pixel_1bit ();
	points += bmp_test_palette ();

	bmp_test_print_summary (points, 6);
	return 0;
}
