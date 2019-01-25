#ifndef JPEG_TURBO_H_INCLUDED
#define JPEG_TURBO_H_INCLUDED

#include "jpeglib.h"
#include <setjmp.h>

struct my_error_mgr
{
    struct jpeg_error_mgr pub;    /* "public" fields */
    jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct
{
    JSAMPLE *image_buffer;          // Points to large array of R,G,B-order data

    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr           jerr;

    FILE       *infile;             // source file
    JSAMPARRAY  buffer;             // Output row buffer
    int         row_stride;         // physical row width in output buffer
    int         exit_code;
    int         width, height;
    char        file_name[1024];
}jpeg_load_state;


jpeg_load_state* ReadJpegHeader(const char* filename);
jpeg_load_state* ReadJpegHeaderOnly(const char* filename);
int JpegRead(unsigned char* buffer, jpeg_load_state* load_state);
int JpegWrite(const char* filename, int width, int height, unsigned char *data);

#endif
