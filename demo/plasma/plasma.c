#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dos.h>
#include "types.h"
#include "pcx.h"
#include "vesa.h"

int main(void)
{
    VBESURFACE *vbesurface_ptr;
    FLATIMAGE  *flatimage_ptr;

    ulong x, y, *offscreen, center_x, center_y;
    ulong angle;

    float sin_table[1024];
    float cos_table[1024];

    ulong *colortable;
    ulong colorindex;

    if((flatimage_ptr = (FLATIMAGE*)
        loadPcxImage((unsigned char*) "palette.pcx", 32)) == 0) {
        return 1;
    } else {
        colortable = (long unsigned int*) &flatimage_ptr->image;
    }

    if((vbesurface_ptr = VBEinit(320, 240, 32)) == 0) {
        puts("\nCould not initialize video.");
        free(flatimage_ptr);
        return 2;
    } else {
        offscreen = (long unsigned int*) vbesurface_ptr->offscreen_ptr;
    }

    for(angle = 0; angle < 1024; angle++)
        sin_table[angle] = sin((float)(PI / 512.0 * angle));
    for(angle = 0; angle < 1024; angle++)
        cos_table[angle] = cos((float)(PI / 512.0 * angle));

    angle = 0;

    while(!kbhit()) {

        angle += 16;

        center_x = 160 + 100 * cos_table[angle % 1024];
        center_y = 120 + 100 * sin_table[angle % 1024];

        for(x = 0; x < 320; x++)
                for(y = 0; y < 240; y++) {
                        colorindex = 128
                                     + 63 * sin_table[(long)((hypot(center_y + (y-center_y/2), center_x + (x-center_x/2)) / 16) * (512.0 / PI)) % 1024]
                                     + 63 * sin_table[(long)((x / (30 + 15 * cos(y / 240))) * (512.0 / PI)) % 1024] *
                                     cos_table[(long)((y / (11 + 20 * sin(x / 320))) * (512.0 / PI)) % 1024] ;

                        offscreen[y*320+x] = colortable[colorindex*32];
                }

        flipScreen();
    }

    VBEshutdown();
    free(flatimage_ptr);
    return 0;
}
