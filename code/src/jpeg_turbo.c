
#include <stdio.h>
#include "jpeglib.h"
#include <setjmp.h>

struct my_error_mgr {
    struct jpeg_error_mgr pub;    /* "public" fields */

    jmp_buf setjmp_buffer;        /* for return to caller */
};


JSAMPLE *image_buffer;          // Points to large array of R,G,B-order data

struct jpeg_decompress_struct cinfo;
struct my_error_mgr           jerr;

FILE       *infile;             // source file
JSAMPARRAY  buffer;             // Output row buffer
int         row_stride;         // physical row width in output buffer


typedef struct my_error_mgr *my_error_ptr;


METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
    printf("my_error_exit %p\n", cinfo->err->output_message);

    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    my_error_ptr myerr = (my_error_ptr)cinfo->err;

    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);

    /* Return control to the setjmp point */
    longjmp(myerr->setjmp_buffer, 1);
}


int ReadJpegHeader(char* filename, int* width, int* height)
{
    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("can't open %s\n", filename);
        return 0;
    }
    else
    {
        printf("opened %s\n", filename);
    }
    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        printf("failed\n");
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        return 0;
    }


    printf("A\n");
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);


    printf("B\n");
    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, infile);


    printf("C\n");
    /* Step 3: read file parameters with jpeg_read_header() */
    (void)jpeg_read_header(&cinfo, TRUE);
    *width  = cinfo.image_width;
    *height = cinfo.image_height;

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

    (void)jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    buffer     = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

    return 1;
}


int JpegRead(char* imageBuffer)
{
    char *dest = imageBuffer;

    printf("JpegRead\n");
    while (cinfo.output_scanline < cinfo.output_height)
    {
        (void)jpeg_read_scanlines(&cinfo, dest, 1);
        dest += row_stride*3;
    }

    (void)jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);

    return 1;
}
