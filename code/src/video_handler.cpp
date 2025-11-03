
#include "video_handler.h"
#include "thumbnail_canvas.h"
#include <iostream>
#include "thumbnail_cache.h"

using namespace std;

ImageFileHandler* CreateVideoHandler()
{
    cout << "CreateVideoHandler()" << endl;
    return new VideoHandler();
}

wxFileName VideoHandler::playerPath;

bool reg_VideoHandler1 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mp4"),  _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler2 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mpg"),  _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler3 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mpeg"), _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler4 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("Windows media video"), _T("wmv"),  _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler5 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("AVI"),                 _T("avi"),  _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler6 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("flv"),  _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler7 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("rmvb"), _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler8 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("RealMedia"),           _T("rm"),   _T("Video file."), CANNOT_VIEW_IMAGE);
bool reg_VideoHandler9 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("Apple QuickTime"),     _T("mov"),  _T("Video file."), CANNOT_VIEW_IMAGE);

int tnNum = 0;

bool VideoHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxSize    tnSize = thumbnail.GetSize();
    
    wxSize largerSize = tnSize *= 4;
    wxImage image(largerSize, true);

    LONGLONG pos      = 300000000L;
    wxString longPath = fileName;

    const wchar_t *fn = longPath.wc_str();
    char *imageData = (char*)image.GetData();

    videoThumbnailReader.OpenFile(fn);
    bool success = videoThumbnailReader.CreateBitmap(imageData, largerSize.GetX(), largerSize.GetY(), pos);

    if (success)
    {
        wxSize newSize = videoThumbnailReader.GetSize();
        wxSize finalSize = newSize / 4;

        if (newSize.GetX() * newSize.GetY() > 0)
        {
            image.Resize(newSize, wxPoint(0, 0));
            image.Rescale(finalSize.GetX(), finalSize.GetY(), wxIMAGE_QUALITY_NORMAL);
            thumbnail.SetImage(image);
        }
    }

    image.Destroy();
    thumbnail.FinishedLoading();

    return true;
}

int VideoHandler::LoadImage(wxString fileName)
{
    //cout << "VideoHandler::LoadImage(" << fileName << ")" << endl;

    wxString command = "C:\\Program Files\\MPC-HC\\mpc-hc64.exe";
    command += wxT(" \"") + fileName + wxT("\"");;

    //cout << command << endl;
    wxExecute(command.c_str(), wxEXEC_ASYNC, NULL);

    return DO_NOTHING;
}

