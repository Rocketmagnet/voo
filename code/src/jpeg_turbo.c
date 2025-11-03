#include <stdio.h>
#include "jpeg_turbo.h"
#include <string.h>



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


int ReadJpegHeader(jpeg_load_state* load_state, const char* filename)
{
    FILE *infile;

    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("ReadJpegHeader(): can't open [%s]\n", filename);
        return 0;
    }
    else
    {
        //printf("OPENED %s\n", filename);
    }


    // If the file was successfully opened, then allocate memory.
    //jpeg_load_state* load_state = malloc(sizeof(jpeg_load_state));

    load_state->cinfo.mem = 0;
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

        //free(load_state);
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
    if (load_state->cinfo.mem == 0)
        printf("WARNING load_state->cinfo.mem == 0\n");
    else
        load_state->buffer     = (*load_state->cinfo.mem->alloc_sarray)((j_common_ptr)&load_state->cinfo, JPOOL_IMAGE, load_state->row_stride, samples_per_row);

	return 1;
}



int ReadJpegHeaderOnly(jpeg_load_state *load_state, const char* filename)
{
    FILE *infile;
    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("ReadJpegHeaderOnly(): can't open [%s]\n", filename);
        return 0;
    }
    else
    {
        //printf("OPENED %s\n", filename);
    }

    // If the file was successfully opened, then allocate memory.
    //jpeg_load_state* load_state = malloc(sizeof(jpeg_load_state));

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

        //free(load_state);
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
    //free(load_state);
    return 1;
}

void ConvertGreyscaleToRGB(int w, int h, unsigned char *data)
{
    int lastPixel = w * h;

    unsigned char* src = data + lastPixel;
    unsigned char* dst = data + (lastPixel * 3);

    for (int i = lastPixel - 1; i >= 0; i--)
    {
        unsigned char grey = *src--;
        *dst-- = grey;
        *dst-- = grey;
        *dst-- = grey;
    }
}

int JpegRead(unsigned char* imageBuffer, jpeg_load_state* load_state)
{
    //printf("JpegRead(%p, %p\n", imageBuffer, load_state);

    if (!imageBuffer)
    {
        printf("  No image buffer\n");
        return 0;
    }

    if (!load_state)
    {
        printf("  No load_state\n");
        return 0;
    }


    char *dest = imageBuffer;

    while (load_state->cinfo.output_scanline < load_state->cinfo.output_height)
    {
		(void)jpeg_read_scanlines(&load_state->cinfo, load_state->buffer, 1);
        
		memcpy(dest, load_state->buffer[0], load_state->row_stride);
		dest += load_state->row_stride;
    }

    if (load_state->cinfo.num_components == 1)
        ConvertGreyscaleToRGB(load_state->width, load_state->height, imageBuffer);

    (void)jpeg_finish_decompress(&load_state->cinfo);
    jpeg_destroy_decompress(&load_state->cinfo);
    fclose(load_state->infile);

	return 1;
}


int JpegWrite(const char* filename, int width, int height, unsigned char *data, int quality)
{
    //printf("JpegWrite(%s, %d, %d, , %d)\n", filename, width, height, quality);
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    JOCTET *icc_profile = NULL;
    long icc_len = 0;
    FILE *output_file = NULL;
    unsigned char *outbuffer = NULL;
    unsigned long outsize = 0;

    output_file = fopen(filename, "wb");     // Specify the destination for the compressed data (eg, a file)

    if (!output_file)
    {
        printf("File Error\n");
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);      // Allocate and initialize a JPEG compression object
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, output_file);

    cinfo.image_width      = width;         // Set parameters for compression, including image size & colorspace
    cinfo.image_height     = height;
    cinfo.input_components = 3;
    cinfo.in_color_space   = JCS_RGB;         // Arbitrary guess
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, TRUE);
    jpeg_start_compress(&cinfo, TRUE);
     
    JSAMPROW row_pointer;          /* pointer to a single row */

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer = (JSAMPROW)&data[cinfo.next_scanline*3*width];
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);

    // Release the JPEG compression object
    jpeg_destroy_compress(&cinfo);
    fclose(output_file);

    return 0;
}


int ChooseBestScaleDenom(struct jpeg_decompress_struct* cinfo, int targetSize)
{
    //printf("  ChooseBestScaleDenom(%d)\n", targetSize);

    int candidates[] = { 8, 4, 2, 1 };

    for (int i = 0; i < 4; ++i)
    {
        cinfo->scale_num   = 1;
        cinfo->scale_denom = candidates[i];

        jpeg_calc_output_dimensions(cinfo);             // compute output dimensions for this scale
        //printf("    %d: (%d, %d)", cinfo->scale_denom, cinfo->output_width, cinfo->output_height);

        if (cinfo->output_width >= targetSize || cinfo->output_height >= targetSize)
        {
            //printf(" - YES!\n");
            return candidates[i];                       // largest denom that still >= target
        }
        //printf("\n");
    }
    //printf("  failed :(\n");
    return 1;                                           // fallback to full-size decode (shouldn't reach often)
}

/*
int FetchThumbnailReadHeader(jpeg_load_state* load_state, const char* filename, int maxSize)
{
    printf("FetchThumbnailReadHeader(");
    printf(filename);
    printf(", %d)\n", maxSize);

    FILE* infile = fopen(filename, "rb");
    if (!infile)
    {
        perror("Error opening input file");
        return 0;
    }

    load_state->infile = infile;
    // --- Decompression setup ---
    //struct jpeg_decompress_struct   dinfo;
    //struct jpeg_error_mgr           jerr;

    //load_state->cinfo.err = jpeg_std_error(&jerr);

    load_state->cinfo.err = jpeg_std_error(&load_state->jerr.pub);
    load_state->jerr.pub.error_exit = my_error_exit;

    if (setjmp(load_state->jerr.setjmp_buffer))
    {
        // If we get here, the JPEG code has signaled an error.
        // We need to clean up the JPEG object, close the input file, and return.

        printf("failed\n");
        jpeg_destroy_decompress(&load_state->cinfo);

        //printf("CLOSE: %s\n", filename);
        fclose(load_state->infile);

        //free(load_state);
        return 0;
    }

    jpeg_create_decompress(&load_state->cinfo);
    jpeg_stdio_src(&load_state->cinfo, infile);
    jpeg_read_header(&load_state->cinfo, TRUE);

    // Request reduced size decoding (fast DCT-based scaling)
    load_state->cinfo.scale_num   = 1;
    load_state->cinfo.scale_denom = ChooseBestScaleDenom(&load_state->cinfo, maxSize);

    jpeg_destroy_decompress(&load_state->cinfo);
    //printf("CLOSE: %s\n", filename);
    fclose(load_state->infile);
    //free(load_state);
    // Start decompression
    //jpeg_start_decompress(&load_state->cinfo);

    load_state->width   = load_state->cinfo.output_width;
    load_state->height  = load_state->cinfo.output_height;
    load_state->scaling = load_state->cinfo.scale_denom;
    int pixel_size      = load_state->cinfo.output_components;

    return 1;
}
*/

int FetchThumbnailReadHeader(jpeg_load_state* load_state, const char* filename, int maxSize)
{
    FILE* infile;

    if ((infile = fopen(filename, "rb")) == NULL)
    {
        printf("ReadJpegHeader(): can't open [%s]\n", filename);
        return 0;
    }
    else
    {
        //printf("OPENED %s\n", filename);
    }


    // If the file was successfully opened, then allocate memory.
    //jpeg_load_state* load_state = malloc(sizeof(jpeg_load_state));

    load_state->cinfo.mem = 0;
    load_state->infile = infile;
    strncpy(load_state->file_name, filename, 1024);
    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    load_state->cinfo.err = jpeg_std_error(&load_state->jerr.pub);
    load_state->jerr.pub.error_exit = my_error_exit;

    jpeg_create_decompress(&load_state->cinfo);
    jpeg_stdio_src(&load_state->cinfo, infile);
    jpeg_read_header(&load_state->cinfo, TRUE);
    load_state->cinfo.scale_denom = 8; //ChooseBestScaleDenom(&load_state->cinfo, maxSize);
    jpeg_calc_output_dimensions(&load_state->cinfo);

    //printf("Scale chosen: %d\n", load_state->cinfo.scale_denom);

    //return 0;

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

        //free(load_state);
        return 0;
    }

    load_state->width  = load_state->cinfo.output_width;
    load_state->height = load_state->cinfo.output_height;

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

    (void)jpeg_start_decompress(&load_state->cinfo);

    load_state->row_stride = load_state->cinfo.output_width * load_state->cinfo.output_components;
    int samples_per_row = 1;

    // Allocate a buffer which will be destroyed automatically when the jpeg has finished being read.
    if (load_state->cinfo.mem == 0)
        printf("WARNING load_state->cinfo.mem == 0\n");
    else
        load_state->buffer = (*load_state->cinfo.mem->alloc_sarray)((j_common_ptr)&load_state->cinfo, JPOOL_IMAGE, load_state->row_stride, samples_per_row);

    return 1;
}


/*
int FetchThumbnailReadHeader(jpeg_load_state* load_state, unsigned char* data)
{
    if (!data)
    {
        fprintf(stderr, "Out of memory\n");
        jpeg_destroy_decompress(&load_state->cinfo);
        fclose(load_state->infile);
        return 1;
    }
}
*/
