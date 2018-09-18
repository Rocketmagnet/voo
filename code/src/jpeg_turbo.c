#include <stdio.h>
#include "jpeg_turbo.h"




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


jpeg_load_state* ReadJpegHeader(const char* filename)
{
	//printf("ReadJpegHeader(%s)\n", filename);

    FILE *infile;
    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("can't open %s\n", filename);
        return 0;
    }
    else
    {
        //printf("OPENED %s\n", filename);
    }

    // If the file was successfully opened, then allocate memory.
    jpeg_load_state* load_state = malloc(sizeof(jpeg_load_state));

    load_state->infile = infile;
    strncpy(load_state->file_name, filename, 1024);
    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    load_state->cinfo.err = jpeg_std_error(&load_state->jerr.pub);
    load_state->jerr.pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(load_state->jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        printf("failed\n");
        jpeg_destroy_decompress(&load_state->cinfo);
        //printf("CLOSE: %s\n", filename);
        fclose(load_state->infile);

        free(load_state);
        return 0;
    }


    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&load_state->cinfo);


    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&load_state->cinfo, load_state->infile);


    /* Step 3: read file parameters with jpeg_read_header() */
    (void)jpeg_read_header(&load_state->cinfo, TRUE);
    load_state->width  = load_state->cinfo.image_width;
    load_state->height = load_state->cinfo.image_height;

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

    (void)jpeg_start_decompress(&load_state->cinfo);

    load_state->row_stride = load_state->cinfo.output_width * load_state->cinfo.output_components;
    int samples_per_row = 1;

    // Allocate a buffer which will be destroyed automatically when the jpeg has finished being read.
    load_state->buffer     = (*load_state->cinfo.mem->alloc_sarray)((j_common_ptr)&load_state->cinfo, JPOOL_IMAGE, load_state->row_stride, samples_per_row);

    return load_state;
}

jpeg_load_state* ReadJpegHeaderOnly(const char* filename)
{
    //printf("ReadJpegHeader(%s)\n", filename);

    FILE *infile;
    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("can't open %s\n", filename);
        return 0;
    }
    else
    {
        //printf("OPENED %s\n", filename);
    }

    // If the file was successfully opened, then allocate memory.
    jpeg_load_state* load_state = malloc(sizeof(jpeg_load_state));

    load_state->infile = infile;
    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    load_state->cinfo.err = jpeg_std_error(&load_state->jerr.pub);
    load_state->jerr.pub.error_exit = my_error_exit;

    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(load_state->jerr.setjmp_buffer))
    {
        /* If we get here, the JPEG code has signaled an error.
        * We need to clean up the JPEG object, close the input file, and return.
        */
        printf("failed\n");
        jpeg_destroy_decompress(&load_state->cinfo);

        //printf("CLOSE: %s\n", filename);
        fclose(load_state->infile);

        free(load_state);
        return 0;
    }


    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&load_state->cinfo);


    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&load_state->cinfo, load_state->infile);


    /* Step 3: read file parameters with jpeg_read_header() */
    (void)jpeg_read_header(&load_state->cinfo, TRUE);
    load_state->width  = load_state->cinfo.image_width;
    load_state->height = load_state->cinfo.image_height;

    jpeg_destroy_decompress(&load_state->cinfo);
    //printf("CLOSE: %s\n", filename);
    fclose(load_state->infile);
    free(load_state);
    return load_state;
}

int JpegRead(unsigned char* imageBuffer, jpeg_load_state* load_state)
{
    //printf("JpegRead\n");
    //row_stride = cinfo.output_width * cinfo.output_components;
	//buffer     = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);
	//outputFile = fopen("output2.raw", "wb");

    char *dest = imageBuffer;
	//JSAMPARRAY scanLine = &dest;

    while (load_state->cinfo.output_scanline < load_state->cinfo.output_height)
    {
        //(void)jpeg_read_scanlines(&cinfo, scanLine, 1);
		(void)jpeg_read_scanlines(&load_state->cinfo, load_state->buffer, 1);

		//printf("  Copy %p to %p   bytes:%d\n", buffer[0], dest, row_stride);
		memcpy(dest, load_state->buffer[0], load_state->row_stride);
		//put_scanline_someplace(buffer[0], row_stride);
		dest += load_state->row_stride;
    }

    (void)jpeg_finish_decompress(&load_state->cinfo);
    jpeg_destroy_decompress(&load_state->cinfo);
    //printf("CLOSE: %s\n", load_state->file_name);
    fclose(load_state->infile);
    //fclose(outputFile);

    free(load_state);
	return 1;
}
