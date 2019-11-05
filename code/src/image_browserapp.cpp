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
#include "wx/dir.h"
#include "wx/stdpaths.h"
#include <iostream>

using namespace std;

#define PRIVATE_DIRS_FILE_NAME     "prvdirs.txt"
#define CONFIG_FILE_NAME           "config.txt"
#define DIRFLAGS_CONTAINS_FILES    1


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


wxString GetConfigFilePath()
{
    wxFileName exePath = wxStandardPaths::Get().GetExecutablePath();

    return exePath.GetPath() + wxT("\\");
}

/*
 * Constructor for Image_BrowserApp
 */
Image_BrowserApp::Image_BrowserApp()
 : configParser((GetConfigFilePath()+CONFIG_FILE_NAME).ToStdString())
{
    cout << "Num Args = " << wxApp::argc << endl;
    Init();
}



/*
 * Member initialisation
 */

void Image_BrowserApp::Init()
{
    //cout << "Image_BrowserApp::Init()" << endl;
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
    //cout << "Image_BrowserApp::OnInit()" << endl;

    int n = argc;

    wxString passedArgument;

    for (int i = 1; i < n; i++)
        passedArgument += argv[i];
    cout << "passedArgument = " << passedArgument << endl;



    if (n >= 2)
    {
        wxFileName filename(passedArgument);

        cout << filename.IsDir() << filename.DirExists() << filename.FileExists() << endl;

        if (filename.IsOk())
        {
            if (filename.DirExists())
            {
                cout << "Passed Directory " << passedArgument << endl;
                configParser.SetString("currentDirectory", std::string(passedArgument));
            }
            if (filename.FileExists())
            {
                cout << "Passed file " << passedArgument << endl;
                configParser.SetString("currentDirectory", std::string(filename.GetPath()));
            }
            else
            {
                cout << "Unknown parameter " << passedArgument << endl;
            }
        }
    }

    //wxApp::OnInit();
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
	imageBrowser = new ImageBrowser( this, -1, _T("Image Browser"), wxPoint(700,0), wxSize(1200, 640));
    imageBrowser->Show(true);


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


