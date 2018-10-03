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
	imageBrowser = new ImageBrowser( NULL, -1, _T("Image Browser"), wxPoint(700,0), wxSize(1200, 640));
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


