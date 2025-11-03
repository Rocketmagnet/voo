/////////////////////////////////////////////////////////////////////////////
// Name:        image_browserapp.h
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     27/11/2017 14:59:37
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

#ifndef _IMAGE_BROWSERAPP_H_
#define _IMAGE_BROWSERAPP_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/image.h"
#include "imagebrowser.h"
#include <wx/filename.h>

////@end includes

/*!
 * Forward declarations
 */

////@begin forward declarations
////@end forward declarations

/*!
 * Control identifiers
 */

////@begin control identifiers
////@end control identifiers

/*!
 * Image_BrowserApp class declaration
 */

class CommandLineArguments
{
public:
    CommandLineArguments(int argc, wxCmdLineArgsArray& argv);

    wxString GetParameter(const wxString parameterName) const;

    bool     HasFileName();
    wxString GetFileName();
    bool     GetPath(wxString &pathReturn);

private:
    wxFileName              path;
    std::vector<wxString>   parameters;
};



class Image_BrowserApp: public wxApp
{    
    DECLARE_CLASS( Image_BrowserApp )
    ///DECLARE_EVENT_TABLE()

public:
    /// Constructor
    Image_BrowserApp();

    void Init();

    /// Initialises the application
    virtual bool OnInit();

    /// Called on exit
    virtual int OnExit();

    //ConfigParser* GetConfigParser() {return &configParser;}
    void          RegisterConfigChange();

private:
    ImageBrowserFrame  *imageBrowserFrame;
    //ConfigParser        configParser;



////@begin Image_BrowserApp event handler declarations

////@end Image_BrowserApp event handler declarations

////@begin Image_BrowserApp member function declarations

////@end Image_BrowserApp member function declarations

////@begin Image_BrowserApp member variables
////@end Image_BrowserApp member variables
};



/*!
 * Application instance declaration 
 */

////@begin declare app
DECLARE_APP(Image_BrowserApp)
////@end declare app

#endif
    // _IMAGE_BROWSERAPP_H_
