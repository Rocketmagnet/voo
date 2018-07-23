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

extern "C"
{
    #include "jpeg_turbo.h"
};

extern void NoteTime(wxString s);
void SetDebuggingText(wxString text);


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
: hasGeneratedTexture(false),
  basicGLPanel(0)
{
    //cout << this << "  GL_Image::GL_Image()" << endl;

    x     = 0.0;
    y     = 0.0;
    scale = 1.0;
}


GL_Image::~GL_Image()
{
    //cout << "Destructor deleting texture " << ID << endl;
    glDeleteTextures(4, ID);
}

bool GL_Image::IsFullyVisible() const
{
    cout << scale << endl;
    cout << scale * width << " " << basicGLPanel->GetWidth() << endl;
    cout << scale * height << " " << basicGLPanel->GetHeight() << endl;

    if (scale*width > basicGLPanel->GetWidth())
        return false;

    if (scale*height > basicGLPanel->GetHeight())
        return false;

    return true;
}


void GL_Image::ExpandToSides()
{
    double screenAspect = basicGLPanel->GetWidth() / basicGLPanel->GetHeight();
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

    //cout << " left          " << left << endl;
    //cout << " right         " << right << endl;
    //cout << " top           " << top << endl;
    //cout << " bottom        " << bottom << endl;
    //cout << " screenWidth   " << screenWidth << endl;
    //cout << " screenHeight  " << screenHeight << endl;
    //cout << " hScreenWidth  " << hScreenWidth << endl;
    //cout << " hScreenHeight " << hScreenHeight << endl;
    //cout << "  leftGap      " << leftGap << endl;
    //cout << " rightGap      " << rightGap << endl;
    //cout << "    topGap     " << topGap << endl;
    //cout << " bottomGap     " << bottomGap << endl;

    //cout << "x=" << x << "    left=" << left << "   gap" << leftGap << " width=" << screenWidth << "  hWidth=" << hScreenWidth << endl;
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
    

void GL_Image::Render()
{
    //NoteTime(wxT("GL_Image::Render"));

    if (!loadedImage)
        return;

    if (!uploadedTexture)
        UploadNextBlock();

    glLoadIdentity();

    glBindTexture(GL_TEXTURE_2D, ID[0]);

    double left   = x - (width  * scale * 0.5);
    double right  = x + (width  * scale * 0.5);
    double top    = y - (height * scale * 0.5);
    double bottom = y + (height * scale * 0.5);

    float screenWidth  = basicGLPanel->GetWidth();
    float screenHeight = basicGLPanel->GetHeight();

    glTranslatef(screenWidth * 0.5, screenHeight * 0.5, 0);

    glBegin(GL_QUADS);
        glTexCoord2f(0, tex_coord_y);    glVertex2f(left, top);
        glTexCoord2f(tex_coord_x, tex_coord_y);    glVertex2f(right, top);
        glTexCoord2f(tex_coord_x, 0);    glVertex2f(right, bottom);
        glTexCoord2f(0, 0);    glVertex2f(left, bottom);
    glEnd();

    //NoteTime(wxT("GL_Image::Render Quad"));
}

void GL_Image::CalculateTextureSizes()
{
    int   auxWidth  = 0;
    int   auxHeight = 0;

    int width0  = width,  width1  = 0;      // Default texture sizes, assuming that the image isn't too big
    int height0 = height, height1 = 0;

    if (width > MAX_WIDTH)
    {
        float power_of_two_that_gives_correct_width = std::log((float)(width- MAX_WIDTH)) / std::log(2.0);
        width1 = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)));
        width0 = MAX_WIDTH;
    }

    if (height > MAX_HEIGHT)
    {
        float power_of_two_that_gives_correct_height = std::log((float)(width - MAX_HEIGHT)) / std::log(2.0);
        height1 = (int)std::pow(2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)));
        height0 = MAX_WIDTH;
    }

    textureSizes[0] = wxSize(width0, height0);
    textureSizes[1] = wxSize(width1, height0);
    textureSizes[2] = wxSize(width0, height1);
    textureSizes[3] = wxSize(width1, height1);
}


void TextureUpload::Init(wxImage *img, wxSize size, wxPoint TL, wxPoint BR)
{
    cout << "TextureUpload::Init(" << size.x << ", " << size.y << ")" << endl;
    wxImg       = img;
    textureSize = size;
    topLeft     = TL;
    bottomRight = BR;

    uploadedTexture = false;

    nextBlockToUpload = 0;
    blockSize = size.y / (BLOCK_SIZE_PIXELS / size.x);
    lastBlock = size.y / blockSize;

    cout << "blockSize = " << blockSize << "    lastBlock = " << lastBlock << endl;
}


bool TextureUpload::UploadNextBlock()
{
    if (!wxImg)
        return;

    int imageWidth  = wxImg->GetSize().x;
    int imageHeight = wxImg->GetSize().y;

    if (nextBlockToUpload == 0)
    {
        if (hasGeneratedTexture)
        {
            cout << "UPLOAD: Deleting texture " << ID << endl;
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

        glBindTexture(GL_TEXTURE_2D, ID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize.x, textureSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    int texX = 0;
    int texY = blockSize * nextBlockToUpload;
    int uploadHeight = height - texY;
    if (uploadHeight > blockSize)
    {
        uploadHeight = blockSize;
    }
    int startAddress = texY * textureSizes[0].x * 3;
    int finalY = texY + uploadHeight;

    cout << "UPLOAD: uploading " << texY << " " << uploadHeight << endl;

    //glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY, textureSizes[0].x, uploadHeight, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);

    for (int i = 0; i < uploadHeight; i++)
    {
        startAddress = (texY + i) * width * 3;
        glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY + i, width, 1, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);

        //glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY, textureSizes[0].x, uploadHeight, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);
    }

    if (nextBlockToUpload == lastBlock)
    {
        //cout << "UPLOAD: Deleting image data" << endl;
        delete[] imageData;
        NoteTime(wxT("delete[] imageData"));
        imageData = 0;
        uploadedTexture = true;
        wxImg.Destroy();
    }

    nextBlockToUpload++;

}


void GL_Image::UploadNextBlock()
{
    //cout << "UploadNextBlock() " << (int)imageData << endl;
    NoteTime(wxT("GL_Image::UploadNextBlock"));

    if (!imageData)
        return;


    if (nextBlockToUpload == 0)
    {
        if (hasGeneratedTexture)
        {
            //cout << "UPLOAD: Deleting texture " << ID << endl;
            hasGeneratedTexture = false;
            glDeleteTextures(4, ID);
            NoteTime(wxT("Delete texture"));
        }

        glGenTextures(4, ID);
        NoteTime(wxT("Generate texture"));
        //cout << "UPLOAD: Generating texture " << ID << endl;
        hasGeneratedTexture = true;

        //cout << "UPLOAD: Binding texture" << endl;
        glBindTexture(GL_TEXTURE_2D, ID[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, ID[0]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSizes[0].x, textureSizes[0].y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    }
    else
    {
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glBindTexture(GL_TEXTURE_2D, ID[0]);
    }

    int texX = 0;
    int texY = blockSize * nextBlockToUpload;
    int uploadHeight = height - texY;
    if (uploadHeight > blockSize)
    {
        uploadHeight = blockSize;
    }
    int startAddress = texY * textureSizes[0].x * 3;
    int finalY = texY + uploadHeight;

    cout << "UPLOAD: uploading " << texY << " " << uploadHeight << endl;

    //glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY, textureSizes[0].x, uploadHeight, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);

    for (int i = 0; i < uploadHeight; i++)
    {
        startAddress = (texY+i) * width * 3;
        glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY+i, width, 1, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);
        
      //glTexSubImage2D(GL_TEXTURE_2D, 0, texX, texY, textureSizes[0].x, uploadHeight, GL_RGB, GL_UNSIGNED_BYTE, &imageData[startAddress]);
    }

    if (nextBlockToUpload == lastBlock)
    {
        //cout << "UPLOAD: Deleting image data" << endl;
        delete[] imageData;
        NoteTime(wxT("delete[] imageData"));
        imageData = 0;
        uploadedTexture = true;
        wxImg.Destroy();
    }
    
    nextBlockToUpload++;
}


void GL_Image::CopyImageLine(int y)
{
    int srcStep = wxImg.HasAlpha() ? 4 : 3;
    GLubyte *bitmapData = wxImg.GetData();
    int src = (height-y-1) * width * srcStep;
    int dst = y * textureSizes[0].x * 3;

    memcpy(&(imageData[dst]), &(bitmapData[src]), width * 3);

    //cout << "srcStep = " << srcStep << endl;
    //for (int x = 0; x < width; x++)
    //{
    //    imageData[dst++] = bitmapData[src+0];
    //    imageData[dst++] = bitmapData[src+1];
    //    imageData[dst++] = bitmapData[src+2];
    //    src += srcStep;
    //}
}


void GL_Image::Invalidate()
{
    if (hasGeneratedTexture)
    {
        glDeleteTextures(4, ID);
    }

    hasGeneratedTexture = false;
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
		jpeg_load_state *load_state = ReadJpegHeader((const  char*)path.c_str());


        if (load_state)
        {
            int w = load_state->width, h = load_state->height;

            wxImg.Create(w, h);
            JpegRead(wxImg.GetData(), load_state);
        }
        else
        {
            cout << "JPEG Load Error" << endl;
            return;
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
    CalculateTextureSizes();
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

    tex_coord_x       = (float)width  / (float)textureSizes[0].x;
    tex_coord_y       = (float)height / (float)textureSizes[0].y;
    nextBlockToUpload = 0;
    blockSize         = 100;
    lastBlock         = ceil((double)height / (double)blockSize) - 1;


    wxLongLong endTime = wxGetLocalTimeMillis();
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
    cout << "GL_ImageServer::SetFileNameList()" << endl;

    fileNameList = fnl;
}

int GL_ImageServer::Cache(int imageNumber)
{
    cout << "  Cache(" << imageNumber << ")" << endl;

    if (!fileNameList)
        return -1;
    

    cout << "  - Caching.  fileNameList[" << imageNumber << " = " << (*fileNameList)[imageNumber] << endl;
    int cacheLocation = GetCacheLocation(imageNumber);

    if (cacheLocation > -1)                             // Is the image already in the cache?
    {
        cout << "  - Already exists at " << cacheLocation << endl;
        return cacheLocation;                           // then no need to do anything.
    }

    t++;

    cacheLocation = GetOldestCacheLocation();
    cout << "  - Placing at " << cacheLocation << endl;

    imageSet[cacheLocation].glImage.Invalidate();
    imageSet[cacheLocation].creationTime            = t;
    imageSet[cacheLocation].imageNumber             = imageNumber;

    //wxString fileName = (*fileNameList)[imageNumber];
    //ImageLoader *imageLoader = new ImageLoader(imageSet[cacheLocation].glImage, fileName, basicGLPanel, glContext);      // Begin loading the image in the background.
    //imageLoader->Run();


    //imageSet[cacheLocation].glImage.Load(path);
    cout << "  - Loading " << (*fileNameList)[imageNumber] << endl;
    imageSet[cacheLocation].glImage.Load((*fileNameList)[imageNumber]);
    cout << "  - Loading done" << endl;

    return cacheLocation;
}

/*
GL_Image* GL_ImageServer::GetImage(int imageNumber)
{
    testImage.Load((*fileNameList)[imageNumber]);
    NoteTime(wxT("testImage.Load()"));
    wxFileName fn((*fileNameList)[imageNumber]);

    //cout << "SetFileName " << fn.GetFullPath() << ", " << fn.GetFullName() << endl;
    testImage.SetFileName(fn.GetFullName());
    return &testImage;
}
*/


GL_Image* GL_ImageServer::GetImage(int imageNumber)
{
    wxString text;

    // text.Printf("GL_ImageServer::GetImage(%d)", imageNumber);

    //SetDebuggingText(text);

    cout << endl << "GL_ImageServer::GetImage(" << imageNumber  << ")" << endl;

    int cacheLocation = GetCacheLocation(imageNumber);

    if (cacheLocation == -1)
    {
        cacheLocation = Cache(imageNumber);
    }
    cout << "  Found at " << cacheLocation << endl;

    imageSet[cacheLocation].glImage.SetFileName((*fileNameList)[imageNumber]);
    return &(imageSet[cacheLocation].glImage);
}


int GL_ImageServer::GetCacheLocation(int imageNumber)
{
    cout << "GL_ImageServer::GetCacheLocation(" << imageNumber << ")" << endl;
    int i, n = imageSet.size();

    for (i = 0; i < n; i++)
    {
        cout << "  checking " << i << " = " << imageSet[i].imageNumber << endl;
        if (imageSet[i].imageNumber == imageNumber)
            return i;
    }

    cout << "  didn't find" << endl;
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
        if (imageSet[i].glImage.hasGeneratedTexture == true)    genTex = '#';

        s.Printf("%02d: %s  %c %c %c %d\n", imageSet[i].imageNumber, fileName, loadedImage, uploaded, genTex, crTime);

        lines += s;
    }

    SetDebuggingText(lines);
}
