#ifndef JPEG_TURBO_H_INCLUDED
#define JPEG_TURBO_H_INCLUDED

int ReadJpegHeader(const char* filename, int* width, int* height);
int JpegRead(unsigned char* buffer);

#endif
