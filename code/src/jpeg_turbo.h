#ifndef JPEG_TURBO_H_INCLUDED
#define JPEG_TURBO_H_INCLUDED

int ReadJpegHeader(char* filename, int* width, int* height);
int JpegRead(char* buffer);


#endif
