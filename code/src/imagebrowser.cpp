/////////////////////////////////////////////////////////////////////////////
// Name:        imagebrowser.cpp
// Purpose:     
// Author:      Johnathan Lesbian Seagull
// Modified by: 
// Created:     27/11/2017 15:00:49
// RCS-ID:      
// Copyright:   
// Licence:     
/////////////////////////////////////////////////////////////////////////////

// Generated by DialogBlocks (unregistered), 27/11/2017 15:00:49

// For compilers that support precompilation, includes "wx/wx.h".
#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

////@begin includes
#include "wx/imaglist.h"
////@end includes

#include "imagebrowser.h"
#include "thumbnail_canvas.h"
#include "wx/sizer.h"
#include "wx/splitter.h"
#include "wx/dirctrl.h"
#include "wx/textfile.h"
#include "wx/arrstr.h"
#include "wx/filefn.h"
#include "directory_functions.h"
#include "message.h"
#include "imageviewer.h"
#include "image_browserapp.h"
#include "file_name_list.h"
#include "status_bar.h"

#include <functional>
#include <iostream>
using namespace std;


extern "C"
{
#include "jpeg_turbo.h"
};



////@begin XPM images
////@end XPM images

wxStatusBar  *sBarGlobal;

/*
 * ImageBrowser type definition
 */

IMPLEMENT_CLASS(ImageBrowser, wxFrame)

LiquidMessageDispatcher liquidMessageDispatcher(0);

/*
 * ImageBrowser event table definition
 */

BEGIN_EVENT_TABLE(ImageBrowser, wxFrame)
	EVT_DIRCTRL_SELECTIONCHANGED(ID_DIRECTORY_CTRL, ImageBrowser::OnDirClicked)
	//EVT_DIRCTRL_MENU_POPPED_UP(wxID_MENU_DIR,       ImageBrowser::DirMenuPopped)
    EVT_DIRCTRL_SHOWING_POPUP_MENU(0,   ImageBrowser::MenuPopped)
    EVT_KEY_DOWN(                                   ImageBrowser::OnKeyDown)
    EVT_MENU(ID_DELETE_DIRECTORY,                   ImageBrowser::OnDeleteDirectory)
    EVT_MENU(ID_ARCHIVE_DIRECTORY,                  ImageBrowser::OnArchiveDirectory)
    EVT_TIMER(555, ImageBrowser::OnDecorationTimer)
END_EVENT_TABLE()


/*
 * ImageBrowser constructors
 */

ImageBrowser::ImageBrowser()
: allowTreeDecoration(false),
  decorationTimer(this, 555)
{
    //cout << "ImageBrowser::ImageBrowser() " << this << endl;
    Init();
}

ImageBrowser::ImageBrowser(Image_BrowserApp* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: allowTreeDecoration(false),
  decorationTimer(this, 555),
  image_BrowserApp(parent)
{
    //cout << "ImageBrowser::ImageBrowser(" << parent << ") " << this << endl;
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*
 * ImageBrowser creator
 */

bool ImageBrowser::Create(Image_BrowserApp* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ImageBrowser creation
    //cout << "ImageBrowser::Create(" << parent << ")" << endl;
    wxFrame::Create( NULL, id, caption, pos, size, style );
    //cout << "ImageBrowser::Create done frame create" << endl;

    CreateControls();
    //cout << "ImageBrowser::Create done CreateControls()" << endl;
    Centre();
////@end ImageBrowser creation

    decorationTimer.StartOnce(1000);
    return true;
}


/*
 * ImageBrowser destructor
 */

ImageBrowser::~ImageBrowser()
{
}


/*
 * Member initialisation
 */

void ImageBrowser::Init()
{
    //cout << "ImageBrowser::Init()" << endl;
}

LiquidMessage someMessage(wxT("SomeMessage"));

void ImageBrowser::OnDecorationTimer(wxTimerEvent& event)
{
    //cout << "Timer Triggered" << endl;
    allowTreeDecoration = true;
    //liquidMessageDispatcher.ProcessMessage(someMessage);
    //cout << "DONE" << endl;
}


/*
 * Control creation for ImageBrowser
 */



#define FRAGMENT_TYPE_ALPHA   1
#define FRAGMENT_TYPE_NUMERIC 2

#define IS_A_DIGIT(c)             ((c>='0') && (c<='9'))
#define IS_A_DIGIT_OR_POINT(c)   (((c>='0') && (c<='9')) || (c=='.'))
#define IS_NUMERIC_PUNCTUATION(c) ((c=='-') || (c=='.'))


int CharType_(char c, char next_c)
{
    if (IS_A_DIGIT(c))
    {
        return FRAGMENT_TYPE_NUMERIC;
    }
    else
    {
        if (IS_NUMERIC_PUNCTUATION(c))
        {
            if (IS_A_DIGIT(next_c) || (next_c==0))
            {
                return FRAGMENT_TYPE_NUMERIC;
            }
        }
        return FRAGMENT_TYPE_ALPHA;
    }
} 
 
char GetNextChar(wxString &s)
{
	if (s.Length() > 0)
		return s[0];
	else
		return 0;
}

char GetNextChar2_(wxString &s, int i)
{
	if (s.Length() > i)
		return s[i];
	else
		return 0;
}

wxString GetFragment_(wxString &s, int &fragmentType)
{
    int charsRemaining = s.Length();

    int numCharsAbsorbed = 0;
    char c, next_c;
    
	int i = 2;
    wxString fragment;

    if (s.Length() >= 1)
        c = s[0];
    else
        return wxString("");

    if (s.Length() > 1)
        next_c = s[1];
    else
        next_c = 0;

    fragmentType = CharType_(c, next_c);

    if (fragmentType == FRAGMENT_TYPE_ALPHA)
    {
        do
        {
            fragment += c;
            c         = next_c;
			next_c    = GetNextChar2_(s, i++);
            charsRemaining--;

        } while ((CharType_(c, next_c) == FRAGMENT_TYPE_ALPHA) && (charsRemaining));
    }
    else if (fragmentType == FRAGMENT_TYPE_NUMERIC)
    {
        do
        {
            fragment += c;
            c = next_c;
            next_c = GetNextChar2_(s, i++);
            charsRemaining--;

        } while ((CharType_(c, next_c) == FRAGMENT_TYPE_NUMERIC) && (charsRemaining));
    }

    s.Remove(0, i-2);
    return fragment;
}

int CompareStringsNatural_(const wxString &_a, const wxString &_b)
{
	wxString a(_a);
	wxString b(_b);

	while (1)
	{
		int fragTypeA, fragTypeB;
		wxString fragA = GetFragment_(a, fragTypeA);
		wxString fragB = GetFragment_(b, fragTypeB);

		int lenA = fragA.Length();
		int lenB = fragB.Length();

		if ((lenA == 0) && (lenB == 0))			return  0;
		if ((lenA == 0) && (lenB  > 0))			return -1;
		if ((lenA  > 0) && (lenB == 0))			return  1;
 
		if ((fragTypeA == FRAGMENT_TYPE_NUMERIC) &&
			(fragTypeB == FRAGMENT_TYPE_NUMERIC))
		{
			double av, bv;
			fragA.ToDouble(&av);
			fragB.ToDouble(&bv);

			if (av == bv)
			{
				cout<< "Numeric: " << fragA << " == " << fragB << endl;
				continue;
			}

			if (av < bv)
			{
				cout << "Numeric: " << fragA << " < " << fragB << endl;
				return -1;
			}
			else
			{
				cout << "Numeric: " << fragA << " < " << fragB << endl;
				return 1;
			}
			//return av < bv;
		}

		if ((fragTypeA == FRAGMENT_TYPE_ALPHA) &&
			(fragTypeB == FRAGMENT_TYPE_ALPHA))
		{
			int c = fragA.Cmp(fragB);

			if (c == 0)
			{
				cout << "Alpha: " << fragA << " == " << fragB << "  c=0" << endl;
				continue;
			}

			if (c < 0)
			{
				cout << "Alpha: " << fragA << " < " << fragB << endl;
			}
			else
			{
				cout << "Alpha: " << fragA << " > " << fragB << endl;
			}

			return c;
		}

		if ((fragTypeA == FRAGMENT_TYPE_ALPHA) &&
			(fragTypeB == FRAGMENT_TYPE_NUMERIC))
		{
			cout << "Alpha: " << fragA << "  Numeric: " << fragB << endl;
			return -1;
		}
		else
		{
			cout << "Numeric: " << fragA << "  Alpha: " << fragB << endl;
			return 1;
		}
	}

	return 0;
}


void ImageBrowser::ReNumberImages(wxCommandEvent &evt)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));


}

void ImageBrowser::MakeTopDirectory(wxCommandEvent &evt)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    //cout << "Make Top " << data->m_path << endl;
}

void ImageBrowser::MenuRescaleImages(wxCommandEvent &event)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    thumbnailCanvas->StopLoadingThumbnails(data->m_path);


    ImageResizer *imageResizer = new ImageResizer(data->m_path, 2000, 2000);

    //ChooseRescaleSize *custom = new ChooseRescaleSize(*imageResizer);
    //custom->ShowModal();

    imageResizer->Run();
}

void ImageBrowser::MenuOpenDirectory(wxCommandEvent &evt)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    wxString command = wxT("explorer \"") + data->m_path + wxT("\"");

    wxExecute(command.c_str(), wxEXEC_ASYNC, NULL);
    //cout << "Make Top " << data->m_path << endl;
}

/*
void ImageBrowser::DirMenuPopped(wxCommandEvent &event)
{
	cout << "DirMenuPopped" << endl;

	dirTreeCtrl->GetMenu()->AppendSeparator();
	int id = dirTreeCtrl->NewMenuItem("Renumber Images");
	dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::ReNumberImages, this, id);
}
*/

void ImageBrowser::MenuPopped(wxCommandEvent &event)
{
    wxMenu *menu = dirTreeCtrl->GetPopupMenu();
	menu->AppendSeparator();


    wxWindowID id = wxNewId();
    menu->Append(id, "Renumber Images");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::ReNumberImages, this, id);

    id = wxNewId();
    menu->Append(id, "Delete Directory");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MenuDeleteDirectory, this, id);

    id = wxNewId();
    menu->Append(id, "Make Top Directory");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MakeTopDirectory, this, id);

    id = wxNewId();
    menu->Append(id, "Rescale Images");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MenuRescaleImages, this, id);

    id = wxNewId();
    menu->Append(id, "Open Directory");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MenuOpenDirectory, this, id);

    menu->UpdateUI();
}


typedef std::function< void(int) > classFuncPtr;

void ImageBrowser::SetAcceleratorTable(const wxAcceleratorTable &accel)
{
    wxWindow::SetAcceleratorTable(accel);
    thumbnailCanvas->SetAcceleratorTable(accel);
}


wxTextCtrl          *debuggingWindow = 0;

void SetDebuggingText(wxString text)
{
    if (debuggingWindow)
        debuggingWindow->SetValue(text);
}

void ImageBrowser::CreateControls()
{    
    ImageBrowser* itemFrame1 = this;
     
    //menuBar = new wxMenuBar;
    //SetMenuBar(menuBar);
	//
	//menu = new wxMenu;
	//menu->Append(wxID_SAVEAS);
	//menu->AppendSeparator();
    //
	//menuBar->Append(menu, wxT("&Image"));
    
	splitter1 = new wxSplitterWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	splitter1->SetSize(GetClientSize());
	splitter1->SetSashGravity(1.0);

    sBarGlobal  = new wxStatusBar(this, ID_STATUSBAR, wxST_SIZEGRIP | wxNO_BORDER);
    sBarGlobal->SetFieldsCount(4);
    this->SetStatusBar(sBarGlobal);

    //wxStaticBitmap *m_statbmp = new wxStaticBitmap(this, wxID_ANY, wxIcon(green_xpm));
    //wxRect rect;
    //sBarGlobal->GetFieldRect(3, rect);
    //wxSize size = m_statbmp->GetSize();
    //wxPoint sbPos = sBarGlobal->GetPosition();
    //rect.x += sbPos.x;
    //rect.y += sbPos.y;
    //m_statbmp->Move(rect.x + (rect.width - size.x) / 2, rect.y + (rect.height - size.y) / 2);
    
    //dirTreeCtrl = new PowerDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(320, 200), wxDIRCTRL_DIR_ONLY);
	dirTreeCtrl = new wxGenericDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(640, 200), wxDIRCTRL_DIR_ONLY              |
																													  wxDIRCTRL_EDIT_LABELS           |
																													  wxDIRCTRL_POPUP_MENU           |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_NAME |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_DATE);

    treeCtrl = dirTreeCtrl->GetTreeCtrl();

    treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDED, &ImageBrowser::TreeExpanded, this, -1);

    if (1)
    {
        wxFrame *debuggingFrame = new wxFrame(this, -1, wxT("Debugging"), wxPoint(200, 600), wxSize(400, 400));
        debuggingWindow = new wxTextCtrl(debuggingFrame, -1, wxT("Test"), wxPoint(0, 0), wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        wxBoxSizer *debugSizer = new wxBoxSizer(wxHORIZONTAL);
        debuggingFrame->SetSizer(debugSizer);
        debugSizer->Add(debuggingWindow, 1, wxEXPAND);
        debuggingFrame->Show(true);
    }
    //dirTreeCtrl->AddRightClickMenuItem("testFunc", this, (wxFrame::(*func)(wxCommandEvent &))ImageBrowser::testFunc);
    //int id = dirTreeCtrl->NewMenuItem("testFunc");
    //dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MenuPopped, this, id);
	
    //int id = dirTreeCtrl->NewMenuItem("testFunc");
    //dirTreeCtrl->Bind(wxEVT_MENU, std::bind(&ImageBrowser::testFunc, this), id);
	
    //dirTreeCtrl = new BetterDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(320, 200), wxDIRCTRL_DIR_ONLY);
    //wxTreeCtrl* itemTreeCtrl4 = new wxTreeCtrl(splitter1, ID_DIRCTRL, wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE|wxSIMPLE_BORDER);

    //rightHandWindow = new RightHandWindow(splitter1);
    //directoryNameCtrl = rightHandWindow->GetDirectoryNameCtrl();
    //thumbnailCanvas   = rightHandWindow->GetThumbnailCanvas();

    imageViewer     = new ImageViewer(this, -1, wxT("Image Viewer"), wxDefaultPosition, wxDefaultSize, 0);
    thumbnailCanvas = new ThumbnailCanvas(this, splitter1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize);
    thumbnailCanvas->SetImageViewer(imageViewer);
    imageViewer->SetThumbnailCanvas(thumbnailCanvas);
	thumbnailCanvas->SetScrollbars(10, 10, 50, 275);
	thumbnailCanvas->LoadThumbnails(".");

    //splitter1->SplitVertically(dirTreeCtrl, rightHandWindow, 100);
    splitter1->SplitVertically(dirTreeCtrl, thumbnailCanvas, 300);
	splitter1->SetSashGravity(0.0);

	LoadPrivateDirs();

    currentDirectory = image_BrowserApp->GetConfigParser()->GetString("currentDirectory");
    if (currentDirectory.IsEmpty())
    {
        currentDirectory = wxGetCwd();
        image_BrowserApp->GetConfigParser()->SetString("currentDirectory", currentDirectory.ToStdString());
        image_BrowserApp->GetConfigParser()->Write();
    }
	dirTreeCtrl->ExpandPath(currentDirectory);

    wxAcceleratorEntry entries[2];
    entries[0].Set(wxACCEL_CTRL, (int) 'D', ID_DELETE_DIRECTORY);
    entries[1].Set(wxACCEL_CTRL, (int) 'K', ID_ARCHIVE_DIRECTORY);
    wxAcceleratorTable accel(1, entries);
    SetAcceleratorTable(accel);

    ////@end ImageBrowser content construction
}


/*
 * Should we show tooltips?
 */

bool ImageBrowser::ShowToolTips()
{
    return true;
}

/*
 * Get bitmap resources
 */

wxBitmap ImageBrowser::GetBitmapResource( const wxString& name )
{
    // Bitmap retrieval
////@begin ImageBrowser bitmap retrieval
    wxUnusedVar(name);
    return wxNullBitmap;
////@end ImageBrowser bitmap retrieval
}

/*
 * Get icon resources
 */

wxIcon ImageBrowser::GetIconResource( const wxString& name )
{
    // Icon retrieval
////@begin ImageBrowser icon retrieval
    wxUnusedVar(name);
    return wxNullIcon;
////@end ImageBrowser icon retrieval
}

void ImageBrowser::RefreshDirTree(wxString path)
{
    dirTreeCtrl->CollapsePath(path);
    dirTreeCtrl->ExpandPath(path);
}

void ImageBrowser::DirectoryWasDeleted(wxString path, wxTreeItemId removedId)
{
    //cout << "ImageBrowser::DirectoryWasDeleted(" << path << endl;

    dirTreeCtrl->SelectPath(path);
    
    wxTreeItemId siblingId = treeCtrl->GetNextSibling(removedId);   // siblingId is the nearest node to the one being deleted.
                                                                    // Either the next ...
    if (!siblingId.IsOk())                                          // 
    {                                                               // 
        siblingId = treeCtrl->GetPrevSibling(removedId);            // or the previous ...
                                                                    // 
        if (!siblingId.IsOk())                                      // 
        {                                                           // 
            siblingId = treeCtrl->GetItemParent(removedId);         // or the parent.
        }                                                           // 
    }                                                               // 

    if (currentDirectory.Contains(path))                            // If we're deleting the currently selected path,
    {                                                               // 
        treeCtrl->SelectItem(siblingId);                            // then we need to jump to the sibling path
        treeCtrl->Delete(removedId);                                // before we delete it.
        treeCtrl->EnsureVisible(siblingId);                         // 
        wxDirItemData *siblingData = (wxDirItemData*)treeCtrl->GetItemData(siblingId);
        currentDirectory = siblingData->m_path;                     // 
    }                                                               // 
    else                                                            // otherwise
    {                                                               // 
        treeCtrl->Delete(removedId);                                // just delete it. No problems.
    }                                                               // 

    thumbnailCanvas->DirectoryWasDeleted(path);
    thumbnailCanvas->HideImageViewer();
    treeCtrl->SetFocus();
    //thumbnailCanvas->LoadThumbnails(currentDirectory);
    //thumbnailCanvas->Refresh();
}


void ImageBrowser::OnDirClicked(wxTreeEvent& event)
{
	if (dirTreeCtrl)
	{
        wxTreeItemId id = event.GetItem();
        currentDirectory = dirTreeCtrl->GetPath(id);
        //dirTreeCtrl->GetTreeCtrl()->EnsureVisible(id);
        dirTreeCtrl->GetTreeCtrl()->SetScrollPos(wxHORIZONTAL, 0, true);
        //dirTreeCtrl->SetScrollPos(wxHORIZONTAL, 0, true);
		//cout << "Chose " << currentDirectory << endl;

		thumbnailCanvas->LoadThumbnails(currentDirectory);
		thumbnailCanvas->Refresh();

        image_BrowserApp->GetConfigParser()->SetString("currentDirectory", currentDirectory.ToStdString());
        image_BrowserApp->GetConfigParser()->Write();
	}

	event.Skip();
}


void ImageBrowser::TreeExpanded(wxTreeEvent &event)
{
    wxTreeItemId id = event.GetItem();
    wxDirItemData *data = (wxDirItemData*)(treeCtrl->GetItemData(id));
    
    //cout << "ImageBrowser::TreeExpanded(" << data->m_path << ")" << endl;
    if (allowTreeDecoration)
    {
        GreyEmptyDirectories(*treeCtrl, id);
        //DirectorySearcher *directorySearcher = new DirectorySearcher(*this, *treeCtrl, id);
        //directorySearcher->Run();
    }
}

void ImageBrowser::OnKeyDown(wxKeyEvent &event)
{
    //cout << "ImageBrowser::OnKeyDown(" << event.GetKeyCode() << ")" << endl;
}


void ImageBrowser::LoadPrivateDirs()
{
}

wxString ImageBrowser::GetCurrentDir()
{
    return currentDirectory;
}


void Pause(int timeMs)
{
    wxStopWatch sw;

    sw.Start();
    while (sw.Time() < timeMs)
    {
        cout << ".";
    }
    cout << endl;
}

wxString tabs = "";

bool RemoveDirectory(wxString pathName)
{
    wxString fn;
    wxDir dir(pathName);
    wxArrayString paths;
    int attemptsRemaining = 0;
    int i = -2;

    if (!dir.IsOpened())
    {
        cout << tabs << "Failed to open " << pathName << endl;
        return false;
    }

    bool cont = dir.GetFirst(&fn);

    cout << endl;
    cout << tabs << "RemoveDirectory(" << pathName << ")" << endl;

    // if there are files to process
    if (cont)
    {
        do {
            wxString subPath = dir.GetName() + wxFILE_SEP_PATH + fn;
            paths.Add(subPath);
        }
        while (dir.GetNext(&fn));

        attemptsRemaining = paths.size() * 2;

        do {
            if (i < 0)
            {
                if (i == -2)
                    Pause(100);
                else
                    Pause(10);
                i = paths.size() - 1;
            }

            cout << tabs << "attemptsRemaining = " << attemptsRemaining << "  i = " << i << "  size = " << paths.size() << endl;
            wxString subPath = paths[i--];
            cout << tabs << "Checking for existence of " << subPath << " " << wxDirExists(subPath) << endl;

            // if the next filename is actually a directory
            if (wxDirExists(subPath + wxFILE_SEP_PATH))
            {
                cout << tabs << "deleting directory" << subPath + wxFILE_SEP_PATH << endl;
                // delete this directory
                tabs += "    ";
                bool success = RemoveDirectory(subPath + wxFILE_SEP_PATH);

                if (success)
                {
                    paths.Remove(subPath);
                    cout << tabs << "successfully deleted sub path: " << subPath << endl;
                }
            }
            else
            {
                // otherwise attempt to delete this file
                cout << tabs << "Attempt to remove file " << subPath << endl;
                if (!wxRemoveFile(subPath))
                { // failed
                    cout << tabs << "failed to delete file: " << subPath << endl;
                }
                else
                { // success
                    paths.Remove(subPath);
                    cout << tabs << "successfully deleted file: " << subPath << endl;
                }
            }
        } while ((attemptsRemaining--) && (paths.size()>0));
    }

    cout << tabs << "Now remove directory " << pathName << endl;
    bool success = wxRmDir(pathName);
    cout << tabs << "success = " << success << endl;

    tabs = tabs.Left(tabs.Length() - 4);
    return true;
}


void ImageBrowser::OnDeleteDirectory(wxCommandEvent &event)
{
    cout << "ImageBrowser::OnDeleteDirectory(" << currentDirectory << ")" << endl;

    wxFileName parentPath = currentDirectory;
    parentPath.RemoveLastDir();
    wxString firstFile = wxFindFirstFile(parentPath.GetFullPath());
    cout << "firstFile = " << firstFile << endl;
    bool parentIsEmpty = firstFile.empty();

    wxString pathToDelete;

    if (parentIsEmpty)
    {
        pathToDelete = parentPath.GetFullPath();
    }
    else
    {
        pathToDelete = currentDirectory;
    }

    cout << "pathToDelete = " << pathToDelete << endl;
    thumbnailCanvas->UnLoadThumbnails(pathToDelete);
    bool success = DeleteDirectory(pathToDelete);

    if (success)
    {
        dirTreeCtrl->SelectPath(pathToDelete);
        wxTreeItemId id = dirTreeCtrl->GetTreeCtrl()->GetSelection();
        DirectoryWasDeleted(pathToDelete, id);
    }
}


// Called when user Right-clicks, and chooses "Delete Directory"
// 
void ImageBrowser::MenuDeleteDirectory(wxCommandEvent &evt)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    wxString path = data->m_path;
    thumbnailCanvas->UnLoadThumbnails(path);
    bool success = DeleteDirectory(path);

    if (success)
    {
        DirectoryWasDeleted(path, id);
    }
}



void ImageBrowser::OnArchiveDirectory(wxCommandEvent &event)
{

}


// Pop up a dialog, asking the user to confirm delete
// If YES, then call ImageBrowser::RemoveDirectory(path)
// 
bool ImageBrowser::DeleteDirectory(wxString path)
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

ConfigParser* ImageBrowser::GetConfigParser()
{
    //cout << "ImageBrowser::GetConfigParser()" << this << endl;
    //cout << "  image_BrowserApp = " << image_BrowserApp << endl;

    return image_BrowserApp->GetConfigParser();
}

void ImageBrowser::ReportDirectoryInfo(wxString path, wxTreeItemId id, int flags)
{
    //if (flags | DIRFLAGS_CONTAINS_FILES)
    //{
    //    treeCtrl->SetItemTextColour(id, wxColour(0, 0, 0));
    //}
    //else
    //{
    //    treeCtrl->SetItemTextColour(id, wxColour(128, 128, 128));
    //}
}

void LoadImage2(wxImage &image, wxString fileName)
{
    image.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    wxFileName fn(fileName);
    wxString   extension = fn.GetExt().Upper();

    if ((extension == "JPG") ||
        (extension == "JPEG"))
    {
        //cout << "Using JpegTurbo to load thumbnail " << fileName << endl;

        jpeg_load_state *load_state = ReadJpegHeader((const  char*)fileName.c_str());

        if (load_state)
        {
            int w = load_state->width, h = load_state->height;

            image.Create(w, h);
            image.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
            JpegRead(image.GetData(), load_state);
        }
    }
    else if (extension == "PNG")
    {
        image.LoadFile(fileName);
    }
}

wxThread::ExitCode ImageResizer::Entry()
{
    //cout << "Rescaling begins" << endl;
    const int X_OVERSIZE = 1;
    const int Y_OVERSIZE = 2;
    const int RESCALED   = 4;

    FileNameList fileNameList;
    wxImage      image;
    wxString     s;


    fileNameList.AddFilter(wxT("*.jpg"));           // We ony rescale JPEGs
    fileNameList.AddFilter(wxT("*.jpeg"));
    fileNameList.LoadFileList(directory);

    int n = fileNameList.NumFiles();

    for (int i = 0; i < n; i++)
    {
        wxFileName fullPath = fileNameList.files[i].fileName.GetFullPath();

        STATUS_TEXT(STATUS_BAR_INFORMATION, "Loading %d/%d", i, n);

        LoadImage2(image, fullPath.GetFullPath());

        float imageWidth  = image.GetSize().GetWidth();
        float imageHeight = image.GetSize().GetHeight();

        float ratioX = imageWidth  / (float)maxWidth;
        float ratioY = imageHeight / (float)maxHeight;
        int   newWidth, newHeight;

        int   flags = 0;

        if (ratioX > 1.0)   flags |= X_OVERSIZE;
        if (ratioY > 1.0)   flags |= Y_OVERSIZE;

        cout << "Ratios " << ratioX << ", " << ratioY << endl;

        switch(flags)
        {
            default:
            case 0:
                break;

            case X_OVERSIZE:
                flags |= RESCALED;
                newWidth  = int(imageWidth  / ratioX + 0.5);
                newHeight = int(imageHeight / ratioX + 0.5);
                break;

            case Y_OVERSIZE:
                flags |= RESCALED;
                newWidth = int(imageWidth / ratioY + 0.5);
                newHeight = int(imageHeight / ratioY + 0.5);
                break;

            case X_OVERSIZE | Y_OVERSIZE:
                flags |= RESCALED;
                if (ratioX > ratioY)
                {
                    newWidth = int(imageWidth / ratioX + 0.5);
                    newHeight = int(imageHeight / ratioX + 0.5);
                }
                else
                {
                    newWidth = int(imageWidth / ratioY + 0.5);
                    newHeight = int(imageHeight / ratioY + 0.5);
                }
                break;
        }

        if (flags & RESCALED)
        {
            STATUS_TEXT(STATUS_BAR_INFORMATION, "Rescaling %d/%d", i, n);
            image.Rescale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);
        }

        STATUS_TEXT(STATUS_BAR_INFORMATION, "Saving %d/%d", i, n);

        JpegWrite(fullPath.GetFullPath(), newWidth, newHeight, image.GetData());
        image.Destroy();
    }

    STATUS_TEXT(STATUS_BAR_INFORMATION, " ");

    return 0;
}


ChooseRescaleSize::ChooseRescaleSize(ImageResizer &ir)
: wxDialog(NULL, wxID_ANY, wxT("Select maximum size"), wxDefaultPosition, wxSize(250, 230)),
  imageResizer(ir)
{
    wxPanel *panel = new wxPanel(this, -1);

    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);

    wxString widthString;
    wxString heightString;

     widthString.Printf("%d", imageResizer.maxWidth);
    heightString.Printf("%d", imageResizer.maxHeight);

    wxTextCtrl  *widthCtrl = new wxTextCtrl(panel, -1,  widthString);
    wxTextCtrl *heightCtrl = new wxTextCtrl(panel, -1, heightString);
    /*
    wxStaticBox *st = new wxStaticBox(panel, -1, wxT("Colors"),
        wxPoint(5, 5), wxSize(240, 150));
    wxRadioButton *rb = new wxRadioButton(panel, -1,
        wxT("256 Colors"), wxPoint(15, 30), wxDefaultSize, wxRB_GROUP);

    wxRadioButton *rb1 = new wxRadioButton(panel, -1,
        wxT("16 Colors"), wxPoint(15, 55));
    wxRadioButton *rb2 = new wxRadioButton(panel, -1,
        wxT("2 Colors"), wxPoint(15, 80));
    wxRadioButton *rb3 = new wxRadioButton(panel, -1,
        wxT("Custom"), wxPoint(15, 105));
    wxTextCtrl *tc = new wxTextCtrl(panel, -1, wxT(""),
        wxPoint(95, 105));

    wxButton *okButton = new wxButton(this, -1, wxT("Ok"),
        wxDefaultPosition, wxSize(70, 30));
    wxButton *closeButton = new wxButton(this, -1, wxT("Close"),
        wxDefaultPosition, wxSize(70, 30));

    hbox->Add(okButton, 1);
    hbox->Add(closeButton, 1, wxLEFT, 5);

    vbox->Add(panel, 1);
    vbox->Add(hbox, 0, wxALIGN_CENTER | wxTOP | wxBOTTOM, 10);
    */

    vbox->Add( widthCtrl, 1);
    vbox->Add(heightCtrl, 1);
    SetSizer(vbox);

    Centre();
    //ShowModal();

    Destroy();
}

