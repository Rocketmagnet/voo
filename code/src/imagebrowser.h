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

#ifndef _IMAGEBROWSER_H_
#define _IMAGEBROWSER_H_

#include "wx/filename.h"
#include "wx/frame.h"
#include "wx/treectrl.h"
#include "wx/aui/auibook.h"
//#include "wx/notebook.h"
#include "config_parser.h"
#include "wx/dnd.h"
#include "deque_thread_safe.h"
#include "status_bar.h"

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
//class wxNotebook;
class ThumbnailCanvas;
class ImageViewer;
class ImageBrowser;
class Image_BrowserApp;
class ImageBrowserFrame;
class ImageBrowserNoteBook;
class FileNameList;
class wxXmlNode;

#define ID_DIRECTORY_CTRL	100
#define ID_NOTEBOOK         101

void    SetDebuggingText(wxString text);
void AppendDebuggingText(wxString text);
int CompareStringsNatural_(const wxString& _a, const wxString& _b);

/*
class RightHandWindow : public wxWindow
{
public:
    RightHandWindow(wxWindow* parent);

    ThumbnailCanvas* GetThumbnailCanvas()    { return thumbnailCanvas; }
    wxTextCtrl*      GetDirectoryNameCtrl()  { return directoryNameCtrl; }


private:
    ThumbnailCanvas  *thumbnailCanvas;
    wxTextCtrl       *directoryNameCtrl;
    wxBoxSizer       *boxSizer;
};
*/

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


// Thread that resizes all the images in the queue.
class ImageResizerPermanent : public wxThread
{
public:
    ImageResizerPermanent(ImageBrowserFrame  *_imageBrowserFrame)
        : wxThread(wxTHREAD_JOINABLE),
        imageBrowserFrame(_imageBrowserFrame)
    {
    }

    ExitCode Entry();
    deque_thread_safe<ResizerEntry>* GetResizerEntries()  {return &resizerEntries; }

private:
    ImageBrowserFrame* imageBrowserFrame;
    void SaveState();
    void LoadState();
    deque_thread_safe<ResizerEntry> resizerEntries;
};

extern wxRect plusRect;

class ArtProvider : public wxAuiDefaultTabArt
{
public:

    wxAuiTabArt* Clone() override
    {
        //std::cout << "Clone() " << this << " ";
        ArtProvider *newArtProvider = new ArtProvider(*this);
        //std::cout << newArtProvider << std::endl;
        //std::cout << newArtProvider->plusRect.x << ", " << newArtProvider->plusRect.y << std::endl;
        return newArtProvider;
    }

    ArtProvider()
    {
        //std::cout << "ArtProvider() " << this << std::endl;
        m_tabTextColour                 = *wxBLUE;
        m_activeTabBackgroundColour     = *wxBLUE;
        m_inactiveTabBackgroundColour   = *wxBLUE;
        m_borderWidth                   = 1;
        std::cout << plusRect.x << ", " << plusRect.y << std::endl;
    }

    void 	SetActiveColour(const wxColour& colour)
    {
        //std::cout << "SetActiveColour(" << colour.Red() << ", " << colour.Green() << ", " << colour.Blue() << ")" << endl;
        m_activeColour = wxColour(128, 128, 255);
    }

    //void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page, const wxRect& rect, int close_button_state, wxRect* out_tab_rect, wxRect* out_button_rect, int* x_extent)
    //{
    //    std::cout << "DrawTab()" << endl;
    //
    //}

    void DrawBackground(wxDC& dc, wxWindow* wnd, const wxRect& rect) override
    {
        //std::cout << "DrawBackground() " << this << std::endl;

        wxAuiDefaultTabArt::DrawBackground(dc, wnd, rect);

        // Draw "+" button at top right
        int size = 16;
        plusRect = wxRect(rect.GetRight() - size - 4, rect.GetTop() + 4, size, size);
        std::cout << plusRect.x << ", " << plusRect.y << std::endl;

        dc.SetBrush(*wxLIGHT_GREY_BRUSH);
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(plusRect);

        dc.SetPen(*wxBLACK_PEN);
        dc.DrawLine(plusRect.x + 4,                  plusRect.y + plusRect.height / 2, plusRect.x + plusRect.width - 4, plusRect.y + plusRect.height / 2); // Horizontal
        dc.DrawLine(plusRect.x + plusRect.width / 2, plusRect.y + 4,                   plusRect.x + plusRect.width / 2, plusRect.y + plusRect.height - 4); // Vertical
    }

    /*
    //void DrawTab(wxDC& dc, wxAuiNotebook* notebook, const wxAuiNotebookPage& page, const wxRect& rect, int tabIndex, bool active)
    void DrawTab(wxDC& dc, wxWindow* wnd, const wxAuiNotebookPage& page, const wxRect& rect, int close_button_state, wxRect* out_tab_rect, wxRect* out_button_rect, int* x_extent);
    
    virtual int GetTabHeight(wxAuiNotebook* notebook)
    {
        return 30;  // Set custom height for tabs
    }

    virtual void SetNormalFont(const wxFont& font)
    {
        m_font = font;
    }

    virtual void SetActiveFont(const wxFont& font)
    {
        m_activeFont = font;
    }
    */



    wxColour m_tabTextColour;
    wxColour m_activeTabBackgroundColour;
    wxColour m_inactiveTabBackgroundColour;
    wxFont   m_font, m_activeFont;
    int      m_borderWidth;
};

// An ImageBrowserNotebook contains pages of ImageBrowsers
// 
// 
class ImageBrowserNotebook : public wxAuiNotebook
{
public:
    ImageBrowserNotebook(ImageBrowserFrame* _parent);
    ~ImageBrowserNotebook();

    void AddPlusButton();
    void AddImageBrowser(wxXmlNode* xmlConfig = (wxXmlNode*)0);
    void SetPathName(wxFileName name, ImageBrowser* imageBrowser);
    void RegisterXmlConfigChange()  const;
    deque_thread_safe<ResizerEntry>* GetResizerEntries() { return resizerEntries; }

    //std::vector<ImageBrowser*>& GetImageBrowsers() { return imageBrowsers; }

    void StatusMessage(wxWindowID tabID, size_t place, wxString text);

    ImageBrowser* GetImageBrowser(size_t pageNumber)
    {
        return (ImageBrowser*)GetPage(pageNumber);
        //return  imageBrowsers[pageNumber];
    }

    void PageChanged(wxBookCtrlEvent& event);
private:
    void OnKeyDown(         wxKeyEvent& event);
    void OnNotebookClick( wxMouseEvent& event);

    //std::vector<ImageBrowser*>       imageBrowsers;
    ImageBrowserFrame*               imageBrowserFrame;

    ImageResizerPermanent           *imageResizerPermanent;
    deque_thread_safe<ResizerEntry> *resizerEntries;

    ArtProvider                     *customTabArt;
};

// Joinable, can be on stack. 
// imageResizerPermanent is wxTHREAD_JOINABLE
// Do not delete it.

class TreeDropTargetHandler : public wxDropTarget
{
public:
    TreeDropTargetHandler()
    : treeCtrl(0)
    {
        fileDataObject = new wxFileDataObject;
        SetDataObject(fileDataObject);
    }

    ~TreeDropTargetHandler()
    {
    }

    wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult)   { return wxDragNone; }
    wxDragResult OnData(    wxCoord x, wxCoord y, wxDragResult defResult)   { return wxDragNone; }
    wxDragResult OnEnter(   wxCoord x, wxCoord y, wxDragResult defResult)   { return wxDragNone; }
    bool         OnDrop(    wxCoord x, wxCoord y)                           { return false;      }

    void SetTreeCtrl(wxTreeCtrl* tc) { treeCtrl = tc; }

    wxTreeCtrl         *treeCtrl;
    wxFileDataObject   *fileDataObject;
    wxTreeItemId        prevDragItemId;
};


// This is the outer frame, containing the notebook
class ImageBrowserFrame : public wxFrame
{
    wxDECLARE_EVENT_TABLE();

public:
    ImageBrowserFrame(Image_BrowserApp* parent, wxWindowID id = SYMBOL_IMAGEBROWSER_IDNAME, const wxString& caption = SYMBOL_IMAGEBROWSER_TITLE, const wxPoint& pos = SYMBOL_IMAGEBROWSER_POSITION, const wxSize& size = SYMBOL_IMAGEBROWSER_SIZE, long style = SYMBOL_IMAGEBROWSER_STYLE);
    ~ImageBrowserFrame();

    void RegisterXmlConfigChange();
    void PageChanged(int page);
    void StatusMessage(wxWindowID tabID, size_t place, wxString text)
    {
        //if (place == STATUS_BAR_INFORMATION)
        //    std::cout << "StatusMessage(" << text << ")" << endl;

        if (statusBarThreadSafe)
            statusBarThreadSafe->Message(tabID, place, text);
    }

    void AddNewTab(wxCommandEvent&);

private:
    void ReadConfiguration();
    void EventCloseImageViewer(wxCloseEvent& event);

    Image_BrowserApp     *image_BrowserApp;
    ImageBrowserNotebook *imageBrowserNotebook;
    StatusBarThreadSafe  *statusBarThreadSafe;
    ArtProvider           auiArtProvider;

    bool                  configChangesBlocked;

};

/*
class MyGenericDirCtrl : public wxGenericDirCtrl
{
public:
    MyGenericDirCtrl(wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& dir = wxDirDialogDefaultFolderStr,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long       style = wxDIRCTRL_DEFAULT_STYLE,
        const wxString& filter = wxEmptyString,
        int        defaultFilter = 0,
        const wxString& name = wxTreeCtrlNameStr)
    {
        wxGenericDirCtrl::wxGenericDirCtrl(parent, id, dir, pos, size, style, filter, defaultFilter, name);

    }
    
private:



};
*/


// An ImageBrowser is a page of the notebook, and contains the GenericDirCtrl and the ThumbnailCanvas
//
class ImageBrowser: public wxPanel
{    
    wxDECLARE_EVENT_TABLE();

public:
    ImageBrowser(wxWindow* _parent, ImageBrowserNotebook* _imageBrowserNotebook, wxWindowID id, wxXmlNode* xmlConfig);
    bool  Create();

    ~ImageBrowser();

    void Init();
	 
    /// Creates the controls and sizers
    void CreateControls();
	void LoadPrivateDirs();

    int GetWindowID() const { return id; }
    void StatusMessage(size_t place, wxString text)
    {
        imageBrowserNotebook->StatusMessage(id, place, text);
    }

    wxBitmap      GetBitmapResource( const wxString& name );
    wxIcon        GetIconResource( const wxString& name );
    static bool   ShowToolTips();

    void OnDecorationTimer(wxTimerEvent& event);
    void TreeExpanded(wxTreeEvent &event);
	void OnDirClicked(wxTreeEvent& event);
	void MenuPopped(wxCommandEvent &event);
	void ReNumberImages(wxCommandEvent &evt);
    void MakeTopDirectory(wxCommandEvent &evt);
    void OnKeyDown(wxKeyEvent &event);
    void OnTreeKeyDown(wxKeyEvent &event);
    void SetAcceleratorTable(const wxAcceleratorTable &accel);
    void RefreshDirTree(wxString path);
    void FullRedrawThumbnails();

    void MenuRescaleImages(wxCommandEvent &event);
    void MenuOpenDirectory(wxCommandEvent &event);
    void MenuDeleteDirectory(wxCommandEvent &evt);
    void MenuCopyPath(wxCommandEvent& evt);
    bool DeleteDirectory(wxString path);
    void DirectoryWasDeleted(wxString path, wxTreeItemId);
    void OnDeleteDirectory(wxCommandEvent &event);
    void OnArchiveDirectory(wxCommandEvent &event);

    void             DragStart()                            
    { 
        std::cout << "DragStart()" << std::endl;
        draggingFiles = true; 
    }

    //wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult defResult);

    //void           OnDragFiles(wxMouseEvent     &event);
    void        OnDropDirFiles(wxDropFilesEvent &event);
    void JumpToRandomDirectory(wxCommandEvent   &event);
    void         MarkDirectory(wxCommandEvent   &event);
    void         BackDirectory(wxCommandEvent   &event);
    void        TouchDirectory(wxCommandEvent   &event);

    wxString GetCurrentDir();

    void ReportDirectoryInfo(wxString path, wxTreeItemId id, int flags);
    wxTreeCtrl* GetTreeCtrl() { return treeCtrl; }

    void SelectPathOnly(wxString path);
        
    void LoadArrayString(wxArrayString &arrayString, wxString fileName);
    void SaveArrayString(wxArrayString &arrayString, wxString fileName);
    void SaveMarkedDirs();

    // Main Events
    void EventOpenImageViewer();
    void EventOpenThumbnails();
    void EventExit();

    void ShowImageViewer(wxString fileName);
    bool Show(bool show = true);
    FileNameList* GetFileNameList() { return fileNameList; }

    void AddXmlConfiguration(wxXmlNode* xmlConfig);
    ThumbnailCanvas* GetThumbnailCanvas() { return thumbnailCanvas; }

////@begin ImageBrowser member variables

    wxWindow            *parent;
    ImageBrowserNotebook   *imageBrowserNotebook;
	wxMenuBar			*menuBar;
	wxMenu				*menu;
	wxSplitterWindow	*splitter1;

	wxGenericDirCtrl	*dirTreeCtrl;
    wxTreeCtrl          *treeCtrl;
    ThumbnailCanvas		*thumbnailCanvas;
   
    //wxStaticText        *debuggingWindow;
    ImageViewer         *imageViewer;
    wxTextCtrl          *directoryNameCtrl;
    //RightHandWindow     *rightHandWindow;
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

    bool                    draggingFiles;
    TreeDropTargetHandler  *treeDropTargetHandler;
    int                     rescaleX;
    int                     rescaleY;
    wxWindowID              id;
    //wxXmlNode              *xmlConfig;
    //wxXmlNode              *xmlConfig_Directory;
    //wxXmlNode              *xmlConfig_Rescale;
};


#endif
