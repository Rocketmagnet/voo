
#include "gl_image.h"
#include "image_loader.h"

GL_Image::GL_Image()
{
}

GL_Image::GL_Image(wxString path)
{
    load(path);
}

void GL_Image::load(wxString path)
{
    ID = LoadImage(path, &width, &height, &textureWidth, &textureHeight);

    tex_coord_x = (float)width / (float)textureWidth;
    tex_coord_y = (float)height / (float)textureHeight;
}

GLuint* GL_Image::getID()
{
    return ID;
}

GL_Image::~GL_Image()
{
    glDeleteTextures(1, ID);
}
