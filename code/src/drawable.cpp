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



wxThread::ExitCode ImageLoader::Entry()
{
	glImage.Load(fileName);

    // Generate a new texture
    //if (glImage.hasGeneratedTexture)
    //{
    //    glDeleteTextures(1, &glImage.ID);
    //}
	//
    //glGenTextures(1, &glImage.ID);
    //glImage.hasGeneratedTexture = true;
	//
    //// check the file exists
    //if (!wxFileExists(fileName))
    //{
    //    cout << ">   File doesn't exist: " << fileName << endl;
    //    exit(1);
    //}
	//
    //glImage.wxImg.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    //glImage.wxImg.LoadFile(fileName);
	//
    //glImage.width  = glImage.wxImg.GetWidth();
    //glImage.height = glImage.wxImg.GetHeight();
	//
    //glImage.loadedImage = true;
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
    glDeleteTextures(1, &ID);
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


    if (height*scale >= screenHeight)           // If image is wider than the screen
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

    if (!loadedImage)
        return;

    if (!uploadedTexture)
        UploadNextBlock();

    //cout << "Render()" << endl;

    glLoadIdentity();


    double left = x - width*scale*0.5;
    double right = x + width*scale*0.5;

    double top = y - height*scale*0.5;
    double bottom = y + height*scale*0.5;

    float screenWidth  = basicGLPanel->GetWidth();
    float screenHeight = basicGLPanel->GetHeight();

    glTranslatef(screenWidth * 0.5, screenHeight * 0.5, 0);

    glBindTexture(GL_TEXTURE_2D, ID);

    glBegin(GL_QUADS);
        glTexCoord2f(          0, tex_coord_y);    glVertex2f(left,  top   );
        glTexCoord2f(tex_coord_x, tex_coord_y);    glVertex2f(right, top   );
        glTexCoord2f(tex_coord_x,           0);    glVertex2f(right, bottom);
        glTexCoord2f(          0,           0);    glVertex2f(left,  bottom);
    glEnd();
}

void GL_Image::UploadNextBlock()
{
    cout << "UploadNextBlock() " << (int)imageData << endl;

    if (!imageData)
        return;

    wxLongLong startTime = wxGetLocalTimeMillis();

    if (hasGeneratedTexture)
        glDeleteTextures(1, &ID);

    glGenTextures(1, &ID);
    hasGeneratedTexture = true;

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    gluBuild2DMipmaps(GL_TEXTURE_2D,
        GL_RGB,
        textureWidth, textureHeight,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        imageData);


    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);

    //glGenerateMipmap(GL_TEXTURE_2D);
    //textureWidth = width;
    //textureHeight = height;

    delete[] imageData;
    imageData = 0;

    uploadedTexture = true;

    wxLongLong endTime = wxGetLocalTimeMillis();
    cout << "upload time 1 = " << endTime-startTime << endl;

    cout << "UploadNextBlock() done" << endl;
}

void GL_Image::CopyImageLine(int y)
{
    int srcStep = wxImg.HasAlpha() ? 4 : 3;
    GLubyte *bitmapData = wxImg.GetData();
    int src = (height-y-1) * width * srcStep;
    int dst = y * textureWidth * 3;

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
    loadedImage = false;
    uploadedTexture = false;
}


void GL_Image::Load(wxString path)
{
    cout << "Load()" << endl;


    loadedImage       = false;
    uploadedTexture   = false;
    //nextBlockToUpload = 0;



    // check the file exists
    if (!wxFileExists(path))
    {
        cout << "File doesn't exist: " << path << endl;
        exit(1);
    }

	int w, h;
	wxFileName fn(path);

	if ((fn.GetExt().Lower() == "jpg") ||
		(fn.GetExt().Lower() == "jpeg"))
	{
		cout << "Using JPEG Turbo for " << path << endl;

		wxLongLong startTime = wxGetLocalTimeMillis();
		//int exitCode = LoadJPEGTest("IMG_2287.jpg"); // fileName.char_str());
		jpeg_load_state *load_state = ReadJpegHeader((const  char*)path.c_str());


        if (load_state)
        {
            int w = load_state->width, h = load_state->height;

            cout << "Image " << w << "x" << h << endl;
            //int exitCode = read_JPEG_file((const  char*)fileName.c_str());
            //char *imageBuffer = new char[w*h*3];
            wxImg.Create(w, h);
            //wxImg.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
            JpegRead(wxImg.GetData(), load_state);


            wxLongLong endTime = wxGetLocalTimeMillis();
            //cout << "exitCode = " << exitCode << endl;
            cout << "JPEG load time = " << endTime - startTime << endl;
        }
	}
	else
	{
		wxImg.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
		wxImg.LoadFile(path);
	}

    wxLongLong startTime = wxGetLocalTimeMillis();

    startTime = wxGetLocalTimeMillis();
    width  = wxImg.GetWidth();
    height = wxImg.GetHeight();
    textureWidth  = width;
    textureHeight = height;

    //cout << "  size = (" << width << ", " << height << ")" << endl;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Many graphics card require that textures be power of two.
    // Below is a simple implementation, probably not optimal but working.
    // If your texture sizes are not restricted to power of 2s, you can
    // of course adapt the bit below as needed.
     
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
        cout << "imageData = " << (int)imageData << endl;
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

        textureWidth = newWidth;
        textureHeight = newHeight;

        //cout << "  size = (" << newWidth << ", " << newHeight << ")" << endl;

        //printf("Unsupported image size. Recommand values: %i %i\n",newWidth,newHeight);   

        GLubyte	*bitmapData = wxImg.GetData();
        GLubyte  *alphaData = wxImg.GetAlpha();

        int old_bytesPerPixel = 3;
        int bytesPerPixel = wxImg.HasAlpha() ? 4 : 3;

        int imageSize = newWidth * newHeight * bytesPerPixel;
        imageData = new GLubyte[imageSize];
        cout << "_imageData = " << (int)imageData << endl;

        int rev_val = height - 1;

        //for (int y = 0; y<newHeight; y++)
        for (int y = 0; y<height; y++)
        {
            CopyImageLine(y);
            //for (int x = 0; x<newWidth; x++)
            //{
            //    if (x<width && y<height)
            //    {
            //        imageData[(x + y*newWidth)*bytesPerPixel + 0] = bitmapData[(x + (rev_val - y)*width)*old_bytesPerPixel + 0];
            //        imageData[(x + y*newWidth)*bytesPerPixel + 1] = bitmapData[(x + (rev_val - y)*width)*old_bytesPerPixel + 1];
            //        imageData[(x + y*newWidth)*bytesPerPixel + 2] = bitmapData[(x + (rev_val - y)*width)*old_bytesPerPixel + 2];
            //
            //        if (bytesPerPixel == 4)
            //            imageData[(x + y*newWidth)*bytesPerPixel + 3] = alphaData[x + (rev_val - y)*width];
            //    }
            //    else
            //    {
            //        imageData[(x + y*newWidth)*bytesPerPixel + 0] = 0;
            //        imageData[(x + y*newWidth)*bytesPerPixel + 1] = 0;
            //        imageData[(x + y*newWidth)*bytesPerPixel + 2] = 0;
            //        if (bytesPerPixel == 4)
            //            imageData[(x + y*newWidth)*bytesPerPixel + 3] = 0;
            //    }
            //}
        }
    }

    cout << "W:" << width << " " << textureWidth << endl;
    cout << "H:" << height << " " << textureHeight << endl;

    tex_coord_x = (float)width  / (float)textureWidth;
    tex_coord_y = (float)height / (float)textureHeight;

    loadedImage = true;
    wxImg.Destroy();

    wxLongLong endTime = wxGetLocalTimeMillis();
    cout << "upload time 2 = " << endTime - startTime << endl;

    cout << "Load() done" << endl;
}


void GL_Image::CreateFakeImage()
{
    //cout << "GL_Image::CreateFakeImage()" << endl;
    GLubyte	*imageData = new GLubyte[64*64];

    glGenTextures(1, &ID);
    hasGeneratedTexture = true;

    glBindTexture(GL_TEXTURE_2D, ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    gluBuild2DMipmaps(GL_TEXTURE_2D,
        GL_RGB,
        64,64,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        imageData);


    delete[] imageData;

    glDeleteTextures(1, &ID);
}


int GL_ImageServer::Cache(int imageNumber)
{
    if (!fileNameList)
        return -1;
    

    cout << "Caching.  fileNameList[" << imageNumber << " = " << (*fileNameList)[imageNumber] << endl;
    int cacheLocation = GetCacheLocation(imageNumber);

    if (cacheLocation > -1)                             // Is the image already in the cache?
    {
        cout << "  Already exists at " << cacheLocation << endl;
        return cacheLocation;                           // then no need to do anything.
    }

    t++;

    cacheLocation = GetOldestCacheLocation();
    cout << "  Placing at " << cacheLocation << endl;

    imageSet[cacheLocation].glImage.Invalidate();
    imageSet[cacheLocation].creationTime            = t;
    imageSet[cacheLocation].imageNumber             = imageNumber;

    //wxString fileName = (*fileNameList)[imageNumber];
    //ImageLoader *imageLoader = new ImageLoader(imageSet[cacheLocation].glImage, fileName, basicGLPanel, glContext);      // Begin loading the image in the background.
    //imageLoader->Run();


    //imageSet[cacheLocation].glImage.Load(path);
    imageSet[cacheLocation].glImage.Load((*fileNameList)[imageNumber]);
    return cacheLocation;
}


GL_Image* GL_ImageServer::GetImage(int imageNumber)
{
    cout << endl << "GL_ImageServer::GetImage(" << imageNumber  << ")" << endl;

    int cacheLocation = GetCacheLocation(imageNumber);

    if (cacheLocation == -1)
    {
        cacheLocation = Cache(imageNumber);
    }
    cout << "  Found at " << cacheLocation << endl;

    //while (!imageSet[cacheLocation].glImage.completedFlag)
    //{
    //    //cout << "Waiting" << endl;
    //}

    //cout << "Detected load complete" << endl;
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

}

void GL_ImageServer::HandleCaching()
{
    
}