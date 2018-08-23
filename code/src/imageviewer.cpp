/////////////////////////////////////////////////////////////////////////////
// Name:        imageviewer.cpp
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     05/01/2018 22:27:05
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 05/01/2018 22:27:05

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

#include <wx/app.h>
#include "image_browserapp.h"
#include "imageviewer.h"
#include "file_name_list.h"
#include "status_bar.h"
#include "thumbnail_canvas.h"
#include <iostream>

wxDECLARE_APP(Image_BrowserApp);

using namespace std;

////@begin XPM images

////@end XPM images


/*
 * ImageViewer type definition
 */

IMPLEMENT_CLASS( ImageViewer, wxFrame )
BEGIN_EVENT_TABLE( ImageViewer, wxFrame )
    EVT_KEY_DOWN(ImageViewer::OnKeyDown)
    EVT_KEY_UP(  ImageViewer::OnKeyUp)
    //EVT_IDLE(ImageViewer::OnIdle)
    EVT_TIMER(IMAGE_VIEWER_TIMER_ID, ImageViewer::OnTimer)
    EVT_MOUSEWHEEL(ImageViewer::OnMouseWheel)
    EVT_CLOSE(ImageViewer::OnClose)
END_EVENT_TABLE()


/*
 * ImageViewer constructors
 */

ImageViewer::ImageViewer()
    : glPanel(0),
    fileNameList(0),
    currentImage(0),
    displayNumber(-1),
    disappearState(DISAPPEAR_STATE_NONE)
{
    Init();
}

ImageViewer::ImageViewer(ThumbnailCanvas* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
: glPanel(),
  fileNameList(0),
  currentImage(0),
  displayNumber(-1),
  myParent(parent),
  disappearState(DISAPPEAR_STATE_NONE)
{
    Init();

    //wxSize sz(wxSystemSettings::GetMetric(wxSYS_SCREEN_X)+16, wxSystemSettings::GetMetric(wxSYS_SCREEN_Y));
    wxSize sz(400,400);
    Create(parent, id, caption, wxPoint(-8, -8), sz, style);

    wxTextCtrl* dropTarget = new wxTextCtrl(this, wxID_ANY, _("Drop files onto me!"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    dropTarget->DragAcceptFiles(true);

    //wxSize sz(400,400);
    //Create(parent, id, caption, wxPoint(0, 0), sz, style);

}


/*
 * ImageViewer creator
 */


static int32_t emptyMask[] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF,
                               0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF};

bool ImageViewer::Create(ThumbnailCanvas* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ImageViewer creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
    Layout();
    wxBitmap bitmap((char*)emptyMask, 32, 32);
    wxBitmap   mask((char*)emptyMask, 32, 32);

    bitmap.SetMask(new wxMask(mask));
    wxImage hiddenImage = bitmap.ConvertToImage();
    hiddenImage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, 16);
    hiddenImage.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, 16);
    wxCursor hiddenCursor = wxCursor(hiddenImage);

    SetCursor(hiddenCursor);
    //Centre();
////@end ImageViewer creation
    return true;
}


/*
 * ImageViewer destructor
 */

ImageViewer::~ImageViewer()
{
////@begin ImageViewer destruction
////@end ImageViewer destruction
}


/*
 * Member initialisation
 */

void ImageViewer::Init()
{
////@begin ImageViewer member initialisation
    keys.resize(512, 0);
////@end ImageViewer member initialisation
}


/*
 * Control creation for ImageViewer
 */

void ImageViewer::CreateControls()
{    
    ImageViewer* itemFrame1 = this;
    
    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);

    SetSizer(sizer);

    glPanel = new BasicGLPanel(this, 0);
    //textCtrl = new wxTextCtrl(this, -1, wxT("Text Info"), wxDefaultPosition, wxSize(400, 1000), wxTE_MULTILINE|wxTE_READONLY);
    textCtrl = 0;

    sizer->Add(glPanel,  1, wxEXPAND);
    //sizer->Add(textCtrl, .1, wxEXPAND);
    SetBackgroundColour(wxColor(0, 0, 0));
    Show(false);

    timer.SetOwner(this, IMAGE_VIEWER_TIMER_ID);

    //wxAcceleratorEntry entries[1];
    //entries[0].Set(wxACCEL_CTRL, (int) 'D', ID_DELETE_DIRECTORY);
    //wxAcceleratorTable accel(1, entries);
    //this->SetAcceleratorTable(accel);
}


/*
 * Should we show tooltips?
 */

bool ImageViewer::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap ImageViewer::GetBitmapResource( const wxString& name )
{
    wxUnusedVar(name);
    return wxNullBitmap;
}

/*
 * Get icon resources
 */

wxIcon ImageViewer::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ImageViewer icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ImageViewer icon retrieval
}

void ImageViewer::ClearKeys()
{
    int i, n = keys.size();

    for (i = 0; i < n; i++)
    {
        keys[i] = 0;
    }
}

void ImageViewer::OnIdle(wxIdleEvent &event)
{
	if (!IsShown())
		return;

    //cout << "ImageViewer::OnIdle" << endl;
    float dx = 0, dy = 0;

    if (keys['E'])        {glPanel->ZoomIn();       }
    if (keys['Q'])        {glPanel->ZoomOut();      }
    if (keys['W'] || keys[WXK_UP   ]) { dy += 10.0; }
    if (keys['S'] || keys[WXK_DOWN ]) { dy -= 10.0; }
    if (keys['A'] || keys[WXK_LEFT ]) { dx += 10.0; }
    if (keys['D'] || keys[WXK_RIGHT]) { dx -= 10.0; }

    if ((dx!=0) || (dy!=0))
        glPanel->MoveRel(dx, dy);

    glPanel->Render(GL_PANEL_RENDER_IMAGE);

    if (displayNumber > -1)
    {
        //cout << "displayNumber " << displayNumber << endl;
        glPanel->DisplayImage(displayNumber);
        displayNumber = -1;
    }
    event.RequestMore();
}


// Called by ThumbnailCanvas whenever an image is requested
// to be displayed.
// 
void ImageViewer::DisplayImage(int imageNumber)
{
    TEXT_MSG("ImageViewer::DisplayImage(%d)\n", imageNumber);

    if (imageNumber < 0)
        return;

    if (imageNumber > fileNameList->MaxFileNumber())
        return;

    currentImage = imageNumber;

    if (glPanel)
    {
        TEXT_MSG("  Display\n");
        Show(true);
        ////ShowFullScreen(true);
        displayNumber = imageNumber;
        SetFocus();
        Refresh();
        timer.Start(10);
        disappearState = DISAPPEAR_STATE_NONE;
    }

}


void ImageViewer::Disappear()
{
    glPanel->Clear();
    currentImage = -1;
    ClearKeys();
    myParent->SetCursor(glPanel->GetImageNumber());
    //Show(false);
    myParent->SetFocus();
    disappearState = DISAPPEAR_STATE_NONE;
    //ShowFullScreen(false);
}

wxLongLong keyTime = 0;

void ImageViewer::OnKeyDown(wxKeyEvent &event)
{
    wxLongLong kt = wxGetLocalTimeMillis();
    if (keyTime != 0)
    {
        cout << "keyTime = " << keyTime - kt << endl;
    }
    keyTime = kt;


	lastKeyCode = event.GetKeyCode();
	//cout << "Key: " << event.GetKeyCode() << endl;
    switch (event.GetKeyCode())
    {
    case WXK_DELETE:
        if (!NextImage())
        {
            if (!PrevImage())
            {
                Disappear();
            }
        }
        myParent->DeleteImage(currentImage);
        break;

    case WXK_ESCAPE:
	case WXK_RETURN:
        disappearState = DISAPPEAR_STATE_REQUESTED;
        break;

    default:
            if (event.GetKeyCode() < keys.size())
                keys[event.GetKeyCode()] = 1;
        break;
    }
}


void ImageViewer::OnKeyUp(wxKeyEvent &event)
{
    //cout << "Frame Key Up" << event.GetKeyCode() << endl;
    switch (event.GetKeyCode())
    {
    case WXK_ESCAPE:
        //cout << "Escape" << endl;
        ClearKeys();
        //Show(false);
        //ShowFullScreen(false);
        break;

    case WXK_HOME:
        HomeImage();
        break;

    case WXK_END:
        EndImage();
        break;

    case 'R':
    case WXK_PAGEUP:
        PrevImage();
        break;

    case 'F':
    case WXK_PAGEDOWN:
        NextImage();
        break;

    default:
        if (event.GetKeyCode() < keys.size())
            keys[event.GetKeyCode()] = 0;
        break;
    }
}

void ImageViewer::OnClose(wxCloseEvent &event)
{
    wxGetApp().ExitMainLoop();
    /*
    cout << "ImageViewer::OnClose()" << endl;

    if (closeEnabled)
    {
        cout << "  closing me" << endl;
        Destroy();
    }
    else
    {
        cout << "  closing parent" << endl;
        GetParent()->Close();
    }
    */
    //event.veto();
    //event.Skip();
}

void ImageViewer::OnMouseWheel(wxMouseEvent &event)
{
    //cout << "Frame Wheel" << endl;
    int newImage = currentImage;

    if (event.GetWheelRotation() < 0)
    {
        PrevImage();
    }

    if (event.GetWheelRotation() > 0)
    {
        NextImage();
    }

    event.Skip();
}

void ImageViewer::HomeImage()
{
    //cout << "HomeImage" << endl;
    int newImage = 0;
    
    if (newImage > fileNameList->MaxFileNumber())
        newImage = fileNameList->MaxFileNumber();

    if (currentImage != newImage)
    {
        DisplayImage(newImage);
        Refresh();
    }
}

void ImageViewer::EndImage()
{
    //cout << "HomeImage" << endl;
    int newImage = fileNameList->MaxFileNumber();

    if (currentImage != newImage)
    {
        DisplayImage(newImage);
        Refresh();
    }
}

bool ImageViewer::NextImage()
{
    int newImage = currentImage;
    newImage++;

    if (newImage > fileNameList->MaxFileNumber())
        newImage = fileNameList->MaxFileNumber();

    if (currentImage != newImage)
    {
        DisplayImage(newImage);
        Refresh();
        return true;
    }
    else
    {
        return false;
    }
}


bool ImageViewer::PrevImage()
{
    int newImage = currentImage;

    newImage--;
    if (newImage < 0)
        newImage = 0;

    if (currentImage != newImage)
    {
        DisplayImage(newImage);
        Refresh();
        return true;
    }
    else
    {
        return false;
    }
}

wxLongLong onTimer = 0;

void NoteTime(wxString s)
{
    static wxULongLong timePrev;
    wxULongLong timeNow;

    timeNow = wxGetLocalTimeMillis();

    cout << "TIME NOTE: " << s.c_str() << " " << timeNow - timePrev << endl;
    timePrev = timeNow;
}

void ImageViewer::OnTimer(wxTimerEvent &event)
{
    //wxLongLong t = wxGetLocalTimeMillis();
    //TEXT_MSG("ImageViewer::OnTimer() %d", (t-onTimer).GetLo());
    //cout << "ImageViewer::OnTimer() " << (t - onTimer).GetLo() << endl;
    //onTimer = t;

    //NoteTime(wxT("ImageViewer::OnTimer"));

    if (!IsShown())
    {
        //TEXT_MSG(" No\n");
        return;
    }

    //cout << "ImageViewer::OnIdle" << endl;
    float dx = 0, dy = 0;

    if (keys['E'])                      { glPanel->ZoomIn(); }
    if (keys['Q'])                      { glPanel->ZoomOut(); }
    if (keys['W'] || keys[WXK_UP]   )   { dy += 10.0; }
    if (keys['S'] || keys[WXK_DOWN] )   { dy -= 10.0; }
    if (keys['A'] || keys[WXK_LEFT] )   { dx += 10.0; }
    if (keys['D'] || keys[WXK_RIGHT])   { dx -= 10.0; }

    if ((dx != 0) || (dy != 0))
        glPanel->MoveRel(dx, dy);

    if (displayNumber > -1)
    {
        //cout << "displayNumber " << displayNumber << endl;
        glPanel->DisplayImage(displayNumber);
        displayNumber = -1;
    }
    //NoteTime(wxT("  display"));

    cout << "disappearState = " << disappearState << endl;

    switch (disappearState)
    {
    case DISAPPEAR_STATE_NONE:
        if (currentImage >= 0)
        {
            glPanel->Render(GL_PANEL_RENDER_IMAGE);
        }
        break;

    case DISAPPEAR_STATE_REQUESTED:
        glPanel->Render(GL_PANEL_BLANK_SCREEN);
        disappearState = DISAPPEAR_STATE_CLOSING;
        break;

    case DISAPPEAR_STATE_CLOSING:
        Disappear();
        disappearState = DISAPPEAR_STATE_CLOSED;
        break;
    }
    //NoteTime(wxT("  render"));
}

