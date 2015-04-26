#ifndef __PCX_H_INCLUDED
#define __PCX_H_INCLUDED

// PCX header structure
struct PcxHeader {
    unsigned char  manufacturer;
    unsigned char  pcx_version;
    unsigned char  pcx_encoding;
    unsigned char  pcx_bits_per_pixel;
    /* window dimensions */
    unsigned short pcx_xmin;
    unsigned short pcx_ymin;
    unsigned short pcx_xmax;
    unsigned short pcx_ymax;
    /* non vital info    */
    unsigned short pcx_hor_ppi;
    unsigned short pcx_ver_dpi;
    unsigned char  pcx_color_map[48];
    unsigned char  reserved;
    unsigned char  pcx_num_planes;
    unsigned short pcx_bytes_per_line;
    unsigned short pcx_palette_info;
    /* mandatory only for PB.IV and above */
    unsigned short pcx_hor_size;
    unsigned short pcx_ver_size;
    unsigned char  filler[54];
} __attribute__ ((packed));

typedef struct PcxHeader PCXHEADER;

struct FlatImage_8Bit {
    unsigned long     width;
    unsigned long     height;
    unsigned char     palette[256][3];
    unsigned char     image;
} __attribute__ ((packed));

typedef struct FlatImage_8Bit FLATIMAGE_8Bit;

struct FlatImage_32Bit {
    unsigned long     width;
    unsigned long     height;
    unsigned long     image;
} __attribute__ ((packed));

typedef struct FlatImage_32Bit FLATIMAGE_32Bit;

struct FlatImage_16Bit {
    unsigned long     width;
    unsigned long     height;
    unsigned short    image;
} __attribute__ ((packed));

typedef struct FlatImage_16Bit FLATIMAGE_16Bit;

struct FlatImage {
    unsigned long     width;
    unsigned long     height;
    unsigned char     image;
} __attribute__ ((packed));

typedef struct FlatImage FLATIMAGE;

//********************************FUNCTIONS***********************************

PCXHEADER *getPcxInfo(FILE *file_ptr);
FLATIMAGE_8Bit  *loadPcxImage_8Bit(FILE *pcx_file_ptr, PCXHEADER *pcx_header_ptr);
FLATIMAGE_32Bit *loadPcxImage_24Bit(FILE *pcx_file_ptr, PCXHEADER *pcx_header_ptr);
void *loadPcxImage(unsigned char *filename, unsigned char bpp);
FLATIMAGE_32Bit *image8Bit_to_32Bit(FLATIMAGE_8Bit *image_ptr);
FLATIMAGE_16Bit *image32Bit_to_16Bit(FLATIMAGE_32Bit *image_ptr);
FLATIMAGE_16Bit *image32Bit_to_15Bit(FLATIMAGE_32Bit *image_ptr);
FLATIMAGE       *image32Bit_to_8Bit(FLATIMAGE_32Bit *image_ptr);

#endif
