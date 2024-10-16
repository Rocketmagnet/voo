/////////////////////////////////////////////////////////////////////////////
// Name:        imageviewer.h
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     05/01/2018 22:27:05
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 05/01/2018 22:27:05

#ifndef _IMAGEVIEWER_H_
#define _IMAGEVIEWER_H_

class FileNameList;
class ThumbnailCanvas;
class ImageBrowser;
class ConfigParser;


#include "wx/frame.h"
#include "wx/image.h"
#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/timer.h"
#include "wx/textctrl.h"
#include <vector>

#include "thumbnail_canvas.h"
#include "gl_panel.h"

//class JpegGpu;

////@begin control identifiers
#define ID_IMAGEVIEWER        10000
#define ID_IMAGEPANEL         10001
#define IMAGE_VIEWER_TIMER_ID 10002
#define SYMBOL_IMAGEVIEWER_STYLE wxCAPTION|wxRESIZE_BORDER|wxSYSTEM_MENU|wxCLOSE_BOX
#define SYMBOL_IMAGEVIEWER_TITLE _("ImageViewer")
#define SYMBOL_IMAGEPANEL_TITLE  _("ImagePanel")
#define SYMBOL_IMAGEVIEWER_IDNAME ID_IMAGEVIEWER
#define SYMBOL_IMAGEVIEWER_SIZE wxSize(400, 300)
#define SYMBOL_IMAGEVIEWER_POSITION wxDefaultPosition
////@end control identifiers

#define TEXT_MSG(fmt, ...) { wxString s; s.Printf(fmt, __VA_ARGS__); if (textCtrl) textCtrl->AppendText(s);  /*std::cout << "* " << s << std::endl; */}
/*
class ImagePanel : public wxPanel
{
    DECLARE_CLASS(ImagePanel)
    DECLARE_EVENT_TABLE()

public:
    ImagePanel(wxWindow* parent, wxWindowID id = ID_IMAGEPANEL, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    
    void DisplayImage(wxString filename);
    void OnPaint(wxPaintEvent &event);
    void OnMouseWheel(wxMouseEvent &event);
    void OnMouse(wxMouseEvent &event);
    
private:
    wxImage     image;
    wxBitmap    bitmap;
};
*/

enum DISAPPEAR_STATE
{
    DISAPPEAR_STATE_NONE      = 0,
    DISAPPEAR_STATE_REQUESTED = 1,
    DISAPPEAR_STATE_CLOSING   = 2,
    DISAPPEAR_STATE_CLOSED    = 3
};

/*!
 * ImageViewer class declaration
 */

class ImageViewer: public wxFrame
{    
    DECLARE_CLASS( ImageViewer )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ImageViewer();
    ImageViewer(ImageBrowser* parent, wxWindowID id = SYMBOL_IMAGEVIEWER_IDNAME, const wxString& caption = SYMBOL_IMAGEVIEWER_TITLE, const wxPoint& pos = SYMBOL_IMAGEVIEWER_POSITION, const wxSize& size = SYMBOL_IMAGEVIEWER_SIZE, long style = SYMBOL_IMAGEVIEWER_STYLE);
    bool Create(ImageBrowser* parent, wxWindowID id = SYMBOL_IMAGEVIEWER_IDNAME, const wxString& caption = SYMBOL_IMAGEVIEWER_TITLE, const wxPoint& pos = SYMBOL_IMAGEVIEWER_POSITION, const wxSize& size = SYMBOL_IMAGEVIEWER_SIZE, long style = SYMBOL_IMAGEVIEWER_STYLE);

    /// Destructor
    ~ImageViewer();

    /// Initialises member variables
    void Init();
    void SetThumbnailCanvas(ThumbnailCanvas *tnc)
    {
        if (glPanel)
            glPanel->SetThumbnailCanvas(tnc);
;        thumbnailCanvas = tnc;
    }

    //void SetStatusBar(wxStatusBar            *sBar) { statusBar = sBar; }
    /// Creates the controls and sizers
    void CreateControls();

    //void SetFileNameList(FileNameList *fnl)
    //{
    //    fileNameList = fnl;
    //    glPanel->SetFileNameList(fnl);
    //}

    void DisplayImage(wxFileName fileName);
    //void DisplayImage(int imageNumber);
    void OnKeyUp(wxKeyEvent &event);
    void OnKeyDown(wxKeyEvent &event);
    void OnMouseWheel(wxMouseEvent &event);
    void OnMouse(wxMouseEvent &event);
    void OnMouseLDClick(wxMouseEvent& event);
    void OnIdle(wxIdleEvent &event);
    void OnClose(wxCloseEvent &event);
    void EnableClose() { closeEnabled = true; }
    void OnTimer(wxTimerEvent &event);

    void ResetZoom();

    void ClearCache();
    bool NextImage(int jump = 1);
    bool PrevImage(int jump = 1);
    void HomeImage();
    void  EndImage();
    void Disappear();
    void AddViewableExtensions(wxString extensions)
    {
        for (auto extension : extensions)
        {
            wxString e = extension;
            e.MakeLower();
            viewableExtensions.Append(e);
        }
    }

	int GetLastKeyCode() { return lastKeyCode; }
    ////@begin ImageViewer event handler declarations
////@end ImageViewer event handler declarations

////@begin ImageViewer member function declarations
    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );
////@end ImageViewer member function declarations

    /// Should we show tooltips?
    static bool ShowToolTips();
    void ClearKeys();

    BasicGLPanel            *glPanel;
    //FileNameList            *fileNameList;    // No. This was a bad idea
	int						 lastKeyCode;
    //size_t                   currentImageNumber;
    float                    zoomRate;
    std::vector<char>        keys;
    //int                      displayNumber;       // File to load and display, when the viewer becomes visible.
    wxFileName               displayThisFileName;
    wxFileName               currentImageFileName;
    bool                     closeEnabled;
    DISAPPEAR_STATE          disappearState;
    wxTimer                  timer;
    wxTextCtrl              *textCtrl;
    ThumbnailCanvas         *thumbnailCanvas;
    ImageBrowser            *imageBrowser;
    wxString                 videoFileExtensions;
    wxString                 videoPlayerPath;
    wxString                 viewableExtensions;
    //JpegGpu                 *jpegGpu;
};

#endif
