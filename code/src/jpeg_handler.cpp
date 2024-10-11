
#include "jpeg_handler.h"
#include "thumbnail_canvas.h"
#include <iostream>

using namespace std;

ImageFileHandler* CreateJpegHandler()
{
    //GerberRenderer * renderer = new GerberRenderer(_T(""));
    //return renderer;
    return new JpegHandler();
}

bool reg_JpegHandler1 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateJpegHandler, _T("JPEG"), _T("jpg"),  _T("Lossy compressed image."), CAN_VIEW_IMAGE);
bool reg_JpegHandler2 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateJpegHandler, _T("JPEG"), _T("jpeg"), _T("Lossy compressed image."), CAN_VIEW_IMAGE);

void JpegHandler::ConvertGreyscaleToRGB(wxImage& image)
{
    int w = image.GetWidth();
    int h = image.GetHeight();

    int lastPixel = w * h;

    unsigned char* src = image.GetData() + lastPixel;
    unsigned char* dst = image.GetData() + (lastPixel*3);

    for (int i = lastPixel - 1; i >= 0; i--)
    {
        unsigned char grey = *src--;
        *dst-- = grey;
        *dst-- = grey;
        *dst-- = grey;
    }
}

bool JpegHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxImage   image;
    bool      returnValue;
    //cout << "JpegHandler::LoadThumbnail(" << fileName << ")" << endl;

    //jpeg_load_state *load_state = ReadJpegHeader((const  char*)fileName.c_str());
    int success = ReadJpegHeader(&jpegLoadState, (const char*)fileName.c_str());
    jpeg_load_state *load_state = &jpegLoadState;

    int w = load_state->width, h = load_state->height;

    if (success)
    {
        image.Create(w, h);
        image.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
        JpegRead(image.GetData(), load_state);
        returnValue = true;

        thumbnail.SetImage(image);
        thumbnail.FinishedLoading();
        image.Destroy();
    }
    else                                                        // Failed images get the XOR pattern.
    {
        returnValue = false;
    }

    //image.Resize(dx2, dy2);       // crop it

    return returnValue;
}

int JpegHandler::LoadImage(wxString fileName)
{
    return LOAD_IMAGE;
}

