
#include "png_handler.h"
#include "thumbnail_canvas.h"
#include <iostream>

using namespace std;


ImageFileHandler* CreatePngHandler()
{
    //GerberRenderer * renderer = new GerberRenderer(_T(""));
    //return renderer;
    //cout << "Created PNG handler" << endl;
    return new PngHandler();
}

bool reg_PngHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("PNG"), _T("png"), _T("Lossless compressed image."),       CAN_VIEW_IMAGE);
bool reg_GifHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("GIF"), _T("gif"), _T("Lossless 8-bit compressed image."), CAN_VIEW_IMAGE);
bool reg_BmpHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("BMP"), _T("bmp"), _T("Lossless image."),                  CAN_VIEW_IMAGE);

bool PngHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxImage   image;
    bool      returnValue;

    if (image.LoadFile(fileName))
    {
        thumbnail.SetImage(image);
        image.Destroy();
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    thumbnail.FinishedLoading();
    return returnValue;
}

int PngHandler::LoadImage(wxString fileName)
{
    return LOAD_IMAGE;
}

