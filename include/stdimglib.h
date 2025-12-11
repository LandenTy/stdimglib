/* stdimglib.h
   Simple 8-bit PCX loader (header-only)
   Designed for DOS / OpenWatcom / 16-bit C89 environments.
*/

#ifndef STDIMGLIB_H
#define STDIMGLIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* PCX file header (always 128 bytes) */
#pragma pack(1)
typedef struct {
    unsigned char Manufacturer;
    unsigned char Version;
    unsigned char Encoding;
    unsigned char BitsPerPixel;
    unsigned short Xmin;
    unsigned short Ymin;
    unsigned short Xmax;
    unsigned short Ymax;
    unsigned short Hres;
    unsigned short Vres;
    unsigned char ColorMap[48];
    unsigned char Reserved;
    unsigned char ColorPlanes;
    unsigned short BytesPerLine;
    unsigned short PaletteType;
    unsigned short HScreenSize;
    unsigned short VScreenSize;
    unsigned char Filler[54];
} PCXHeader;
#pragma pack()

/* 
   Loads an 8-bit PCX file.
   - path: filename
   - out_pixels: mallocs pixel buffer (width * height)
   - palette_out: 768-byte RGB array (0-255 per channel)
   - out_width/out_height: image dimensions
   Returns 1 on success, 0 on failure.
*/
static int load_pcx_8bit(const char *path,
                         unsigned char **out_pixels,
                         unsigned char palette_out[768],
                         int *out_width,
                         int *out_height)
{
    FILE *f;
    PCXHeader hdr;
    unsigned short bytesPerLine;
    unsigned char *pixels = NULL;
    unsigned char *scanbuf = NULL;
    int width, height, y;
    long file_end_pos;
    size_t readcount;

    if (!path || !out_pixels || !palette_out || !out_width || !out_height)
        return 0;

    f = fopen(path, "rb");
    if (!f)
        return 0;

    /* read PCX header */
    if (fread(&hdr, 1, sizeof(PCXHeader), f) != sizeof(PCXHeader)) {
        fclose(f);
        return 0;
    }

    /* validate */
    if (hdr.Manufacturer != 0x0A || hdr.Encoding != 1 ||
        hdr.BitsPerPixel != 8 || hdr.ColorPlanes != 1) {
        fclose(f);
        return 0;
    }

    width  = hdr.Xmax - hdr.Xmin + 1;
    height = hdr.Ymax - hdr.Ymin + 1;
    bytesPerLine = hdr.BytesPerLine;

    if (width <= 0 || height <= 0) {
        fclose(f);
        return 0;
    }

    pixels = (unsigned char *)malloc(width * height);
    if (!pixels) {
        fclose(f);
        return 0;
    }

    scanbuf = (unsigned char *)malloc(bytesPerLine);
    if (!scanbuf) {
        free(pixels);
        fclose(f);
        return 0;
    }

    /* decode PCX RLE */
    for (y = 0; y < height; ++y) {
        int x = 0;
        while (x < bytesPerLine) {
            int c = fgetc(f);
            if (c == EOF) goto fail;
            if ((c & 0xC0) == 0xC0) {
                int count = c & 0x3F;
                int val = fgetc(f);
                if (val == EOF) goto fail;
                while (count-- && x < bytesPerLine)
                    scanbuf[x++] = (unsigned char)val;
            } else {
                scanbuf[x++] = (unsigned char)c;
            }
        }
        memcpy(pixels + y * width, scanbuf, width);
    }

    /* read palette (0x0C + 768 bytes) */
    fseek(f, 0, SEEK_END);
    file_end_pos = ftell(f);
    if (file_end_pos < 769L) goto fail;
    fseek(f, file_end_pos - 769L, SEEK_SET);
    if (fgetc(f) != 0x0C) goto fail;

    readcount = fread(palette_out, 1, 768, f);
    if (readcount != 768) goto fail;

    fclose(f);
    free(scanbuf);
    *out_pixels = pixels;
    *out_width = width;
    *out_height = height;
    return 1;

fail:
    fclose(f);
    if (scanbuf) free(scanbuf);
    if (pixels) free(pixels);
    return 0;
}

/* Frees the pixel buffer allocated by load_pcx_8bit() */
static void free_image(unsigned char *pixels)
{
    if (pixels) free(pixels);
}

#endif /* STDIMGLIB_H */
