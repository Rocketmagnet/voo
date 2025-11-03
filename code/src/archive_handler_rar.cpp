
#include "archive_handler_rar.h"
#include "thumbnail_canvas.h"
#include "vector_renderer.h"
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <iostream>
using namespace std;

wxString archiveThumbRar = wxT("P(64,64,64) B(64,64,64) R(0,0,1,1) P(160,160,160,2) B(250,250,250) R(0.19,0.08,0.61,0.83) P(0,95,120,1) B(0,190,240) R(0.17,0.23,0.42,0.19) P(255,255,255,1) T(0.26,0.25,$$EXT$$) X");

ImageFileHandler* CreateRarHandler()
{
    //cout << "Created RAR handler" << endl;
    return new RarHandler();
}


bool reg_RarHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateRarHandler, _T("RAR"), _T("rar"), _T("Compressed Archive"), CANNOT_VIEW_IMAGE);

bool RarHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    
    wxBitmap  &bitmap(thumbnail.GetBitmapReference());
    wxFileName path(fileName);
    wxString extension = path.GetExt();
    extension.MakeUpper();
    bitmap.Create(thumbnail.GetSize());
    VectorRenderer vr;
    bitmap.Create(thumbnail.GetSize());
    wxString program = archiveThumbRar;
    program.Replace(wxT("$$EXT$$"), extension);
    vr.Render(program, bitmap);

    thumbnail.FinishedLoading();

    return true;
}

//void CreateDirectoryInParts(wxString original, wxString newDirectories)
//{
//    wxFileName path(original);
//    wxFileName newDirs(newDirectories);
//    wxFileName progress(original);
//
//    wxArrayString dirs = newDirs.GetDirs();
//
//    int i, n = dirs.size();
//    for (i = 0; i < n; i++)
//    {
//        progress.AppendDir(dirs[i]);
//        //cout << i << ": " << progress.GetFullPath() << endl;
//        if (!progress.DirExists())
//        {
//            wxFileName::Mkdir(progress.GetFullPath(), wxPATH_MKDIR_FULL);
//            //cout << success << endl;
//        }
//    }
//}


int RarHandler::LoadImage(wxString fileName)
{
    wxFileName fn(fileName);

    wxString command("\"C:\\Program Files\\WinRAR\\unrar.exe\" x \"");

    command += fileName;
    command += "\" \"";
    command += fn.GetPath();
    command += "\"";

    //wxExecute(command, wxEXEC_SYNC | wxEXEC_HIDE_CONSOLE, NULL, NULL);
    wxExecute(command, wxEXEC_SYNC, NULL, NULL);

    return REFRESH_TREE;
}

