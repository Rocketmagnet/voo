
#include "png_handler.h"
#include "thumbnail_canvas.h"



ImageFileHandler* CreatePngHandler()
{
    //GerberRenderer * renderer = new GerberRenderer(_T(""));
    //return renderer;
    return new PngHandler();
}

bool reg_PngHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("PNG"), _T("png"), _T("Lossless compressed image."));
bool reg_GifHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("GIF"), _T("gif"), _T("Lossless 8-bit compressed image."));
bool reg_BmpHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePngHandler, _T("BMP"), _T("bmp"), _T("Lossless image."));

void PngHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxImage   image;

    if (image.LoadFile(fileName))
    {
        thumbnail.SetImage(image);
        image.Destroy();
    }
    thumbnail.FinishedLoading();
}

int PngHandler::LoadImage(wxString fileName)
{
    return LOAD_IMAGE;
}

