/////////////////////////////////////////////////////////////////////////////
// Name:        image_browserapp.cpp
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     27/11/2017 14:59:37
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
////@end includes

#include "image_browserapp.h"
#include "imagebrowser.h"
#include "imageviewer.h"
#include <iostream>

using namespace std;

////@begin XPM images
////@end XPM images


/*
 * Application instance implementation
 */

////@begin implement app
IMPLEMENT_APP( Image_BrowserApp )
////@end implement app


/*
 * Image_BrowserApp type definition
 */

IMPLEMENT_CLASS( Image_BrowserApp, wxApp )


/*
 * Image_BrowserApp event table definition
 */

BEGIN_EVENT_TABLE( Image_BrowserApp, wxApp )
    EVT_MENU(ID_DELETE_DIRECTORY, Image_BrowserApp::OnDeleteDirectory)
END_EVENT_TABLE()


/*
 * Constructor for Image_BrowserApp
 */

Image_BrowserApp::Image_BrowserApp()
{
    Init();
}


/*
 * Member initialisation
 */

void Image_BrowserApp::Init()
{
////@begin Image_BrowserApp member initialisation
////@end Image_BrowserApp member initialisation
}

/*
 * Initialisation for Image_BrowserApp
 */

bool Image_BrowserApp::OnInit()
{    
////@begin Image_BrowserApp initialisation
	// Remove the comment markers above and below this block
	// to make permanent changes to the code.

#if wxUSE_XPM
	wxImage::AddHandler(new wxXPMHandler);
#endif
#if wxUSE_LIBPNG
	wxImage::AddHandler(new wxPNGHandler);
#endif
#if wxUSE_LIBJPEG
	//wxImage::AddHandler(new wxJPEGHandler);
#endif
#if wxUSE_GIF
	wxImage::AddHandler(new wxGIFHandler);
#endif
	imageBrowser = new ImageBrowser( NULL, -1, _T("Image Browser"), wxDefaultPosition, wxSize(1024, 640));
    imageBrowser->Show(true);

    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_CTRL, (int) 'D', ID_DELETE_DIRECTORY);
    //entries[1].Set(wxACCEL_CTRL, (int) 'X', wxID_EXIT);
    //entries[2].Set(wxACCEL_SHIFT, (int) 'A', ID_ABOUT);
    //entries[3].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_CUT);
    wxAcceleratorTable accel(1, entries);
    imageBrowser->SetAcceleratorTable(accel);

////@end Image_BrowserApp initialisation

    return true;
}


/*
 * Cleanup for Image_BrowserApp
 */

int Image_BrowserApp::OnExit()
{    
////@begin Image_BrowserApp cleanup
	return wxApp::OnExit();
////@end Image_BrowserApp cleanup
}

bool RemoveDirectory(wxString pathName)
{
    wxString fn;
    wxDir dir(pathName);
    bool cont = dir.GetFirst(&fn);

    cout << "RemoveDirectory(" << pathName << ")" << endl;

    // if there are files to process
    if (cont)
    {
        do {
            // if the next filename is actually a directory
            wxString subPath = dir.GetName() + wxFILE_SEP_PATH + fn + wxFILE_SEP_PATH;

            cout << "Checking for existence of " << subPath << " " << wxDirExists(subPath) << endl;

            if (wxDirExists(subPath))
            {
                cout << "deleting " << subPath << endl;
                // delete this directory
                RemoveDirectory(subPath);
            }
            else
            {
                // otherwise attempt to delete this file
                cout << "Attempt to remove file " << pathName + fn << endl;
                if (!wxRemoveFile(pathName + fn))
                {
                    // error if we couldn't
                    //wxLogError("Could not remove file \"" + pathName + fn + "\"");
                    return false;
                }
            }
        }
        // get the next file name
        while (dir.GetNext(&fn));
    }

    cout << "Now remove directory " << pathName << endl;
    bool success = wxRmDir(pathName);
    cout << "success = " << success << endl;
    return true;
}

bool DeleteDirectory(wxString path)
{


    wxMessageDialog *test = new wxMessageDialog(nullptr, wxT("Delete ") + path, wxT("Warning"), wxYES_NO | wxNO_DEFAULT);
    int ret = test->ShowModal();

    //cout << "Returned " << ret << endl;

    if (ret == wxID_YES)
    {
        RemoveDirectory(path);
        return true;
    }
    else
    {
        return false;
    }
}

void Image_BrowserApp::OnDeleteDirectory(wxCommandEvent &event)
{
    wxString path = imageBrowser->GetCurrentDir();
    bool success = DeleteDirectory(path);

    if (success)
    {
        //wxFileName dir(path);
        //dir.RemoveLastDir();
        imageBrowser->DirectoryWasDeleted(path);
    }
}
