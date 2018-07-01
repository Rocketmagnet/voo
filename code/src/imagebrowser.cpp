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
#include "directory_functions.h"

#include <functional>
#include <iostream>
using namespace std;

#define PRIVATE_DIRS_FILE_NAME     "prvdirs.txt"
#define CONFIG_FILE_NAME           "config.txt"
#define DIRFLAGS_CONTAINS_FILES    1


////@begin XPM images
////@end XPM images

wxStatusBar  *sBarGlobal;

/*
 * ImageBrowser type definition
 */

IMPLEMENT_CLASS(ImageBrowser, wxFrame)


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
  decorationTimer(this, 555),
  configParser(CONFIG_FILE_NAME)
{
    Init();
}

ImageBrowser::ImageBrowser( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: allowTreeDecoration(false),
  decorationTimer(this, 555),
  configParser(CONFIG_FILE_NAME)
{
    Init();
    Create( parent, id, caption, pos, size, style );
}


/*
 * ImageBrowser creator
 */

bool ImageBrowser::Create( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
{
////@begin ImageBrowser creation
    wxFrame::Create( parent, id, caption, pos, size, style );

    CreateControls();
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
    configParser.SetString("currentDirectory", currentDirectory.ToStdString());
    configParser.Write();
}


/*
 * Member initialisation
 */

void ImageBrowser::Init()
{
}

void ImageBrowser::OnDecorationTimer(wxTimerEvent& event)
{
    cout << "Timer Triggered" << endl;
    allowTreeDecoration = true;
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

    cout << "Make Top " << data->m_path << endl;
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

    menu->UpdateUI();
}

void ImageBrowser::MenuDeleteDirectory(wxCommandEvent &evt)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    wxString path = data->m_path;

    bool success = DeleteDirectory(path);

    if (success)
    {
        DirectoryWasDeleted(path, id);
    }
}

typedef std::function< void(int) > classFuncPtr;

void ImageBrowser::SetAcceleratorTable(const wxAcceleratorTable &accel)
{
    wxWindow::SetAcceleratorTable(accel);
    thumbnailCanvas->SetAcceleratorTable(accel);
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
	dirTreeCtrl = new wxGenericDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(320, 200), wxDIRCTRL_DIR_ONLY              |
																													  wxDIRCTRL_EDIT_LABELS           |
																													  wxDIRCTRL_POPUP_MENU           |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_NAME |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_DATE);

    treeCtrl = dirTreeCtrl->GetTreeCtrl();

    treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDED, &ImageBrowser::TreeExpanded, this, -1);

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

    thumbnailCanvas = new ThumbnailCanvas(splitter1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize);
	thumbnailCanvas->SetScrollbars(10, 10, 50, 275);
	thumbnailCanvas->LoadThumbnails(".");

    //splitter1->SplitVertically(dirTreeCtrl, rightHandWindow, 100);
    splitter1->SplitVertically(dirTreeCtrl, thumbnailCanvas, 100);
	splitter1->SetSashGravity(0.0);

	LoadPrivateDirs();

    currentDirectory = configParser.GetString("currentDirectory");
    if (currentDirectory.IsEmpty())
    {
        currentDirectory = wxGetCwd();
        configParser.SetString("currentDirectory", currentDirectory.ToStdString());
        configParser.Write();
    }
	dirTreeCtrl->ExpandPath(currentDirectory);
	 
    wxAcceleratorEntry entries[1];
    entries[0].Set(wxACCEL_CTRL, (int) 'D', ID_DELETE_DIRECTORY);
    entries[0].Set(wxACCEL_CTRL, (int) 'K', ID_ARCHIVE_DIRECTORY);
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
    //thumbnailCanvas->LoadThumbnails(currentDirectory);
    //thumbnailCanvas->Refresh();
}


void ImageBrowser::OnDirClicked(wxTreeEvent& event)
{
	if (dirTreeCtrl)
	{
        wxTreeItemId id = event.GetItem();
        currentDirectory = dirTreeCtrl->GetPath(id);
        dirTreeCtrl->GetTreeCtrl()->EnsureVisible(id);
		cout << "Chose " << currentDirectory << endl;

		thumbnailCanvas->LoadThumbnails(currentDirectory);
		thumbnailCanvas->Refresh();
	}

	event.Skip();
}


void ImageBrowser::TreeExpanded(wxTreeEvent &event)
{
    wxTreeItemId id = event.GetItem();
    wxDirItemData *data = (wxDirItemData*)(treeCtrl->GetItemData(id));
    
    if (allowTreeDecoration)
    {
        GreyEmptyDirectories(*treeCtrl, id);
        //DirectorySearcher *directorySearcher = new DirectorySearcher(*this, *treeCtrl, id);
        //directorySearcher->Run();
    }
}

void ImageBrowser::OnKeyDown(wxKeyEvent &event)
{
    cout << "ImageBrowser::OnKeyDown(" << event.GetKeyCode() << ")" << endl;
}


void ImageBrowser::LoadPrivateDirs()
{
	wxFileName  privateDirsFileName(PRIVATE_DIRS_FILE_NAME);
	wxTextFile  privateDirsFile(PRIVATE_DIRS_FILE_NAME);

	if (privateDirsFileName.Exists())
	{
		cout << "loading" << endl;
		privateDirsFile.Open();
	}
	{
		cout << "creating" << endl;
		cout << privateDirsFile.Create() << endl;
		privateDirsFile.Write();
		privateDirsFile.Close();
		return;
	}

	wxString str;

	for (str = privateDirsFile.GetFirstLine(); !privateDirsFile.Eof(); str = privateDirsFile.GetNextLine())
	{
		cout << "Loaded " << str << endl;
		// do something with the current line in str
	}
	cout << "Loaded " << str << endl;

}

wxString ImageBrowser::GetCurrentDir()
{
    return currentDirectory;
}

bool RemoveDirectory(wxString pathName)
{
    wxString fn;
    wxDir dir(pathName);

    if (!dir.IsOpened())
    {
        cout << "Failed to open " << pathName << endl;
        return false;
    }

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


void ImageBrowser::OnDeleteDirectory(wxCommandEvent &event)
{
    wxTreeItemId    id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData  *itemData = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));
    wxString        path(itemData->m_path);

    cout << "ImageBrowser::OnDeleteDirectory(" << path << ")" << endl;
    bool success = DeleteDirectory(path);

    if (success)
    {
        DirectoryWasDeleted(path, id);
    }
}


void ImageBrowser::OnArchiveDirectory(wxCommandEvent &event)
{

}


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


void ImageBrowser::ReportDirectoryInfo(wxString path, wxTreeItemId id, int flags)
{
    if (flags | DIRFLAGS_CONTAINS_FILES)
    {
        treeCtrl->SetItemTextColour(id, wxColour(0, 0, 0));
    }
    else
    {
        treeCtrl->SetItemTextColour(id, wxColour(128, 128, 128));
    }
}



RightHandWindow::RightHandWindow(wxWindow *parent)
: wxWindow(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL)
{
    thumbnailCanvas   = new ThumbnailCanvas(this, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize);
    directoryNameCtrl = new wxTextCtrl(this, wxID_ANY, wxT("Hello"), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxWANTS_CHARS);
    boxSizer          = new wxBoxSizer(wxVERTICAL);

    boxSizer->Add(directoryNameCtrl, 0, wxEXPAND);
    boxSizer->Add(thumbnailCanvas, 1, wxEXPAND);

    SetSizer(boxSizer);
}

