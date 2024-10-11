
#include "partial_handler.h"
#include "thumbnail_canvas.h"
#include "vector_renderer.h"
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <iostream>
using namespace std;

wxString partialThumb = wxT("P(64,64,64) B(64,64,64) R(0,0,1,1) P(120,120,120,2) B(160,160,160) R(0.19,0.08,0.61,0.83) P(120,0,0,1) B(240,0,0) R(0.17,0.23,0.42,0.19) P(255,255,255,1) T(0.26,0.25,$$EXT$$) X");

ImageFileHandler* CreatePartialHandler()
{
    return new PartialHandler();
}

bool reg_PartialHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreatePartialHandler, _T("Partial"), _T("part"), _T("Partially downloaded file"), CANNOT_VIEW_IMAGE);

bool PartialHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    
    wxBitmap  &bitmap(thumbnail.GetBitmapReference());
    wxFileName path(fileName);
    wxString extension = path.GetExt();
    extension.MakeUpper();
    bitmap.Create(thumbnail.GetSize());
    VectorRenderer vr;
    bitmap.Create(thumbnail.GetSize());
    wxString program = partialThumb;
    program.Replace(wxT("$$EXT$$"), extension);
    vr.Render(program, bitmap);

    thumbnail.FinishedLoading();

    return true;
}


int PartialHandler::LoadImage(wxString fileName)
{
    return 0;
}

