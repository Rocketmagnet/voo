#ifndef _image_loader
#define _image_loader

#include "wx/glcanvas.h"

GLuint* LoadImage(wxString path, int* imageWidth, int* imageHeight, int* textureWidth, int* textureHeight);

#endif
