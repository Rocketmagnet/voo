#include <gl/glew.h>
#include "drawable.h"
#include "gl_panel.h"
#include <iostream>
using namespace std;

#include <GL/gl.h>
#include <wx/glcanvas.h>
#include "wx/wx.h"
#include "wx/image.h"
#include "status_bar.h"
#include "file_name_list.h"
#include "wx/time.h"
#include "wx/filename.h"

//extern "C"
//{
//    #include "jpeg_turbo.h"
//};

extern void NoteTime(wxString s);
void SetDebuggingText(wxString text);

#define OVERLAP 10
#define UNDERLAP 4

std::ostream & operator << (std::ostream & os, const RectangleVector & rv)
{
    return os << rv.TL << rv.BR;
}


wxThread::ExitCode ImageLoader::Entry()
{
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
        scale = 0.01;
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
    //cout << "TextureUpload::Render()" << endl;
    if (!valid)
    {
        //cout << "         - " << endl;
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

    //cout << ID << "        (" << x0 << ", " << y0 << ") - (" << x1 << ", " << y1 << ")" << endl;
    //cout << "(" << myTexturePortion.TL.x << ", " << myTexturePortion.TL.y << ")";
    //cout << "(" << myTexturePortion.BR.x << ", " << myTexturePortion.BR.y << ")" << endl;

    glBegin(GL_QUADS);
        //glColor3f(1.0, 0.00, 0);    glVertex2f(x0, y0);
        //glColor3f(1.0, 0.50, 0);    glVertex2f(x1, y0);
        //glColor3f(1.0, 1.00, 0);    glVertex2f(x1, y1);
        //glColor3f(1.0, 0.50, 0);    glVertex2f(x0, y1);

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
        return;

    if (!uploadedTexture)
    {
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
    //cout << endl;
    //cout << "TextureUpload::Init(" << TL.x << ", " << TL.y << " - " << BR.x << ", " << BR.y << ")" << endl;

      originalImagePortion = RectangleVector(TL, BR);
      expandedImagePortion = RectangleVector(TL, BR);

    if ((originalImagePortion.XSize() == 0) || (originalImagePortion.YSize() == 0))
    {
        valid = false;
        //cout << "Valid = " << valid << endl;
        return;
    }
    else
    {
        valid = true;
        //cout << "Valid = " << valid << endl;
    }

    //clips = clip;
    //cout << "Clips = " << clips << endl;
    if (overlap & OVERLAP_LEFT)
    {
          expandedImagePortion.TL.x -= OVERLAP;
        //renderableImagePortion.TL.x -= OVERLAP / 2;
    }

    if (overlap & OVERLAP_RIGHT)
    {
          expandedImagePortion.BR.x += OVERLAP;
        //renderableImagePortion.BR.x += OVERLAP / 2;
    }

    //cout << "  originalImagePortion = " << originalImagePortion   << endl;
    //cout << "  expandedImagePortion = " << expandedImagePortion   << endl;
    //cout << "renderableImagePortion = " << renderableImagePortion << endl;


    int width  = expandedImagePortion.XSize();
    int height = expandedImagePortion.YSize();

    copyWidth = width;

    float power_of_two_that_gives_correct_width = std::log((float)(width - 1)) / std::log(2.0);
    float power_of_two_that_gives_correct_height = std::log((float)(height - 1)) / std::log(2.0);
    int textureWidth = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)));
    int textureHeight = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)));

    textureSize = RectangleVector(textureWidth, textureHeight);
    //textureSize = wxSize(textureWidth, textureHeight);

    //cout << "textureSize = " << textureSize.TL << textureSize.BR << endl;
    wxImg = img;

    double top = 0;
    double left = 0;

    //if (overlap & CLIP_LEFT) { left = OVERLAP / (double)textureWidth; }
    //if (overlap & CLIP_TOP)  { top  = OVERLAP / (double)textureHeight; }


     allocatedTexturePortion = RectangleVector( Vector2D(0,0),
                                                textureSize.GetFraction(expandedImagePortion.Size()) );

     renderableTexturePortion = originalImagePortion;
     renderableTexturePortion.Subtract(expandedImagePortion.TL);

     renderableTexturePortion = RectangleVector(textureSize.GetFraction(renderableTexturePortion.TL),
                                                textureSize.GetFraction(renderableTexturePortion.BR)  );


     //renderableTexturePortion.TL.x *= allocatedTexturePortion.TL.x;
     //renderableTexturePortion.TL.y *= allocatedTexturePortion.TL.y;
     //renderableTexturePortion.BR.x *= allocatedTexturePortion.BR.x;
     //renderableTexturePortion.BR.y *= allocatedTexturePortion.BR.y;
     //cout << "renderableTexturePortion = " << renderableTexturePortion << endl;;

    // allocatedTexturePortion = RectangleVector( Vector2D(0,0),
    //                                            textureSize.GetFraction(expandedImagePortion.Size()) );
    //
    //renderableTexturePortion = RectangleVector( textureSize.GetFraction(originalImagePortion.TL), 
    //                                            textureSize.GetFraction(originalImagePortion.BR)  );

    //cout << "  allocatedTexturePortion = " << allocatedTexturePortion << endl;
    //cout << "  renderableTexturePortion = " << renderableTexturePortion << endl;

    //myTexturePortion = RectangleVector(Vector2D(left, top), Vector2D((double)width / (double)textureWidth, (double)height / (double)textureHeight));

    //cout << "textureSize = (" << textureWidth << ", " << textureHeight << ")" << endl;
    //cout << "copyWidth = " << copyWidth << endl;
    //cout << "originalImagePortion = (" << originalImagePortion.TL.x << ", " << originalImagePortion.TL.y << ") - (" << originalImagePortion.BR.x << ", " << originalImagePortion.BR.y << ")" << endl;
    //cout << "myTexturePortion = (" << myTexturePortion.TL.x << ", " << myTexturePortion.TL.y << ") - (" << myTexturePortion.BR.x << ", " << myTexturePortion.BR.y << ")" << endl;

    //cout << "TL = " << topLeftFloat << "    BR = " << bottomRightFloat << endl;

    uploadedTexture = false;
    
    currentY = 0;
    //lastY    = originalImagePortion;
    //cout << "blockSize = " << blockSize << "    lastBlock = " << lastBlock << endl;
}

bool TextureUpload::UploadNextBlock()
{
    if (!wxImg)
        return true;

    if (uploadedTexture)
        return true;

    if (!valid)
        return true;

    //return true;

    int imageWidth  = wxImg->GetSize().x;
    int imageHeight = wxImg->GetSize().y;

    if (currentY == 0)
    {
        currentY = expandedImagePortion.TL.y;
        if (hasGeneratedTexture)
        {
            //cout << "UPLOAD: Deleting texture " << ID << endl;
            hasGeneratedTexture = false;
            glDeleteTextures(1, &ID);
            //NoteTime(wxT("Delete texture"));
        }

        glGenTextures(1, &ID);
        //NoteTime(wxT("Generate texture"));
        //cout << "UPLOAD: Generating texture " << ID << endl;
        hasGeneratedTexture = true;

        //cout << "UPLOAD: Binding texture" << endl;
        glBindTexture(GL_TEXTURE_2D, ID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        //cout << "Texture size = " << textureSize.x << ", " << textureSize.y << endl;

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize.BR.x, textureSize.BR.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    for (int i = 0; i < 64; i++)
    {
        int texX = 0;
        int texY = currentY;

        //cout << "(" << currentY << " x " << wxImg->GetSize().x << " + " << originalImagePortion.TL.x << ") * 3" << endl;
        int srcAddress = (currentY * wxImg->GetSize().x + expandedImagePortion.TL.x) * 3;
        unsigned char* imageData = wxImg->GetData();
        int level = 0;
        int leftEdge = 0;
        //cout << "Copying: (0, " << currentY << ")   size = (" << copyWidth << ", " << 1 << ")  from " << srcAddress << endl;
        glTexSubImage2D(GL_TEXTURE_2D, level, leftEdge, currentY, copyWidth, 1, GL_RGB, GL_UNSIGNED_BYTE, &imageData[srcAddress]);

        currentY++;
        if (currentY == expandedImagePortion.BR.y)
        {
            //cout << "uploadedTexture = true  1" << endl;
            uploadedTexture = true;
            //glGenerateMipmap(GL_TEXTURE_2D);
            //glGenerateTextureMipmap(ID);
        }
        //GL_INVALID_ENUM
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


void GL_Image::Load(wxString path)
{
    //cout << "GL_Image::Load(" << path << ")" << endl;

    Invalidate();


    // check the file exists
    if (!wxFileExists(path))
    {
        cout << "  File doesn't exist: " << path << endl;
        exit(1);
    }

	int w, h;
	wxFileName fn(path);

	if ((fn.GetExt().Lower() == "jpg") ||
		(fn.GetExt().Lower() == "jpeg"))
	{
		//cout << "  Using JPEG Turbo for " << path << endl;

		wxLongLong startTime = wxGetLocalTimeMillis();
		//int exitCode = LoadJPEGTest("IMG_2287.jpg"); // fileName.char_str());
		int success = ReadJpegHeader(&load_state, (const  char*)path.c_str());
		
		int w = load_state.width, h = load_state.height;
		cout << "Success = " << success << ", " << w << ", " << h << endl;
		if (success)
        {		
            wxImg.Create(w, h);
            JpegRead(wxImg.GetData(), &load_state);
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
		wxImg.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
		wxImg.LoadFile(path);
	}

    //cout << "  Loading done. Starting upload." << endl;
    wxLongLong startTime = wxGetLocalTimeMillis();

    startTime     = wxGetLocalTimeMillis();
    width         = wxImg.GetWidth();
    height        = wxImg.GetHeight();
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
    return fileName;
}

void GL_Image::SetFileName(wxString fn)
{
    fileName.Printf("%s  %dx%d", fn, width, height);
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
        imageSet[i].imageNumber = -1;
        imageSet[i].glImage.fileName = "";
        imageSet[i].creationTime = 0;
    }

    UpdateDebuggingText();
}

void GL_ImageServer::SetFileNameList(FileNameList *fnl)
{
    fileNameList = fnl;
}


void GL_ImageServer::Reset()
{
    ClearCache();
}

int GL_ImageServer::Cache(int imageNumber)
{
    //cout << "  Cache(" << imageNumber << ")" << endl;

    if (!fileNameList)
        return -1;
    

    //cout << "  - Caching.  fileNameList[" << imageNumber << " = " << (*fileNameList)[imageNumber] << endl;
    int cacheLocation = GetCacheLocation(imageNumber);

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
    imageSet[cacheLocation].imageNumber             = imageNumber;

    wxString fileName = (*fileNameList)[imageNumber];
    ImageLoader *imageLoader = new ImageLoader(imageSet[cacheLocation].glImage, fileName, basicGLPanel, glContext);      // Begin loading the image in the background.
    imageLoader->Run();


    //imageSet[cacheLocation].glImage.Load(path);
    //cout << "  - Loading " << (*fileNameList)[imageNumber] << endl;
    //imageSet[cacheLocation].glImage.Load((*fileNameList)[imageNumber]);
    //cout << "  - Loading done" << endl;

    return cacheLocation;
}



GL_Image* GL_ImageServer::GetImage(int imageNumber)
{
    wxString text;

    // text.Printf("GL_ImageServer::GetImage(%d)", imageNumber);

    //SetDebuggingText(text);

    //cout << endl << "GL_ImageServer::GetImage(" << imageNumber  << ")" << endl;

    int cacheLocation = GetCacheLocation(imageNumber);

    if (cacheLocation == -1)
    {
        cacheLocation = Cache(imageNumber);
    }
    //cout << "  Found at " << cacheLocation << endl;
    
    imageSet[cacheLocation].glImage.SetFileName((*fileNameList)[imageNumber]);
    return &(imageSet[cacheLocation].glImage);
}


int GL_ImageServer::GetCacheLocation(int imageNumber)
{
    //cout << "GL_ImageServer::GetCacheLocation(" << imageNumber << ")" << endl;
    int i, n = imageSet.size();

    for (i = 0; i < n; i++)
    {
        //cout << "  checking " << i << " = " << imageSet[i].imageNumber << endl;
        if (imageSet[i].imageNumber == imageNumber)
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

        s.Printf("%02d: %s  %c %c %c %d\n", imageSet[i].imageNumber, fileName, loadedImage, uploaded, genTex, crTime);

        lines += s;
    }

    //SetDebuggingText(lines);
}

size_t GL_ImageServer::GetNumImages()
{
	return fileNameList->NumFiles();
}
