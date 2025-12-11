# stdimglib
stdimglib is an open-sourced DOS Image library that supports camera-space positioning of images.

# Feature Support:
---------
 - PCX Images (far and near)
 - Rectangle Image Mask
 - Triangle Image Mask
 - Polygon Image Mask
 - Transparent Image Mask
 - VGA Mode13h
 - Double Buffering
 - Fixed 8.8 Math
 - Image Positions
 - Image Rotations
 - Image Scales

# Working on Supporting:
---------
 - CGA (4-colour, 2-bit)
 - EGA (16-colour, 4-bit)
 - BMP Images

# Loading Images
```C
if (!load_pcx_8bit("..\\images\\brick.pcx", &pixels, pal, &w, &h)) {
        printf("Failed to load PCX.\n");
        getch();
        return 1;
}
```
This code just tries to load the 8-bit PCX Image, if it fails it will say it failed.

# Fixed Point Operations
`INT_TO_FIX` Converts an Integer to a fixed-point number

`FIX_TO_INT` Converts a fixed-point number back to an Integer

`FLOAT_TO_FIX` Converts a floating point number to a fixed-point number

`FIX_MUL` Multiplies two numbers and converts them to a fixed-point decimal

`FIX_DIV` Divides two numbers and converts them to a fixed-point decimal
