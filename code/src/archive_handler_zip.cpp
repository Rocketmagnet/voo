
#include "archive_handler_zip.h"
#include "thumbnail_canvas.h"
#include "vector_renderer.h"
#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <iostream>
using namespace std;

wxString archiveThumb = wxT("P(64,64,64) B(64,64,64) R(0,0,1,1) P(160,160,160,2) B(250,250,250) R(0.19,0.08,0.61,0.83) P(0,95,120,1) B(0,190,240) R(0.17,0.23,0.42,0.19) P(255,255,255,1) T(0.26,0.25,$$EXT$$) X");

ImageFileHandler* CreateZipHandler()
{
    return new ZipHandler();
}


bool reg_ZipHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateZipHandler, _T("ZIP"), _T("zip"), _T("Compressed Archive"));
//bool reg_RarHandler = ImageFileHandlerRegistry::instance().RegisterImageFileHandler(CreateRarHandler, _T("RAR"), _T("rar"), _T("Compressed Archive"));

void ZipHandler::LoadThumbnail(wxString fileName, Thumbnail &thumbnail)
{
    wxLogNull logNo;													// logging is suspended while this object is in scope
    
    wxBitmap  &bitmap(thumbnail.GetBitmapReference());
    wxFileName path(fileName);
    wxString extension = path.GetExt();
    extension.MakeUpper();
    bitmap.Create(thumbnail.GetSize());
    VectorRenderer vr;
    bitmap.Create(thumbnail.GetSize());
    wxString program = archiveThumb;
    program.Replace(wxT("$$EXT$$"), extension);
    vr.Render(program, bitmap);

    thumbnail.FinishedLoading();
}

void CreateDirectoryInParts(wxString original, wxString newDirectories)
{
    wxFileName path(original);
    wxFileName newDirs(newDirectories);
    wxFileName progress(original);

    wxArrayString dirs = newDirs.GetDirs();

    int i, n = dirs.size();
    for (i = 0; i < n; i++)
    {
        progress.AppendDir(dirs[i]);
        //cout << i << ": " << progress.GetFullPath() << endl;
        if (!progress.DirExists())
        {
            wxFileName::Mkdir(progress.GetFullPath(), wxPATH_MKDIR_FULL);
            //cout << success << endl;
        }
    }
}

int ZipHandler::LoadImage(wxString fileName)
{
    wxFileName path(fileName.Upper());

    wxFFileInputStream     *in = new wxFFileInputStream(fileName);
    wxArchiveInputStream   *zip = new wxZipInputStream(*in);

    //wxArchiveInputStream   *zip;
    //
    //if (path.GetExt() == wxT("ZIP"))
    //    zip = new wxZipInputStream(*in);

    //if (path.GetExt() == wxT("RAR"))
    //    zip = new wxRarInputStream(*in);

    wxString target_dir = path.GetPath() + "\\";

    wxArchiveEntry *entry;
    while ((entry = zip->GetNextEntry()) != NULL)
    {
        cout << "----" << entry->GetName() << endl;
        if (entry->IsDir())
        {
            wxLogNull nolog;      // suppress error message if directory already exists  
            wxString dir_path = target_dir + entry->GetName();
            //wxLogMessage( "creating dir: %s", dir_path );
            bool success = wxFileName::Mkdir(dir_path);
        }
        else
        {
            wxFileName dir_path(target_dir + entry->GetName());

            if (!dir_path.DirExists())
            {
                CreateDirectoryInParts(target_dir, entry->GetName());
            }

            wxFileOffset fileSize = entry->GetSize();
            char *buf = new char[fileSize];
            zip->OpenEntry(*entry);
            zip->Read((char*)buf, fileSize);

            //wxLogMessage("creating file: %s %ld", target_dir + entry->GetName(), (long)fileSize );
            wxFile out(target_dir + entry->GetName(), wxFile::write);
            if (out.IsOpened())
            {
                out.Write((char*)buf, fileSize);
                out.Close();
            }
            wxDELETE(entry);
            delete[] buf;
        }
    }

    delete zip;
    delete in;

    //return REFRESH_TREE | DELETE_FILE;
    return REFRESH_TREE;
}

