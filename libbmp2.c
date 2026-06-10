/* Copyright 2016 - 2017 Marc Volker Dickmann
 * Project: LibBMP
 */
#include <stdio.h>
#include <stdlib.h>
#include "libbmp2.h"

/* BMP_HEADER */

void
bmp_header_init_df (bmp_header           *header,
                    const int             width,
                    const int             height,
                    const unsigned short  bit_count)
{
	size_t row_bytes;
	unsigned int palette_size;

	header->bfReserved = 0;
	header->biSize = 40;
	header->biWidth = width;
	header->biHeight = height;
	header->biPlanes = 1;
	header->biBitCount = bit_count;
	header->biCompression = 0;
	header->biSizeImage = 0;
	header->biXPelsPerMeter = 0;
	header->biYPelsPerMeter = 0;
	header->biClrUsed = 0;
	header->biClrImportant = 0;

	if (bit_count == 24)
	{
		row_bytes = sizeof (bmp_pixel) * width;
		palette_size = 0;
	}
	else if (bit_count == 8)
	{
		row_bytes = width;
		palette_size = 1024;
	}
	else
	{
		row_bytes = (width + 7) / 8;
		palette_size = 8;
	}

	header->bfOffBits = 54 + palette_size;
	header->bfSize = header->bfOffBits
	                 + (row_bytes + BMP_GET_PADDING (row_bytes)) * abs (height);
}

enum bmp_error
bmp_header_write (const bmp_header *header,
                  FILE             *img_file)
{
	const unsigned short magic = BMP_MAGIC;

	if (header == NULL)
	{
		return BMP_HEADER_NOT_INITIALIZED;
	}
	else if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}

	fwrite (&magic, sizeof (magic), 1, img_file);

	/* Use the type instead of the variable because its a pointer! */
	fwrite (header, sizeof (bmp_header), 1, img_file);
	return BMP_OK;
}

enum bmp_error
bmp_header_read (bmp_header *header,
                 FILE       *img_file)
{
	unsigned short magic;

	if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}

	/* Check if its an bmp file by comparing the magic nbr: */
	if (fread (&magic, sizeof (magic), 1, img_file) != 1 ||
	    magic != BMP_MAGIC)
	{
		return BMP_INVALID_FILE;
	}

	if (fread (header, sizeof (bmp_header), 1, img_file) != 1)
	{
		return BMP_ERROR;
	}

	return BMP_OK;
}

/* BMP_PALETTE */

void
bmp_palette_init (bmp_palette          *palette,
                  const unsigned int    n_entries)
{
	palette->entry = malloc (sizeof (bmp_palette_entry) * n_entries);
	palette->n_entries = n_entries;
}

void
bmp_palette_free (bmp_palette *palette)
{
	free (palette->entry);
	palette->entry = NULL;
	palette->n_entries = 0;
}

/* BMP_PIXEL */

void
bmp_pixel_init (bmp_pixel           *pxl,
                const unsigned char  red,
                const unsigned char  green,
                const unsigned char  blue)
{
	pxl->red = red;
	pxl->green = green;
	pxl->blue = blue;
}

void
bmp_pixel_1bit_set (bmp_img             *img,
                    const size_t         x,
                    const size_t         y,
                    const unsigned char  val)
{
	const size_t byte_idx = x / 8;

	if (val)
	{
		img->img_pixels_idx[y][byte_idx] |= (unsigned char)(1 << (7 - (int)(x % 8)));
	}
	else
	{
		img->img_pixels_idx[y][byte_idx] &= (unsigned char)(~(1 << (7 - (int)(x % 8))));
	}
}

unsigned char
bmp_pixel_1bit_get (const bmp_img  *img,
                    const size_t    x,
                    const size_t    y)
{
	const size_t byte_idx = x / 8;

	return (img->img_pixels_idx[y][byte_idx] >> (7 - (int)(x % 8))) & 1;
}

/* BMP_IMG */

void
bmp_img_alloc (bmp_img *img)
{
	const size_t h = abs (img->img_header.biHeight);
	const unsigned short bit_count = img->img_header.biBitCount;
	size_t y;
	size_t row_bytes;
	unsigned int n_colors;

	if (bit_count == 24)
	{
		img->img_pixels = malloc (sizeof (bmp_pixel*) * h);
		for (y = 0; y < h; y++)
		{
			img->img_pixels[y] = malloc (sizeof (bmp_pixel) * img->img_header.biWidth);
		}
	}
	else if (bit_count == 1 || bit_count == 8)
	{
		if (bit_count == 8)
		{
			row_bytes = img->img_header.biWidth;
			n_colors = img->img_header.biClrUsed;
			if (n_colors == 0)
			{
				n_colors = 256;
			}
		}
		else
		{
			row_bytes = (img->img_header.biWidth + 7) / 8;
			n_colors = img->img_header.biClrUsed;
			if (n_colors == 0)
			{
				n_colors = 2;
			}
		}

		img->img_pixels_idx = malloc (sizeof (unsigned char*) * h);
		for (y = 0; y < h; y++)
		{
			img->img_pixels_idx[y] = malloc (sizeof (unsigned char) * row_bytes);
		}

		img->img_palette = malloc (sizeof (bmp_palette));
		img->img_palette->entry = malloc (sizeof (bmp_palette_entry) * n_colors);
		img->img_palette->n_entries = n_colors;
	}
}

void
bmp_img_init_df (bmp_img              *img,
                 const int             width,
                 const int             height,
                 const unsigned short  bit_count)
{
	bmp_header_init_df (&img->img_header, width, height, bit_count);

	img->img_pixels = NULL;
	img->img_pixels_idx = NULL;
	img->img_palette = NULL;

	bmp_img_alloc (img);
}

void
bmp_img_free (bmp_img *img)
{
	const size_t h = abs (img->img_header.biHeight);
	size_t y;

	if (img->img_pixels != NULL)
	{
		for (y = 0; y < h; y++)
		{
			free (img->img_pixels[y]);
		}
		free (img->img_pixels);
		img->img_pixels = NULL;
	}

	if (img->img_pixels_idx != NULL)
	{
		for (y = 0; y < h; y++)
		{
			free (img->img_pixels_idx[y]);
		}
		free (img->img_pixels_idx);
		img->img_pixels_idx = NULL;
	}

	if (img->img_palette != NULL)
	{
		free (img->img_palette->entry);
		free (img->img_palette);
		img->img_palette = NULL;
	}
}

enum bmp_error
bmp_img_write (const bmp_img *img,
               const char    *filename)
{
	FILE *img_file;
	enum bmp_error err;
	const unsigned short bit_count = img->img_header.biBitCount;
	const int i_h = abs (img->img_header.biHeight);
	const int i_offset = (img->img_header.biHeight > 0 ? i_h - 1 : 0);
	unsigned char padding[3];
	int y;
	size_t row_bytes;
	unsigned int n_colors;

	img_file = fopen (filename, "wb");
	if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}

	err = bmp_header_write (&img->img_header, img_file);
	if (err != BMP_OK)
	{
		fclose (img_file);
		return err;
	}

	/* Write the palette for indexed modes: */
	if (bit_count <= 8 && img->img_palette != NULL)
	{
		n_colors = img->img_header.biClrUsed;
		if (n_colors == 0)
		{
			n_colors = img->img_palette->n_entries;
		}
		fwrite (img->img_palette->entry, sizeof (bmp_palette_entry),
		        n_colors, img_file);
	}

	padding[0] = '\0';
	padding[1] = '\0';
	padding[2] = '\0';

	if (bit_count == 24)
	{
		row_bytes = img->img_header.biWidth * sizeof (bmp_pixel);
	}
	else if (bit_count == 8)
	{
		row_bytes = img->img_header.biWidth;
	}
	else
	{
		row_bytes = (img->img_header.biWidth + 7) / 8;
	}

	for (y = 0; y < i_h; y++)
	{
		if (bit_count == 24)
		{
			fwrite (img->img_pixels[abs (i_offset - y)], sizeof (bmp_pixel),
			        img->img_header.biWidth, img_file);
		}
		else
		{
			fwrite (img->img_pixels_idx[abs (i_offset - y)], sizeof (unsigned char),
			        row_bytes, img_file);
		}

		fwrite (padding, sizeof (unsigned char),
		        BMP_GET_PADDING (row_bytes), img_file);
	}

	fclose (img_file);
	return BMP_OK;
}

enum bmp_error
bmp_img_read (bmp_img    *img,
              const char *filename)
{
	FILE *img_file;
	enum bmp_error err;
	unsigned short bit_count;
	int i_h;
	int i_offset;
	int y;
	size_t row_bytes;
	size_t padding_bytes;
	unsigned int palette_entries;

	img_file = fopen (filename, "rb");
	if (img_file == NULL)
	{
		return BMP_FILE_NOT_OPENED;
	}

	err = bmp_header_read (&img->img_header, img_file);
	if (err != BMP_OK)
	{
		fclose (img_file);
		return err;
	}

	bit_count = img->img_header.biBitCount;
	i_h = abs (img->img_header.biHeight);
	i_offset = (img->img_header.biHeight > 0 ? i_h - 1 : 0);

	img->img_pixels = NULL;
	img->img_pixels_idx = NULL;
	img->img_palette = NULL;

	bmp_img_alloc (img);

	/* Read the palette for indexed modes: */
	if (bit_count <= 8 && img->img_palette != NULL)
	{
		palette_entries = img->img_palette->n_entries;
		if (fread (img->img_palette->entry, sizeof (bmp_palette_entry),
		           palette_entries, img_file) != palette_entries)
		{
			fclose (img_file);
			return BMP_ERROR;
		}
	}

	if (bit_count == 24)
	{
		row_bytes = img->img_header.biWidth * sizeof (bmp_pixel);
		padding_bytes = BMP_GET_PADDING (row_bytes);

		for (y = 0; y < i_h; y++)
		{
			if (fread (img->img_pixels[abs (i_offset - y)], sizeof (bmp_pixel),
			           (size_t)img->img_header.biWidth, img_file) != (size_t)img->img_header.biWidth)
			{
				fclose (img_file);
				return BMP_ERROR;
			}
			fseek (img_file, padding_bytes, SEEK_CUR);
		}
	}
	else
	{
		if (bit_count == 8)
		{
			row_bytes = img->img_header.biWidth;
		}
		else
		{
			row_bytes = (img->img_header.biWidth + 7) / 8;
		}

		padding_bytes = BMP_GET_PADDING (row_bytes);

		for (y = 0; y < i_h; y++)
		{
			if (fread (img->img_pixels_idx[abs (i_offset - y)], sizeof (unsigned char),
			           row_bytes, img_file) != row_bytes)
			{
				fclose (img_file);
				return BMP_ERROR;
			}
			fseek (img_file, padding_bytes, SEEK_CUR);
		}
	}

	fclose (img_file);
	return BMP_OK;
}
