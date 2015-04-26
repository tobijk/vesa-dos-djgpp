/*****************************************************************************
*COMP:DJGPP                     PCX Routines                                 *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "types.h"
#include "pcx.h"

/*****************************************************************************
*                                                                            *
*                              loadPcxHeader                                 *
*                                                                            *
*****************************************************************************/
PCXHEADER *getPcxInfo(FILE *pcx_file_ptr)
{
    unsigned long file_cur_position;
    PCXHEADER *pcx_header_ptr;

    file_cur_position = ftell(pcx_file_ptr); 
    rewind(pcx_file_ptr); 

    /* get 128 bytes from the beginning of the file */
    if((pcx_header_ptr = malloc(128 * sizeof(char))) == 0) return(0);
    if(fread(pcx_header_ptr , sizeof(char), 128, pcx_file_ptr) != 128) {
        free(pcx_header_ptr);
        return(0);
    }

    fseek(pcx_file_ptr, file_cur_position, SEEK_SET);
    return(pcx_header_ptr); 
}

/*****************************************************************************
*                                                                            *
*                              LoadPcxImage (8Bit)                           *
*                                                                            *
*****************************************************************************/
FLATIMAGE_8Bit *loadPcxImage_8Bit(FILE *pcx_file_ptr, PCXHEADER *pcx_header_ptr)
{
    FLATIMAGE_8Bit *flat_image_ptr;
    unsigned char  pcx_byte, pcx_count_byte;
    unsigned long  pcx_image_size, pcx_counter, color;

    pcx_image_size = (pcx_header_ptr->pcx_ymax - pcx_header_ptr->pcx_ymin + 1) *
                   (pcx_header_ptr->pcx_xmax - pcx_header_ptr->pcx_xmin + 1);

    if((flat_image_ptr = malloc(2 * sizeof(long) + pcx_image_size + 768)) == 0)
        return(0);

    flat_image_ptr->width = (pcx_header_ptr->pcx_xmax - pcx_header_ptr->pcx_ymin + 1);
    flat_image_ptr->height = (pcx_header_ptr->pcx_ymax - pcx_header_ptr->pcx_ymin + 1);

    // LOAD THE PALETTE VALUES
    fseek(pcx_file_ptr, -768, SEEK_END);
    if((fread(&flat_image_ptr->palette[0][0], sizeof(char), 768,
              pcx_file_ptr)) != 768) {
        free(flat_image_ptr);
        return(0);
    }

    for(color = 0; color < 256; color++) { 
        flat_image_ptr->palette[color][0] = flat_image_ptr->palette[color][0] / 4;
        flat_image_ptr->palette[color][1] = flat_image_ptr->palette[color][1] / 4;
        flat_image_ptr->palette[color][2] = flat_image_ptr->palette[color][2] / 4;
    }

    // DECODE IMAGE
    fseek(pcx_file_ptr, 128, SEEK_SET);
    for(pcx_counter = 0; pcx_counter < pcx_image_size;) {
        if(fread(&pcx_byte, sizeof(char), 1, pcx_file_ptr) != 1) {
            free(flat_image_ptr);
            return(0);
        }
        if(pcx_byte > 192) {
            pcx_count_byte = pcx_byte - 192;
            if(fread(&pcx_byte, sizeof(char), 1, pcx_file_ptr) != 1) {
                free(flat_image_ptr);
                return(0);
            }
        } else pcx_count_byte = 1;

        for(; pcx_count_byte > 0; pcx_count_byte--) {
            (&flat_image_ptr->image)[pcx_counter] = pcx_byte;
            pcx_counter++;
            if((pcx_header_ptr->pcx_bytes_per_line - flat_image_ptr->width == 1) &&
                    (pcx_counter % (flat_image_ptr->width) == 0))
                fseek(pcx_file_ptr, 1, SEEK_CUR);
        }
    }

    return(flat_image_ptr);
}

/*****************************************************************************
*                                                                            *
*                              LoadPcxImage (24Bit)                          *
*                                                                            *
*****************************************************************************/
FLATIMAGE_32Bit *loadPcxImage_24Bit(FILE *pcx_file_ptr, PCXHEADER *pcx_header_ptr)
{
    FLATIMAGE_32Bit *flat_image_ptr;
    unsigned char *scanline_ptr;
    unsigned char  pcx_byte, pcx_count_byte;
    unsigned long  pcx_image_size, pixel_nr, scanline;

    pcx_image_size = (pcx_header_ptr->pcx_ymax - pcx_header_ptr->pcx_ymin + 1) *
                   (pcx_header_ptr->pcx_xmax - pcx_header_ptr->pcx_xmin + 1);

    if((flat_image_ptr = malloc(2 * sizeof(long) + pcx_image_size * sizeof(long))) == 0)
        return(0);

    flat_image_ptr->width = (pcx_header_ptr->pcx_xmax - pcx_header_ptr->pcx_ymin + 1);
    flat_image_ptr->height = (pcx_header_ptr->pcx_ymax - pcx_header_ptr->pcx_ymin + 1);

    // DECODE IMAGE
    fseek(pcx_file_ptr, 128, SEEK_SET);
    if((scanline_ptr = malloc(pcx_header_ptr->pcx_bytes_per_line * 3)) == 0) {
        free(flat_image_ptr);
        return(0);
    }

    for(scanline = 0; scanline < flat_image_ptr->height; scanline++) {
        for(pixel_nr = 0; pixel_nr < pcx_header_ptr->pcx_bytes_per_line * 3;) {
            if(fread(&pcx_byte, sizeof(char), 1, pcx_file_ptr) != 1) {
                free(scanline_ptr);
                free(flat_image_ptr);
                return(0);
            }
            if(pcx_byte > 192) {
                pcx_count_byte = pcx_byte - 192;
                if(fread(&pcx_byte, sizeof(char), 1, pcx_file_ptr) != 1) {
                    free(scanline_ptr);
                    free(flat_image_ptr);
                    return(0);
                }
            } else pcx_count_byte = 1;

            for(; pcx_count_byte > 0; pcx_count_byte--) {
                scanline_ptr[pixel_nr] = pcx_byte;
                pixel_nr++;
            }
        }

        for(pixel_nr = 0; pixel_nr < pcx_header_ptr->pcx_bytes_per_line; pixel_nr++)
            ((char*)(&flat_image_ptr->image))[2 + pixel_nr * 4 + scanline *
                                            pcx_header_ptr->pcx_bytes_per_line * 4]
            = scanline_ptr[pixel_nr];

        for(pixel_nr = 0; pixel_nr < pcx_header_ptr->pcx_bytes_per_line; pixel_nr++)
            ((char*)(&flat_image_ptr->image))[1 + pixel_nr * 4 + scanline *
                                            pcx_header_ptr->pcx_bytes_per_line * 4]
            = scanline_ptr[pixel_nr + pcx_header_ptr->pcx_bytes_per_line];

        for(pixel_nr = 0; pixel_nr < pcx_header_ptr->pcx_bytes_per_line; pixel_nr++)
            ((char*)(&flat_image_ptr->image))[0 + pixel_nr * 4 + scanline *
                                            pcx_header_ptr->pcx_bytes_per_line * 4]
            = scanline_ptr[pixel_nr + pcx_header_ptr->pcx_bytes_per_line * 2];
    }

    free(scanline_ptr);
    return(flat_image_ptr);
}

/*****************************************************************************
*                                                                            *
*                              LoadPcxImage                                  *
*                                                                            *
*****************************************************************************/
void *loadPcxImage(uchar *image_filename, uchar bpp)
{
    FILE  *image_file_ptr;
    FLATIMAGE       *flat_image_ptr;
    FLATIMAGE_8Bit  *flat_image_ptr_8Bit;
    FLATIMAGE_32Bit *flat_image_ptr_32Bit;
    FLATIMAGE_16Bit *flat_image_ptr_16Bit;
    PCXHEADER  *pcx_header_ptr;

    if((image_file_ptr = fopen(image_filename, "rb")) == 0) return(0);
    if((pcx_header_ptr = getPcxInfo(image_file_ptr)) == 0) {
        fclose(image_file_ptr);
        return(0);
    }

    if(((pcx_header_ptr->pcx_bits_per_pixel) *(pcx_header_ptr->pcx_num_planes)) == 8) {
        flat_image_ptr_8Bit = loadPcxImage_8Bit(image_file_ptr, pcx_header_ptr);
        flat_image_ptr_32Bit = image8Bit_to_32Bit(flat_image_ptr_8Bit);
        if(flat_image_ptr_8Bit) free(flat_image_ptr_8Bit);
    } else if(((pcx_header_ptr->pcx_bits_per_pixel) *(pcx_header_ptr->pcx_num_planes)) == 24) {
        flat_image_ptr_32Bit = loadPcxImage_24Bit(image_file_ptr, pcx_header_ptr);
    } else return(0);

    if(flat_image_ptr_32Bit == 0) {
        fclose(image_file_ptr);
        free(pcx_header_ptr);
        return(0);
    }

    if(bpp == 8) {
        flat_image_ptr = image32Bit_to_8Bit(flat_image_ptr_32Bit);
        free(flat_image_ptr_32Bit);
        free(pcx_header_ptr);
        fclose(image_file_ptr);
        return(flat_image_ptr);          // return 8 bit depth
    }
    if(bpp == 16) {
        flat_image_ptr_16Bit = image32Bit_to_16Bit(flat_image_ptr_32Bit);
        free(flat_image_ptr_32Bit);
        free(pcx_header_ptr);
        fclose(image_file_ptr);
        return(flat_image_ptr_16Bit);         // return 16 bit depth
    }
    if(bpp == 15) {
        flat_image_ptr_16Bit = image32Bit_to_15Bit(flat_image_ptr_32Bit);
        free(flat_image_ptr_32Bit);
        free(pcx_header_ptr);
        fclose(image_file_ptr);
        return(flat_image_ptr_16Bit);         // return 15 bit depth
    }
    if(bpp == 32) {
        free(pcx_header_ptr);
        fclose(image_file_ptr);
        return(flat_image_ptr_32Bit);         // return 32 bit depth
    }

    return(0);
}

/*****************************************************************************
*                                                                            *
*                        Convert Image 8 to 32 Bit                           *
*                                                                            *
*****************************************************************************/
FLATIMAGE_32Bit *image8Bit_to_32Bit(FLATIMAGE_8Bit *flat_image_ptr_8Bit)
{
    FLATIMAGE_32Bit *flat_image_ptr_32Bit;
    unsigned long flat_image_size, pixel_nr, color;
    unsigned char PixelValue;

    if(flat_image_ptr_8Bit == 0) return(0);

    flat_image_size = flat_image_ptr_8Bit->width * flat_image_ptr_8Bit->height;
    if((flat_image_ptr_32Bit =
                malloc(2 * sizeof(long) + flat_image_size * sizeof(long))) == 0)
        return(0);

    flat_image_ptr_32Bit->width = flat_image_ptr_8Bit->width;
    flat_image_ptr_32Bit->height = flat_image_ptr_8Bit->height;

    for(color = 0; color < 256; color++) { 
        flat_image_ptr_8Bit->palette[color][0] =
            flat_image_ptr_8Bit->palette[color][0] * 4;
        flat_image_ptr_8Bit->palette[color][1] =
            flat_image_ptr_8Bit->palette[color][1] * 4;
        flat_image_ptr_8Bit->palette[color][2] =
            flat_image_ptr_8Bit->palette[color][2] * 4;
    }

    for(pixel_nr = 0; pixel_nr < flat_image_size; pixel_nr++) {
        PixelValue = ((char*)&flat_image_ptr_8Bit->image)[pixel_nr];
        ((char*)&flat_image_ptr_32Bit->image)[pixel_nr*4] =
            (flat_image_ptr_8Bit->palette)[PixelValue][2];
        ((char*)&flat_image_ptr_32Bit->image)[pixel_nr*4+1] =
            (flat_image_ptr_8Bit->palette)[PixelValue][1];
        ((char*)&flat_image_ptr_32Bit->image)[pixel_nr*4+2] =
            (flat_image_ptr_8Bit->palette)[PixelValue][0];
    }
    return(flat_image_ptr_32Bit);
}

/*****************************************************************************
*                                                                            *
*                        Convert Image 32 to 16 Bit                          *
*                                                                            *
*****************************************************************************/
FLATIMAGE_16Bit *image32Bit_to_16Bit(FLATIMAGE_32Bit *flat_image_ptr_32Bit)
{
    FLATIMAGE_16Bit *flat_image_ptr_16Bit;
    unsigned long flat_image_size, pixel_nr;

    if(flat_image_ptr_32Bit == 0) return(0);

    flat_image_size = flat_image_ptr_32Bit->width * flat_image_ptr_32Bit->height;
    if((flat_image_ptr_16Bit =
                malloc(2 * sizeof(long) + flat_image_size * sizeof(short))) == 0)
        return(0);

    flat_image_ptr_16Bit->width = flat_image_ptr_32Bit->width;
    flat_image_ptr_16Bit->height = flat_image_ptr_32Bit->height;

    for(pixel_nr = 0; pixel_nr < flat_image_size; pixel_nr++) {
        (&flat_image_ptr_16Bit->image)[pixel_nr] =
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 2] << 8) & 0xF800) +
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 1] << 3) & 0x07E0) +
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 0] >> 3) & 0x001F);
    }

    return(flat_image_ptr_16Bit);
}

/*****************************************************************************
*                                                                            *
*                        Convert Image 32 to 15 Bit                          *
*                                                                            *
*****************************************************************************/
FLATIMAGE_16Bit *image32Bit_to_15Bit(FLATIMAGE_32Bit *flat_image_ptr_32Bit)
{
    FLATIMAGE_16Bit *flat_image_ptr_15Bit;
    unsigned long flat_image_size, pixel_nr;

    if(flat_image_ptr_32Bit == 0) return(0);

    flat_image_size = flat_image_ptr_32Bit->width * flat_image_ptr_32Bit->height;
    if((flat_image_ptr_15Bit =
                malloc(2 * sizeof(long) + flat_image_size * sizeof(short))) == 0)
        return(0);

    flat_image_ptr_15Bit->width = flat_image_ptr_32Bit->width;
    flat_image_ptr_15Bit->height = flat_image_ptr_32Bit->height;

    for(pixel_nr = 0; pixel_nr < flat_image_size; pixel_nr++) {
        (&flat_image_ptr_15Bit->image)[pixel_nr] =
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 2] << 7) & 0x7C00) +
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 1] << 2) & 0x03E0) +
            ((((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 0] >> 3) & 0x001F);
    }

    return(flat_image_ptr_15Bit);
}

/*****************************************************************************
*                                                                            *
*                        Convert Image 32 to 8 Bit                           *
*                                                                            *
*****************************************************************************/
FLATIMAGE *image32Bit_to_8Bit(FLATIMAGE_32Bit *flat_image_ptr_32Bit)
{
    FLATIMAGE *flat_image_ptr_8Bit;
    unsigned long flat_image_size, pixel_nr;
    unsigned short red, green, blue;

    if(flat_image_ptr_32Bit == 0) return(0);

    flat_image_size = flat_image_ptr_32Bit->width * flat_image_ptr_32Bit->height;
    if((flat_image_ptr_8Bit =
                malloc(2 * sizeof(long) + flat_image_size)) == 0)
        return(0);

    flat_image_ptr_8Bit->width = flat_image_ptr_32Bit->width;
    flat_image_ptr_8Bit->height = flat_image_ptr_32Bit->height;

    for(pixel_nr = 0; pixel_nr < flat_image_size; pixel_nr++) {
        red =
            (((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 2] >> 5) & 0x7;
        green =
            (((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 1] >> 5) & 0x7;
        blue =
            (((char*)&flat_image_ptr_32Bit->image)[pixel_nr * 4 + 0] >> 6) & 0x3;

        (&flat_image_ptr_8Bit->image)[pixel_nr] = (red * 32 + green * 4 + blue);
    }

    return(flat_image_ptr_8Bit);
}
