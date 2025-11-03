#include <gl/glew.h>

#include <iostream>
using namespace std;

//#include <GL/gl.h>
#include <wx/glcanvas.h>
#include "wx/wx.h"
#include "wx/image.h"
#include "status_bar.h"
#include "file_name_list.h"
#include "wx/time.h"
#include "wx/filename.h"

#include "drawable.h"
#include "gl_panel.h"

//extern "C"
//{
//    #include "jpeg_turbo.h"
//};

extern void NoteTime(wxString s);
void SetDebuggingText(wxString text);

#define OVERLAP 10
#define UNDERLAP 4

void PrintOpenGLVersion()
{
    const char* version = (const char*)glGetString(GL_VERSION);
    if (version)
        std::cout << "OpenGL Version: " << version << std::endl;
    else
        std::cerr << "Failed to retrieve OpenGL version!" << std::endl;
}

std::ostream & operator << (std::ostream & os, const RectangleVector & rv)
{
    return os << rv.TL << rv.BR;
}


wxThread::ExitCode ImageLoader::Entry()
{
    SetPriority(wxPRIORITY_MIN);
	glImage.Load(fileName);
    return 0;
}


void ZoomAway(float &point, float centre, float amount)
{
    point += (point - centre) * amount;
}


GL_Image::GL_Image()
: basicGLPanel(0)
{
    //cout << this << "  GL_Image::GL_Image()" << endl;

    x     = 0.0;
    y     = 0.0;
    scale = 1.0;
}


GL_Image::~GL_Image()
{
    //cout << "Destructor deleting texture " << ID << endl;
    //glDeleteTextures(4, ID);
}

bool GL_Image::IsFullyVisible() const
{
    //cout << scale << endl;
    //cout << scale * width << " " << basicGLPanel->GetWidth() << endl;
    //cout << scale * height << " " << basicGLPanel->GetHeight() << endl;

    if (scale*width > basicGLPanel->GetWidth())
        return false;

    if (scale*height > basicGLPanel->GetHeight())
        return false;

    return true;
}

double GL_Image::VisibilityAtScale(double sc) const
{
    double screenAspect = (double)basicGLPanel->GetWidth() / (double)basicGLPanel->GetHeight();
    double screenWidth  = (double)basicGLPanel->GetWidth();
    double screenHeight = (double)basicGLPanel->GetHeight();
    double imageAspect  = (double)width / (double)height;

    double scaleWidth   = width  * sc;
    double scaleHeight  = height * sc;

    double visWidth  = min(scaleWidth,  screenWidth);
    double visHeight = min(scaleHeight, screenHeight);
    double visArea = visWidth * visHeight;
    double scaleArea = scaleWidth * scaleHeight;

    return visArea / scaleArea;
}


// Try to zoom this image to seem like a similar amount of
// zoom as the previous image.
// Determine how much of the previous image is visible, and zoom this image
// so that about the same amount of the image is visible.
void GL_Image::CopyVisibilityFrom(const GL_Image &image)
{
    double  scaleLinear;     // scale above which the visibility changes as 1/(scale  )  (kind of)
    double  scaleSquare;     // scale above which the visibility changes as 1/(scale^2)  (kind of)
    double  visSquare;       // Visibility at scale == scaleSquare

    double  prevVisArea = image.VisibilityAtScale(image.scale);  // We want to achieve this much visibility

    double  screenAspect = (double)basicGLPanel->GetWidth() / (double)basicGLPanel->GetHeight();
    double  imageAspect = (double)width / (double)height;
    double  screenWidth = basicGLPanel->GetWidth();
    double  screenHeight = basicGLPanel->GetHeight();
    double  dI;
    double  dS;

    if (imageAspect > screenAspect)     // If image is more WideScreen than the screen
    {                                   // then touch the sides, and leave gaps at the top and bottom
        scaleLinear = screenWidth / width;
        scaleSquare = screenHeight / height;
        dI          = width;
        dS          = screenWidth;
        visSquare   = screenWidth / (width*scaleSquare);    // When the image is scaled to fit the height of the screen,
                                                            // this much of the image is visible.
    }
    else                                // If screen is more Widescreen than the image
    {
        scaleLinear = screenHeight / height;
        scaleSquare = screenWidth / width;
        dI          = height;
        dS          = screenHeight;
        visSquare   = screenHeight / (height*scaleSquare);  // When the image is scaled to fit the width of the screen,
                                                            // this much of the image is visible.
    }

    // Now, do we need to have both width and height bigger than the screen, or just one of them?
    if (prevVisArea < visSquare)
    {                                   // Full zoom
        scale = sqrt((screenHeight*screenWidth) / (prevVisArea*height*width));
    }
    else
    {                                   // Half zoom
        scale = dS / (prevVisArea*height);
    }
}


void GL_Image::CopyPositionFrom(const GL_Image &image)
{
    double prevWidth    = image.width  * image.scale;
    double prevHeight   = image.height * image.scale;
    double thisWidth    =       width  *       scale;
    double thisHeight   =       height *       scale;
    double screenWidth  = basicGLPanel->GetWidth();
    double screenHeight = basicGLPanel->GetHeight();
    double prevXPan = image.x / prevWidth;
    double prevYPan = image.y / prevHeight;

    double xPan = 0.0;
    double yPan = 0.0;

    x = prevXPan * thisWidth;
    y = prevYPan * thisHeight;

    //if (prevWidth  > screenWidth )  { xPan = (prevWidth  - screenWidth ) / prevWidth;  }
    //if (prevHeight > screenHeight)  { yPan = (prevHeight - screenHeight) / prevHeight; }


}

void GL_Image::CopyScaleAndPositionFrom(const GL_Image &image)
{
    if (image.IsFullyVisible())
    {
        //std::cout << "Fully visible" << std::endl;
        scale = 0.01f;
        x = 0;
        y = 0;
        ExpandToSides();
    }
    else
    {
        //std::cout << "Not fully visible" << std::endl;
        //std::cout << th5is << "  Copying scale " << image.scale << "  (" << x << ", " << y << ")  from " << &image << std::endl;
        scale = image.scale;
        x = image.x;
        y = image.y;
        CopyVisibilityFrom(image);
        CopyPositionFrom(image);
    }
}


void GL_Image::ExpandToSides()
{
    double screenAspect = (double)basicGLPanel->GetWidth() / (double)basicGLPanel->GetHeight();
    double imageAspect  = (double)width / (double)height;
    double screenWidth  = basicGLPanel->GetWidth();
    double screenHeight = basicGLPanel->GetHeight();

    //cout << "GL_Image::ExpandToSides()" << endl;
    //cout << "screenWidth  = " << screenWidth  << endl;
    //cout << "screenHeight = " << screenHeight << endl;
    //cout << "x      = " << x << endl;
    //cout << "y      = " << y << endl;
    //cout << "width  = " << width << endl;
    //cout << "height = " << height << endl;
    //cout << "scale  = " << scale << endl;
    //cout << "imageAspect = " << imageAspect << endl;

    if (imageAspect > screenAspect)     // If image is more WideScreen than the screen
    {                                   // then touch the sides, and leave gaps at the top and bottom
        if (width*scale < screenWidth)
        {
            x     = 0;
            scale = screenWidth / width;
            y     = 0;
        }
    }
    else                                // If screen is more Widescreen than the image
    {
        if (height*scale < screenHeight)
        {
            y     = 0;
            scale = screenHeight / height;
            x     = 0;
            //cout << "x      = " << x << endl;
            //cout << "y      = " << y << endl;
            //cout << "scale  = " << scale << endl;
        }
    }

    //cout << "x      = " << x << endl;
    //cout << "y      = " << y << endl;
    //cout << "width  = " << width << endl;
    //cout << "height = " << height << endl;
    //cout << "scale  = " << scale << endl;
}

void GL_Image::ClampToSides()
{
    double left  = x - width*scale*0.5;
    double right = x + width*scale*0.5;

    double top    = y - height*scale*0.5;
    double bottom = y + height*scale*0.5;

    double screenWidth  = basicGLPanel->GetWidth();
    double screenHeight = basicGLPanel->GetHeight();

    double hScreenWidth  = screenWidth  * 0.5;
    double hScreenHeight = screenHeight * 0.5;

    double  leftGap = hScreenWidth + left;         // positive if image doesn't overlap left side of screen
    double rightGap = right - hScreenWidth;         // positive if image DOES    overlap left side of screen

    double    topGap = hScreenHeight + top;
    double bottomGap = bottom - hScreenHeight;

    if (width*scale >= screenWidth)           // If image is wider than the screen
    {
        if (leftGap > 0.0)              //     If there's a space on the LEFT hand side
        {
            x -= leftGap;               //     .. then move it so it touches the left hand side
        }

        if (rightGap < 0.0)             //     If there's a space on the RIGHT hand side
        {
            x -= rightGap;    //     .. then move it so it touches the right hand side
        }
    }
    else                                    // If image is narrower than the screen
    {
        if (leftGap < 0.0)                     // If image is off the left side of the screen
        {
            x -= leftGap;
        }

        if (rightGap > 0.0)
        {
            x -= rightGap;
        }

    }


    if (height*scale >= screenHeight)           // If image is taller than the screen
    {
        if (topGap > 0.0)              //     If there's a space on the LEFT hand side
        {
            y -= topGap;               //     .. then move it so it touches the left hand side
        }

        if (bottomGap < 0.0)             //     If there's a space on the RIGHT hand side
        {
            y -= bottomGap;    //     .. then move it so it touches the right hand side
        }
    }
    else                                    // If image is narrower than the screen
    {
        if (topGap < 0.0)                     // If image is off the left side of the screen
        {
            y -= topGap;
        }

        if (bottomGap > 0.0)
        {
            y -= bottomGap;
        }
    }
}
    
double Remap(double i, double sMin, double sMax, double dMin, double dMax)
{
    return (i - sMin) * ((dMax - dMin) / (sMax - sMin)) + (dMin);
}

void TextureUpload::Render(Vector2D scrTL, Vector2D scrBR)
{
    //PrintOpenGLVersion();
    //cout << "TextureUpload::Render()" << endl;
    if (!valid)
    {
        //cout << "!valid" << endl;
        return;
    }

    //myTexturePortion;
    

    float x0 = Remap(originalImagePortion.TL.x, 0, wxImg->GetSize().x, scrTL.x, scrBR.x);
    float x1 = Remap(originalImagePortion.BR.x, 0, wxImg->GetSize().x, scrTL.x, scrBR.x);
    float y0 = Remap(originalImagePortion.TL.y, 0, wxImg->GetSize().y, scrTL.y, scrBR.y);
    float y1 = Remap(originalImagePortion.BR.y, 0, wxImg->GetSize().y, scrTL.y, scrBR.y);

    //if (clips & CLIP_LEFT )
    //    x0 = Remap(originalImagePortion.TL.x + OVERLAP, 0, wxImg->GetSize().x, scrTL.x, scrBR.x);
    //else
    //    x1 = Remap(originalImagePortion.TL.x + 2, 0, wxImg->GetSize().x, scrTL.x, scrBR.x);

    //if (clips & CLIP_TOP  )        y0 = Remap(originalImagePortion.TL.y + OVERLAP, 0, wxImg->GetSize().y, scrTL.y, scrBR.y);

    //if (clips & CLIP_LEFT)
    //    y1 -= 100;
    //else
    //    y0 += 100;

    //cout << renderableTexturePortion << endl;

    glBindTexture(GL_TEXTURE_2D, ID);

    glBegin(GL_QUADS);
        glTexCoord2f(renderableTexturePortion.TL.x, renderableTexturePortion.TL.y);    glVertex2f(x0, y0);
        glTexCoord2f(renderableTexturePortion.BR.x, renderableTexturePortion.TL.y);    glVertex2f(x1, y0);
        glTexCoord2f(renderableTexturePortion.BR.x, renderableTexturePortion.BR.y);    glVertex2f(x1, y1);
        glTexCoord2f(renderableTexturePortion.TL.x, renderableTexturePortion.BR.y);    glVertex2f(x0, y1);

        //glTexCoord2f(myTexturePortion.TL.x, myTexturePortion.TL.y);    glVertex2f(x0, y0);
    //glTexCoord2f(myTexturePortion.BR.x, myTexturePortion.TL.y);    glVertex2f(x1, y0);
    //glTexCoord2f(myTexturePortion.BR.x, myTexturePortion.BR.y);    glVertex2f(x1, y1);
    //glTexCoord2f(myTexturePortion.TL.x, myTexturePortion.BR.y);    glVertex2f(x0, y1);
    //glColor3f(0.1, 0.00, 0);    glVertex2f(x0, y0);
        //glColor3f(0.1, 0.25, 0);    glVertex2f(x1, y0);
        //glColor3f(0.1, 0.50, 0);    glVertex2f(x1, y1);
        //glColor3f(0.1, 0.25, 0);    glVertex2f(x0, y1);
    glEnd();

}

void GL_Image::Render()
{
    //NoteTime(wxT("GL_Image::Render"));
    //cout << endl << endl << "GL_Image::Render()" << endl;

    if (!loadedImage)
    {
        //cout << "!loadedImage" << endl;
        return;
    }

    if (!uploadedTexture)
    {
        //cout << "!uploadedTexture" << endl;
        //uploadedTexture = true;
        for (int i = 0; i < 4; i++)
        {
            if (!textureUploads[i].UploadNextBlock())
            {
                //cout << "uploadedTexture = false  2" << endl;
                uploadedTexture = false;
            }
        }
    }
    glLoadIdentity();


    double scrLeft   = x - (width  * scale * 0.5);
    double scrRight  = x + (width  * scale * 0.5);
    double scrTop    = y - (height * scale * 0.5);
    double scrBottom = y + (height * scale * 0.5);
    //cout << "WxH = " << width << " x " << height << endl;
    //cout << "Rendering (" << scrLeft << ", " << scrTop << ") - (" << scrRight << ", " << scrBottom << ")" << endl;
    float screenWidth = basicGLPanel->GetWidth();
    float screenHeight = basicGLPanel->GetHeight();

    glTranslatef(screenWidth * 0.5, screenHeight * 0.5, 0);

    textureUploads[0].Render(Vector2D(scrLeft, scrTop), Vector2D(scrRight, scrBottom));
    //glTranslatef(screenWidth * 0.05, screenHeight * 0.05, 0);
    textureUploads[1].Render(Vector2D(scrLeft, scrTop), Vector2D(scrRight, scrBottom));
    //for (int i = 0; i < 2; i++)
    //{
    //    textureUploads[i].Render(Vector2D(scrLeft, scrTop), Vector2D(scrRight, scrBottom));
    //}
}

void GL_Image::CalculateTextureSizes()
{
    int   auxWidth  = 0;
    int   auxHeight = 0;

    int width0 = width;
    int width1 = 0;

    int height0 = height;
    int height1 = 0;

    if (width > MAX_WIDTH)
    {
        width0 = MAX_WIDTH;
        width1 = width - width0;
    }

    if (height > MAX_HEIGHT)
    {
        height0 = MAX_HEIGHT;
        height1 = height - height0;
    }

    //cout << width0 << ", " << height0 << ", " << width << ", " << height << endl;

    textureUploads[0].Init(&wxImg, Vector2D(     0,  0), Vector2D(width0, height0), OVERLAP_RIGHT );
    textureUploads[1].Init(&wxImg, Vector2D(width0,  0), Vector2D(width , height0), OVERLAP_LEFT  );

    //textureUploads[2].Init(&wxImg, Vector2D(     0, height0), Vector2D(width0, height ), CLIP_TOP           );
    //textureUploads[3].Init(&wxImg, Vector2D(width0, height0), Vector2D(width , height ), CLIP_TOP|CLIP_LEFT );

    //textureUploads[0].Init(&wxImg, Vector2D(0, 0), Vector2D(width0, height0));
    //textureUploads[1].Init(&wxImg, Vector2D(width0 - 1, 0), Vector2D(width + 1, height0));
    //textureUploads[2].Init(&wxImg, Vector2D(0, height0 - 1), Vector2D(width0, height + 1));
    //textureUploads[3].Init(&wxImg, Vector2D(width0 - 1, height0 - 1), Vector2D(width + 1, height + 1));

    //textureSizes[0] = wxSize(width0, height0);
    //textureSizes[1] = wxSize(width1, height0);
    //textureSizes[2] = wxSize(width0, height1);
    //textureSizes[3] = wxSize(width1, height1);
}

TextureUpload::~TextureUpload()
{
    if (hasGeneratedTexture)
    {
        //cout << "Deleting texture " << ID << endl;
        glDeleteTextures(1, &ID);
    }
}



void TextureUpload::Init(wxImage *img, Vector2D TL, Vector2D BR, int overlap)
{
    originalImagePortion = RectangleVector(TL, BR);
    expandedImagePortion = RectangleVector(TL, BR);

    if ((originalImagePortion.XSize() == 0) || (originalImagePortion.YSize() == 0))
    {
        valid = false;
        return;
    }
    else
    {
        valid = true;
    }

    if (overlap & OVERLAP_LEFT)
    {
          expandedImagePortion.TL.x -= OVERLAP;
    }

    if (overlap & OVERLAP_RIGHT)
    {
          expandedImagePortion.BR.x += OVERLAP;
    }


    int width  = expandedImagePortion.XSize();
    int height = expandedImagePortion.YSize();

    copyWidth  = width;

    float power_of_two_that_gives_correct_width = std::log((float)(width - 1)) / std::log(2.0);
    float power_of_two_that_gives_correct_height = std::log((float)(height - 1)) / std::log(2.0);
    int textureWidth = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)));
    int textureHeight = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)));

    textureSize = RectangleVector(textureWidth, textureHeight);

    wxImg = img;

    double top = 0;
    double left = 0;


    allocatedTexturePortion = RectangleVector( Vector2D(0,0), textureSize.GetFraction(expandedImagePortion.Size()) );

    renderableTexturePortion = originalImagePortion;
    renderableTexturePortion.Subtract(expandedImagePortion.TL);

    renderableTexturePortion = RectangleVector(textureSize.GetFraction(renderableTexturePortion.TL),
                                               textureSize.GetFraction(renderableTexturePortion.BR)  );

    uploadedTexture = false;
    
    currentY = 0;
}

/*
void BlackTexture::CreateTexture()
{
    GLchar blackPixels[8 * 8 * 3] = { 0 };

    glGenTextures(1, &ID);
    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 8, 0, GL_RGB, GL_UNSIGNED_BYTE, blackPixels);    // Fill the texture with black pixels
}
*/

void TextureUpload::CreateTexture(size_t width, size_t height)
{
    /*
    BlackTexture blackTexture;

    GLuint blackTextureID = blackTexture.GetId();

    glGenTextures(1, &ID);
    hasGeneratedTexture = true;

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    GLfloat clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f }; // RGBA all zero

    GLuint FramebufferName = 0;
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);                                 // Create a framebuffer so we can render to the texture

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ID, 0);

    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    glViewport(0, 0, width, height);

    glBegin(GL_QUADS);
        glColor3f(0, 0, 0);
        glVertex2f(    0,      0);
        glVertex2f(width,      0);
        glVertex2f(width, height);
        glVertex2f(    0, height);
    glEnd();
    */
}

GLuint createBlackTextureQuad(int width, int height)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    //glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

    // Set viewport to texture size
    glViewport(0, 0, width, height);

    
    // Create a simple quad (two triangles)
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0,
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Simple black shader program
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main() {\n"
        "   gl_Position = vec4(aPos, 1.0);\n"
        "}\0";

    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(0.0, 0.0, 0.2, 1.0);\n" // Black color
        "}\0";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Render the quad
    glUseProgram(shaderProgram);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Cleanup
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(shaderProgram);

    
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind FBO
    glViewport(0, 0, 1024, 768); // Reset viewport

    //Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(0);
    return textureID;
}


bool TextureUpload::UploadNextBlock()
{
    if (!wxImg)
    {
        return true;
    }

    if (uploadedTexture)
    {
        return true;
    }

    if (!valid)
    {
        return true;
    }

    int imageWidth  = wxImg->GetSize().x;
    int imageHeight = wxImg->GetSize().y;

    if (currentY == 0)
    {
        currentY = expandedImagePortion.TL.y;
        if (hasGeneratedTexture)
        {
            hasGeneratedTexture = false;
            glDeleteTextures(1, &ID);
        }

        ID = createBlackTextureQuad(textureSize.BR.x, textureSize.BR.y);

        hasGeneratedTexture = true;
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    wxSize imageSize = wxImg->GetSize();
    size_t bytes = imageSize.GetWidth() * imageSize.GetHeight() * 3;

    // For some reason, some rare images crash glTexSubImage2D() when uploading the last row of the image.
    // Here we add one extra row at the bottom of the image, and it seems to prevent this problem from happening.
    wxSize sizeWithExtraRow = wxSize(imageSize.GetWidth(), imageSize.GetHeight() + 1);
    wxImg->Resize(sizeWithExtraRow, wxPoint(0, 0));

    for (int i = 0; i < expandedImagePortion.BR.y; i++)
    {
        int texX = 0;
        //int texY = currentY;

        int srcAddress = (currentY * wxImg->GetSize().x + expandedImagePortion.TL.x) * 3;
        unsigned char* imageData = wxImg->GetData();

        int level = 0;
        int leftEdge = 0;

        glTexSubImage2D(GL_TEXTURE_2D, level, leftEdge, currentY, copyWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, &imageData[srcAddress]);
        currentY++;

        if (currentY == expandedImagePortion.BR.y)
        {
            uploadedTexture = true;
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Use mipmaps
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
        if (uploadedTexture)
            break;
    }

    return true;
}




void GL_Image::Invalidate()
{
    //hasGeneratedTexture = false;
    loadedImage         = false;
    uploadedTexture     = false;
}

// True if image width is 2^n - 9
// For some reason that doesn't work
// 
bool IsBadWidth(int w)
{
    int x = 16;

    while (true)
    {
        if (w == (x - 9))
            return true;

        if (x > w)
            return false;
           
        x *= 2;
    }
}

void GL_Image::Load(wxFileName fileName)
{
    //cout << "GL_Image::Load(" << fileName.GetFullPath() << ")" << endl;

    Invalidate();


    // check the file exists
    if (!wxFileExists(fileName.GetFullPath()))
    {
        //cout << "  File doesn't exist: " << fileName.GetFullPath() << endl;
        exit(1);
    }

    //int w, h;
    wxFileName fn(fileName);

	if ((fn.GetExt().Lower() == "jpg") ||
		(fn.GetExt().Lower() == "jpeg"))
	{
		//cout << "  Using JPEG Turbo for " << fileName.GetFullPath() << endl;

		wxLongLong startTime = wxGetLocalTimeMillis();
		//int exitCode = LoadJPEGTest("IMG_2287.jpg"); // fileName.char_str());

		int success = ReadJpegHeader(&load_state, (const  char*)fileName.GetFullPath().c_str());
		
		int w = load_state.width, h = load_state.height;
		//cout << "Success = " << success << ", " << w << ", " << h << endl;
		if (success)
        {		
            wxImg.Create(w, h);
            JpegRead(wxImg.GetData(), &load_state);

            if (IsBadWidth(w))                                              // If image is 1015 or 2039 etc. pixels wide, 
            {
                wxImg.Resize(wxSize(w + 2, h), wxPoint(0, 0), 0, 0, 0);     // then add a 1-pixel border on the left and right edges. Strange bug.
            }
        }
		else
		{
			wxImg.Create(32, 32);
			unsigned char *data = wxImg.GetData();
			for (int y = 0; y < 32; y++)
			    for (int x = 0; x < 32; x++)
				{
					*data++ =  x    ^  y   ;
					*data++ = (x*2) ^ (y*2);
					*data++ = (x*3) ^ (y*3);
				}
		}
	}
	else
	{
        //cout << "Using wxImg.LoadFile()\n";
		wxImg.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
		wxImg.LoadFile(fileName.GetFullPath());
	}

    //cout << "  Loading done. Starting upload." << endl;
    wxLongLong startTime = wxGetLocalTimeMillis();

    startTime     = wxGetLocalTimeMillis();
    width         = wxImg.GetWidth();
    height        = wxImg.GetHeight();

    //imageFFT.CreateFftSubImage(wxImg, wxImg, wxPoint(0, 0), 256);
    //imageFFT.CreateFftWholeImage(wxImg);

    //textureWidth  = width;
    //textureHeight = height;
    //cout << "  size = (" << width << ", " << height << ")" << endl;

    imageData = wxImg.GetData();
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Many graphics cards require that textures be power of two.
    // Below is a simple implementation, probably not optimal but working.
    // If your texture sizes are not restricted to powers of 2, you can
    // of course adapt the bit below as needed.

    /*
    float power_of_two_that_gives_correct_width = std::log((float)(width)) / std::log(2.0);
    float power_of_two_that_gives_correct_height = std::log((float)(height)) / std::log(2.0);

    // check if image dimensions are a power of two
    if ((int)power_of_two_that_gives_correct_width == power_of_two_that_gives_correct_width &&
        (int)power_of_two_that_gives_correct_height == power_of_two_that_gives_correct_height)
    {
        // note: must make a local copy before passing the data to OpenGL, as GetData() returns RGB 
        // and we want the Alpha channel if it's present. Additionally OpenGL seems to interpret the 
        // data upside-down so we need to compensate for that.
        GLubyte *bitmapData = wxImg.GetData();
        GLubyte *alphaData  = wxImg.GetAlpha();

        int bytesPerPixel = wxImg.HasAlpha() ? 4 : 3;

        int imageSize = (width) * (height) * bytesPerPixel;
        imageData = new GLubyte[imageSize];
        int rev_val = height - 1;

        for (int y = 0; y<height; y++)
        {
            for (int x = 0; x<(width); x++)
            {
                imageData[(x + y*(width))*bytesPerPixel + 0] = bitmapData[(x + (rev_val - y)*width) * 3 + 0];
                imageData[(x + y*(width))*bytesPerPixel + 1] = bitmapData[(x + (rev_val - y)*width) * 3 + 1];
                imageData[(x + y*(width))*bytesPerPixel + 2] = bitmapData[(x + (rev_val - y)*width) * 3 + 2];

                if (bytesPerPixel == 4)
                    imageData[(x + y*width)*bytesPerPixel + 3] = alphaData[x + (rev_val - y)*width];
            }
        }

    }
    else // texture is not a power of two. We need to resize it
    {
        int newWidth = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)));
        int newHeight = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)));

        textureSizes[0].x = newWidth;
        textureSizes[0].y = newHeight;

        //cout << "  size = (" << newWidth << ", " << newHeight << ")" << endl;

        //printf("Unsupported image size. Recommand values: %i %i\n",newWidth,newHeight);   

        GLubyte	*bitmapData = wxImg.GetData();
        GLubyte  *alphaData = wxImg.GetAlpha();

        int old_bytesPerPixel = 3;
        int bytesPerPixel = wxImg.HasAlpha() ? 4 : 3;

        int imageSize = newWidth * newHeight * bytesPerPixel;
        imageData = new GLubyte[imageSize];

        int rev_val = height - 1;

        //for (int y = 0; y<newHeight; y++)
        for (int y = 0; y<height; y++)
        {
            CopyImageLine(y);
        }
    }
    */
    //cout << "W:" << width << " " << textureWidth << endl;
    //cout << "H:" << height << " " << textureHeight << endl;

    CalculateTextureSizes();
    

    loadedImage = true;
}


wxString GL_Image::GetInfoString() const
{
    return infoString;
    //return fileName;
}

wxString GL_Image::GetDimensionsString() const
{
    return dimensionsString;
}

void GL_Image::SetFileName(wxFileName fileName)
{
    //fileName.Printf("%s\n%dx%d", fileName.GetFullPath(), width, height);
    //fileName.Printf("%s", fn);
    //dimensionsString.Printf("%d x %d", width, height);
}

wxString GL_Image::GetZoomInfo() const
{
    wxString s;

    s.Printf("Zoom: %d%%", (int)(scale*100.0));

    return s;
}


double GL_Image::GetScaleDifference(const GL_Image& glImage) const
{
    return ((double)(width*height) / (double)(glImage.width*glImage.height));

    //double scaleX, scaleY;
    //
    //if (width > glImage.width)
    //{
    //    scaleX = width / glImage.width;
    //}
    //else`
    //{
    //    scaleX = glImage.width / width;
    //}
    //
    //if (height > glImage.height)
    //{
    //    scaleY = height / glImage.height;
    //}
    //else
    //{
    //    scaleY = glImage.height / height;
    //}
    //
    //if (scaleX > scaleY)
    //    return scaleX;
    //else
    //    return scaleY;
}


void GL_ImageServer::ClearCache()
{
    //cout << "GL_ImageServer::ClearCache()" << endl;
    int i, n = imageSet.size();

    for (i = 0; i < n; i++)
    {
        imageSet[i].fileName         = "";
        imageSet[i].glImage.fileName = "";
        imageSet[i].creationTime     = 0;
    }

    UpdateDebuggingText();
}

//void GL_ImageServer::SetFileNameList(FileNameList *fnl)
//{
//    fileNameList = fnl;
//}


void GL_ImageServer::Reset()
{
    ClearCache();
}

int GL_ImageServer::Cache(wxFileName fileName)
{

    //cout << "  - Caching.  fileNameList[" << imageNumber << " = " << (*fileNameList)[imageNumber] << endl;
    int cacheLocation = GetCacheLocation(fileName);

    if (cacheLocation > -1)                             // Is the image already in the cache?
    {
        //cout << "  - Already exists at " << cacheLocation << endl;
        return cacheLocation;                           // then no need to do anything.
    }

    t++;

    cacheLocation = GetOldestCacheLocation();
    //cout << "  - Placing at " << cacheLocation << endl;

    imageSet[cacheLocation].glImage.Invalidate();
    imageSet[cacheLocation].creationTime            = t;
    imageSet[cacheLocation].fileName = fileName;

    //cout << "    caching " << fileName << endl;

    //cout << "Creating handler" << endl;
    ImageLoader *imageLoader = new ImageLoader(imageSet[cacheLocation].glImage, fileName, basicGLPanel, glContext);      // Begin loading the image in the background.
    imageLoader->Run();


    //imageSet[cacheLocation].glImage.Load(path);
    //cout << "  - Loading " << (*fileNameList)[imageNumber] << endl;
    //imageSet[cacheLocation].glImage.Load((*fileNameList)[imageNumber]);
    //cout << "  - Loading done" << endl;

    return cacheLocation;
}

ImageLoader::~ImageLoader()
{
    //cout << "Image handler deleting" << endl;
}



GL_Image* GL_ImageServer::GetImage(wxFileName fileName)
{
    wxString text;

    // text.Printf("GL_ImageServer::GetImage(%d)", imageNumber);

    //SetDebuggingText(text);

    //cout << endl << "GL_ImageServer::GetImage(" << imageNumber  << ")" << endl;

    int cacheLocation = GetCacheLocation(fileName);

    if (cacheLocation == -1)
    {
        cacheLocation = Cache(fileName);
    }
    //cout << "  Found at " << cacheLocation << endl;

    //cout << "thumbnailCanvas = " << &thumbnailCanvas << endl;

    imageSet[cacheLocation].glImage.SetFileName(fileName);

    if (thumbnailCanvas)
    {
        wxString infoString = thumbnailCanvas->GetInfoString(fileName);
        //cout << "Setting info string: " << infoString << endl;
        imageSet[cacheLocation].glImage.SetInfoString(infoString);
    }
    else
    {
        //cout << "No info string" << endl;
    }

    return &(imageSet[cacheLocation].glImage);
}


int GL_ImageServer::GetCacheLocation(wxFileName fileName)
{
    //cout << "GL_ImageServer::GetCacheLocation(" << imageNumber << ")" << endl;
    int i, n = imageSet.size();

    for (i = 0; i < n; i++)
    {
        //cout << "  checking " << i << " = " << imageSet[i].imageNumber << endl;
        if (imageSet[i].fileName == fileName)
            return i;
    }

    //cout << "  didn't find" << endl;
    return -1;
}


int GL_ImageServer::GetOldestCacheLocation()
{
    int i, n = imageSet.size();
    int oldestIndex = -1;
    int oldestAge = -1;

    for (i = 0; i < n; i++)
    {
        if (imageSet[i].creationTime < oldestAge)
        {
            oldestAge = imageSet[i].creationTime;
            oldestIndex = i;
        }
    }

    return oldestIndex;
}

int  GL_ImageServer::NextImageToCache()
{
    /*
    // First check if we need and can get the next image
    if (currentImage < (*fileNameList).MaxFileNumber())
    {
        int nextImage = currentImage + 1;
        if (GetCacheLocation(nextImage) > -1)
        {
            return nextImage;
        }
    }

    // Now start checking 
    */
    return -1;
}

void GL_ImageServer::HandleCaching()
{
    UpdateDebuggingText();
}

void GL_ImageServer::UpdateDebuggingText()
{
    int i, n = imageSet.size();
    wxString lines;

    for (i = 0; i < n; i++)
    {
        wxString s;
        wxFileName fullName = imageSet[i].glImage.fileName;
        wxString fileName = fullName.GetName();

        fileName += "              ";
        fileName = fileName.Mid(0, 15);

        wxChar loadedImage = '_', uploaded = '_', genTex = '_';
        int crTime = imageSet[i].creationTime;

        if (imageSet[i].glImage.loadedImage     == true)   loadedImage = '#';
        if (imageSet[i].glImage.uploadedTexture == true)      uploaded = '#';
        //if (imageSet[i].glImage.hasGeneratedTexture == true)    genTex = '#';

        s.Printf("%s  %c %c %c %d\n", fileName, loadedImage, uploaded, genTex, crTime);

        lines += s;
    }

    //SetDebuggingText(lines);
}

size_t GL_ImageServer::GetNumImages()
{
    return 99; // fileNameList->NumFiles();
}
