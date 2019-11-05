
#include "video_handler.h"
#include "thumbnail_canvas.h"
#include <iostream>
using namespace std;

ImageFileHandler* CreateVideoHandler()
{
    return new VideoHandler();
}

bool reg_VideoHandler1 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mp4"),  _T("Video file."));
bool reg_VideoHandler2 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mpg"),  _T("Video file."));
bool reg_VideoHandler3 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("mpeg"), _T("Video file."));
bool reg_VideoHandler4 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("Windows media video"), _T("wmv"),  _T("Video file."));
bool reg_VideoHandler5 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("AVI"),                 _T("avi"),  _T("Video file."));
bool reg_VideoHandler6 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("flv"),  _T("Video file."));
bool reg_VideoHandler7 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("MPEG Layer 4"),        _T("rmvb"), _T("Video file."));
bool reg_VideoHandler8 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("RealMedia"),           _T("rm"),   _T("Video file."));
bool reg_VideoHandler9 = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateVideoHandler, _T("Apple QuickTime"),     _T("mov"), _T("Video file."));

void VideoHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{

    wxLogNull logNo;													// logging is suspended while this object is in scope
    wxSize tnSize = thumbnail.GetSize();
    tnSize *= 4;
    wxImage   image(tnSize, true);

    LONGLONG pos = 300000000L;
    const wchar_t *fn = fileName.wc_str();

    char *id = (char*)image.GetData();
    videoThumbnailReader.OpenFile(fn);
    videoThumbnailReader.CreateBitmap(id, tnSize.GetX(), tnSize.GetY(), pos);
    wxSize newSize(videoThumbnailReader.GetNewWidth(), videoThumbnailReader.GetNewHeight());

    if (newSize.GetX()*newSize.GetY() > 0)
    {
        image.Resize(newSize, wxPoint(0, 0));

        newSize /= 4;

        image.Rescale(newSize.GetX(), newSize.GetY(), wxIMAGE_QUALITY_HIGH);
        thumbnail.SetImage(image);
    }
    image.Destroy();

    thumbnail.FinishedLoading();
}

int VideoHandler::LoadImage(wxString fileName)
{
    cout << "VideoHandler::LoadImage(" << fileName << ")" << endl;

    wxString command = "C:\\Program Files\\MPC-HC\\mpc-hc64.exe ";
    command += wxT(" \"") + fileName + wxT("\"");;

    cout << command << endl;
    wxExecute(command.c_str(), wxEXEC_ASYNC, NULL);
    
    return DO_NOTHING;
}

