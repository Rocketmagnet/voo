
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

bool reg_JpegHandler1 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateJpegHandler, _T("JPEG"), _T("jpg"),  _T("Lossy compressed image."));
bool reg_JpegHandler2 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateJpegHandler, _T("JPEG"), _T("jpeg"), _T("Lossy compressed image."));

void JpegHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxImage   image;

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
    }
    else
    {
        image.Create(32, 32);
        unsigned char *data = image.GetData();
        for (int y = 0; y < 32; y++)
            for (int x = 0; x < 32; x++)
            {
                *data++ = x ^ y;
                *data++ = (x * 2) ^ (y * 2);
                *data++ = (x * 3) ^ (y * 3);
            }
    }

    //image.Resize(dx2, dy2);       // crop it
    thumbnail.SetImage(image);
    thumbnail.FinishedLoading();
    image.Destroy();
}

int JpegHandler::LoadImage(wxString fileName)
{
    return LOAD_IMAGE;
}

