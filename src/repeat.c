/*
    RUNTIME ERROR: PCX Images "Snap" incorrectly.
    Image will rotate 95% of the way, and snap visually
    the last few frames.

    FIX: Adjust FPA.

    RUNTIME ERROR 2: Masks cannot be resized, so larger images
    are cutoff around the outsides :(

    FIX: Adjust the Masking rules and how they are sized
*/

#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fixed.h"
#include "stdimglib.h"

/* ==========================================================================
   VIDEO / PALETTE
   ========================================================================== */

void set_mode13(void) {
    union REGS r; r.h.ah = 0; r.h.al = 0x13; int86(0x10, &r, &r);
}
void set_text_mode(void) {
    union REGS r; r.h.ah = 0; r.h.al = 3; int86(0x10, &r, &r);
}
void set_palette_256(unsigned char pal[768]) {
    int i;
    outp(0x3C8, 0);
    for (i = 0; i < 768; i++)
        outp(0x3C9, pal[i] >> 2);
}

/* ==========================================================================
   DOUBLE BUFFER (BACKBUFFER)
   ========================================================================== */

unsigned char backbuffer[64000];

void clear_backbuffer(void)
{
    memset(backbuffer, 0, 64000);
}

void flip_buffer(void)
{
    unsigned char far *vram = (unsigned char far*)MK_FP(0xA000, 0);
    _fmemcpy(vram, backbuffer, 64000);
}

/* ==========================================================================
   MASK FUNCTIONS
   ========================================================================== */

int mask_circle(int x, int y, int w, int h)
{
    int cx = w / 2;
    int cy = h / 2;
    int dx = x - cx;
    int dy = y - cy;
    int r = (w < h ? w : h) / 2;
    return (dx*dx + dy*dy <= r*r);
}

int mask_triangle(int x, int y, int w, int h)
{
    float fx = (float)x / (float)w;
    float fy = (float)y / (float)h;
    return fy <= fx && fy <= (1.f - fx);
}

/* Transparent mask using palette index 0 */
unsigned char *g_pixels = NULL;

int mask_transparent(int x, int y, int w, int h)
{
    return g_pixels[y*w + x] != 0;
}

/* ==========================================================================
   ROTATE + SCALE + MASK BLITTER (8.8 FIXED)
   ========================================================================== */

void blit_mask(
    unsigned char *src,
    int sw, int sh,
    int dst_x, int dst_y,
    fixed angle_fix,
    fixed scale_fix,
    int (*maskfn)(int,int,int,int)
)
{
    int hx, hy;
    float angle_deg;
    float rad;
    fixed cs, sn;
    fixed invs;

    int y, x;
    int screen_x, screen_y;
    fixed fx, fy;
    fixed rx, ry;
    int sx, sy;

    hx = sw / 2;
    hy = sh / 2;

    angle_deg = (float)angle_fix / (float)FIX_ONE;
    rad = angle_deg * 3.14159265f / 180.0f;

    cs = FLOAT_TO_FIX(cos(rad));
    sn = FLOAT_TO_FIX(sin(rad));

    invs = FIX_DIV(FIX_ONE, scale_fix);

    for (y = -hy; y < hy; y++)
    {
        screen_y = dst_y + y;
        if (screen_y < 0 || screen_y >= 200) continue;

        for (x = -hx; x < hx; x++)
        {
            screen_x = dst_x + x;
            if (screen_x < 0 || screen_x >= 320) continue;

            fx = INT_TO_FIX(x);
            fy = INT_TO_FIX(y);

            rx = FIX_MUL(fx, cs) + FIX_MUL(fy, sn);
            ry = -FIX_MUL(fx, sn) + FIX_MUL(fy, cs);

            rx = FIX_MUL(rx, invs);
            ry = FIX_MUL(ry, invs);

            sx = FIX_TO_INT(rx) + hx;
            sy = FIX_TO_INT(ry) + hy;

            if ((unsigned)sx < (unsigned)sw && (unsigned)sy < (unsigned)sh)
            {
                if (maskfn(sx, sy, sw, sh))
                {
                    backbuffer[screen_y * 320 + screen_x] =
                        src[sy * sw + sx];
                }
            }
        }
    }
}

/* ==========================================================================
   MAIN
   ========================================================================== */

int main(void)
{
    unsigned char *pixels;
    unsigned char pal[768];
    int w, h;

    int cx = 160;
    int cy = 100;

    fixed angle;
    fixed scale;

    if (!load_pcx_8bit("..\\images\\brick.pcx", &pixels, pal, &w, &h)) {
        printf("Failed to load PCX.\n");
        getch();
        return 1;
    }

    g_pixels = pixels;

    set_mode13();
    set_palette_256(pal);

    angle = INT_TO_FIX(0);
    scale = FLOAT_TO_FIX(0.5f);    

    while (!kbhit()) {
        clear_backbuffer();

        blit_mask(
            pixels, w, h,
            cx, cy,
            angle,
            scale,
            mask_transparent
        );

        flip_buffer();

        /* Increment angle a little each frame */
        angle += INT_TO_FIX(1);

        /* Wrap around at 360 degrees */
        if (angle >= INT_TO_FIX(360))
            angle -= INT_TO_FIX(360);
    }

    getch();
    set_text_mode();
    free_image(pixels);
    return 0;
}
