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
#include <wx/dynlib.h>
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


wxDynamicLibrary library_glew32_dll;

bool Image_BrowserApp::OnInit()
{    
    CommandLineArguments commandLineArguments(argc, argv);                                          // Parse command line arguments
                                                                                                    // ----------------------------

    wxString configDirectoryParameter = commandLineArguments.GetParameter("-CD");
    wxString configDirectory = (GetConfigFilePath() + CONFIG_FILE_NAME).ToStdString();              // Assume config.txt directory is local
    if (configDirectoryParameter.Length() > 0)                                                      // If a directory was specified on the command line, 
    {
        //cout << "config Directory = " << configDirectoryParameter << endl;
        configDirectory = configDirectoryParameter;                                                 // ... then use that instead.
    }
    else
    {
        //cout << "No config directory specified" << endl;
    }

    //cout << "using: " << configDirectory << endl;
    configParser.LoadConfigFile(configDirectory.ToStdString());


    //cout << "currentDirectory = " << configParser.GetString("currentDirectory") << endl;

    wxString currentDirectory;                                                                      // Image directory    
    bool success = commandLineArguments.GetPath(currentDirectory);
    //cout << "currentDirectory = " << currentDirectory  << " success=" << success << endl;
    if (success)
    {
        wxFileName fileName = currentDirectory; // .ToStdString();
        configParser.SetString("currentDirectory", fileName.GetPath().ToStdString());


    }


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


CommandLineArguments::CommandLineArguments(int argc, wxCmdLineArgsArray& argv)
{
    bool     makingPath         = true;
    bool     parameterNameFound = false;
    wxString parameterName;
    wxString parameterValue;
    wxString pathAccumulator;

    for (int i = 1; i < argc; i++)
    {
        wxString parameter = argv[i];

        if (parameter.StartsWith("-"))
        {
            parameterName      = argv[i];
            parameterNameFound = true;
            makingPath         = false;
        }
        else
        {
            if (parameterNameFound)
            {
                parameterValue = argv[i];
                parameters.push_back(parameterName);
                parameters.push_back(parameterValue);
                cout << "Parameter: " << parameterName << " = " << parameterValue << endl;
            }
        }

        if (makingPath)
        {
            pathAccumulator += parameter;
        }
        else
        {
            //cout << "parameter: " << parameter << endl;
        }
    }

    path = pathAccumulator;
    cout << "Path = " << pathAccumulator << endl;

}

wxString CommandLineArguments::GetParameter(const wxString parameterName) const
{
    int n = parameters.size();

    for (int i = 0; i < n; i+=2)
    {
        cout << "Checking " << parameters[i] << endl;
        if (parameters[i] == parameterName)
        {
            cout << "Found " << parameters[i + 1] << endl;
            return (parameters[i + 1]);
        }
    }
    return "";
}

bool CommandLineArguments::GetPath(wxString& pathReturn)
{
    if (path.DirExists())
    {
        pathReturn = path.GetPath();
        return true;
    }

    return false;
}
