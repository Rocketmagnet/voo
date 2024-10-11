/////////////////////////////////////////////////////////////////////////////
// Name:        imagebrowser.h
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     27/11/2017 15:00:49
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 27/11/2017 15:00:49

#ifndef _IMAGEBROWSER_H_
#define _IMAGEBROWSER_H_


/*!
 * Includes
 */

////@begin includes
#include "wx/filename.h"
#include "wx/frame.h"
#include "wx/treectrl.h"
#include "config_parser.h"
#include "wx/dnd.h"
//#include <deque>
#include "deque_thread_safe.h"

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
#define ID_IMAGEBROWSER         10000
#define ID_SCROLLEDWINDOW       10001
#define ID_DIRCTRL              10002
#define ID_PANEL                10003
#define ID_STATUSBAR            10004
#define ID_DELETE_DIRECTORY     10005
#define ID_RESCALE_DIRECTORY    10006
#define ID_ARCHIVE_DIRECTORY    10007
#define ID_RANDOM_DIRECTORY     10008
#define ID_MARK_DIRECTORY       10009
#define ID_BACK_DIRECTORY       10010
#define ID_TOUCH_DIRECTORY      10011

#define SYMBOL_IMAGEBROWSER_STYLE       wxDEFAULT_FRAME_STYLE   |   \
                                        wxCAPTION               |   \
                                        wxRESIZE_BORDER         |   \
                                        wxSYSTEM_MENU           |   \
                                        wxMAXIMIZE              |   \
                                        wxMINIMIZE_BOX          |   \
                                        wxMAXIMIZE_BOX          |   \
                                        wxCLOSE_BOX             |   \
                                        wxSIMPLE_BORDER         |   \
                                        wxTAB_TRAVERSAL

#define SYMBOL_IMAGEBROWSER_TITLE _("Image Browser 3")
#define SYMBOL_IMAGEBROWSER_IDNAME ID_IMAGEBROWSER
#define SYMBOL_IMAGEBROWSER_SIZE wxSize(400, 300)
#define SYMBOL_IMAGEBROWSER_POSITION wxDefaultPosition
////@end control identifiers

class wxMenuBar;
class wxMenu;
class wxSplitterWindow;
class wxGenericDirCtrl;
class ThumbnailCanvas;
class ImageViewer;
class Image_BrowserApp;
class FileNameList;

#define ID_DIRECTORY_CTRL	100

void    SetDebuggingText(wxString text);
void AppendDebuggingText(wxString text);
int CompareStringsNatural_(const wxString& _a, const wxString& _b);

class RightHandWindow : public wxWindow
{
public:
    RightHandWindow(wxWindow *parent);

    ThumbnailCanvas* GetThumbnailCanvas()    { return thumbnailCanvas; }
    wxTextCtrl*      GetDirectoryNameCtrl()  { return directoryNameCtrl; }


private:
    ThumbnailCanvas  *thumbnailCanvas;
    wxTextCtrl       *directoryNameCtrl;
    wxBoxSizer       *boxSizer;
};


//! Used to keep track of each image that needs resizing, what what the maximum size should be for each.
struct ResizerEntry
{
    ResizerEntry(wxFileName fn, int xs, int ys)
    : fileName(fn),
      xSize(xs),
      ySize(ys)
    {}

    int        xSize, ySize;
    wxFileName fileName;
};

// Thread that resizes all the images in the queue.
class ImageResizerPermanent : public wxThread
{
public:
    ImageResizerPermanent(deque_thread_safe<ResizerEntry> &entries)
        : wxThread(wxTHREAD_JOINABLE),
        resizerEntries(entries)
    {
    }

    ExitCode Entry();

private:
    void SaveState();
    void LoadState();
    deque_thread_safe<ResizerEntry> &resizerEntries;
};


class ChooseRescaleSize : public wxDialog
{
public:
    ChooseRescaleSize(int xs, int ys);
    int GetWidth();
    int GetHeight();

    wxStaticText   *st;
    wxTextCtrl     *widthCtrl;
    wxTextCtrl     *heightCtrl;
};

class TreeDropTargetHandler : public wxDropTarget
{
public:
    TreeDropTargetHandler ()
        : treeCtrl(0)
    {
        SetDataObject(&fileDataObject);
    }

    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);
    wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult defResult) { return wxDragNone; }
    wxDragResult OnEnter(   wxCoord x, wxCoord y, wxDragResult defResult) { return wxDragNone; }
    bool         OnDrop(wxCoord x, wxCoord y);


    void SetTreeCtrl(wxTreeCtrl* tc) { treeCtrl = tc; }

    wxTreeCtrl         *treeCtrl;
    wxFileDataObject    fileDataObject;
    wxTreeItemId        prevDragItemId;
};


//! ImageBrowser class declaration
class ImageBrowser: public wxFrame
{    
    DECLARE_CLASS( ImageBrowser )
    DECLARE_EVENT_TABLE()

public:
    /// Constructors
    ImageBrowser();
    ImageBrowser(int showImageNumber);

    ImageBrowser(Image_BrowserApp* parent, wxWindowID id = SYMBOL_IMAGEBROWSER_IDNAME, const wxString& caption = SYMBOL_IMAGEBROWSER_TITLE, const wxPoint& pos = SYMBOL_IMAGEBROWSER_POSITION, const wxSize& size = SYMBOL_IMAGEBROWSER_SIZE, long style = SYMBOL_IMAGEBROWSER_STYLE );

    bool Create(Image_BrowserApp* parent, wxWindowID id = SYMBOL_IMAGEBROWSER_IDNAME, const wxString& caption = SYMBOL_IMAGEBROWSER_TITLE, const wxPoint& pos = SYMBOL_IMAGEBROWSER_POSITION, const wxSize& size = SYMBOL_IMAGEBROWSER_SIZE, long style = SYMBOL_IMAGEBROWSER_STYLE );

    /// Destructor
    ~ImageBrowser();

    /// Initialises member variables
    void Init();
	 
    /// Creates the controls and sizers
    void CreateControls();
	void LoadPrivateDirs();

    ConfigParser* GetConfigParser();

    /// Retrieves bitmap resources
    wxBitmap GetBitmapResource( const wxString& name );

    /// Retrieves icon resources
    wxIcon GetIconResource( const wxString& name );

    /// Should we show tooltips?
    static bool ShowToolTips();

    void OnDecorationTimer(wxTimerEvent& event);
    void TreeExpanded(wxTreeEvent &event);
	void OnDirClicked(wxTreeEvent& event);
	//void DirMenuPopped(wxCommandEvent &event);
	void MenuPopped(wxCommandEvent &event);
	void ReNumberImages(wxCommandEvent &evt);
    void MakeTopDirectory(wxCommandEvent &evt);
    void OnKeyDown(wxKeyEvent &event);
    void OnTreeKeyDown(wxKeyEvent &event);
    void SetAcceleratorTable(const wxAcceleratorTable &accel);
    void RefreshDirTree(wxString path);

    void MenuRescaleImages(wxCommandEvent &event);
    void MenuOpenDirectory(wxCommandEvent &event);
    void MenuDeleteDirectory(wxCommandEvent &evt);
    bool DeleteDirectory(wxString path);
    void DirectoryWasDeleted(wxString path, wxTreeItemId);
    void OnDeleteDirectory(wxCommandEvent &event);
    void OnArchiveDirectory(wxCommandEvent &event);

    void             DragStart()                            
    { 
        std::cout << "DragStart()" << std::endl;
        draggingFiles = true; 
    }

    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);

    void           OnDragFiles(wxMouseEvent     &event);
    void        OnDropDirFiles(wxDropFilesEvent &event);
    void JumpToRandomDirectory(wxCommandEvent   &event);
    void         MarkDirectory(wxCommandEvent   &event);
    void         BackDirectory(wxCommandEvent   &event);
    void        TouchDirectory(wxCommandEvent   &event);

    wxString GetCurrentDir();

    void ReportDirectoryInfo(wxString path, wxTreeItemId id, int flags);
    //wxTreeItemId  GetTreeItemId(wxString path, wxTreeItemId root);
    wxTreeCtrl* GetTreeCtrl() { return treeCtrl; }

    void testFunc2(int i);
    void SelectPathOnly(wxString path);
        
    void LoadArrayString(wxArrayString &arrayString, wxString fileName);
    void SaveArrayString(wxArrayString &arrayString, wxString fileName);
    void SaveMarkedDirs();

    // Main Events
    void EventOpenImageViewer();
    void EventCloseImageViewer();
    void EventOpenThumbnails();
    void EventExit();

    void ShowImageViewer(wxString fileName);
    bool Show(bool show = true);
    FileNameList* GetFileNameList() { return fileNameList; }

////@begin ImageBrowser member variables

    Image_BrowserApp    *image_BrowserApp;
	wxMenuBar			*menuBar;
	wxMenu				*menu;
	wxSplitterWindow	*splitter1;
	wxGenericDirCtrl	*dirTreeCtrl;
    wxTreeCtrl          *treeCtrl;
	ThumbnailCanvas		*thumbnailCanvas;
    //wxStaticText        *debuggingWindow;
    ImageViewer         *imageViewer;
    wxTextCtrl          *directoryNameCtrl;
    RightHandWindow     *rightHandWindow;
    FileNameList        *fileNameList;

	wxArrayString		 privateDirs;
    wxArrayString        knownDirList;
    wxArrayString        history;
    wxArrayString		 markedDirsIncoming;
    wxArrayString		 markedDirsOutgoing;
    wxString             currentDirectory;

    wxTimer              decorationTimer;
    bool                 allowTreeDecoration;
    bool                 recordHistory;
    bool                 created;
    ImageResizerPermanent           imageResizerPermanent;
    deque_thread_safe<ResizerEntry> resizerEntries;

    bool                draggingFiles;
    TreeDropTargetHandler   treeDropTargetHandler;
    ////@end ImageBrowser member variables
};




#endif
// _IMAGEBROWSER_H_
