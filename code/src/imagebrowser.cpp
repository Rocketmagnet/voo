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

#include <functional>
#include <iostream>
using namespace std;

#define PRIVATE_DIRS_FILE_NAME "prvdirs.txt"

 
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

	////@begin ImageBrowser event table entries
	////@end ImageBrowser event table entries
	EVT_DIRCTRL_SELECTIONCHANGED(ID_DIRECTORY_CTRL, ImageBrowser::OnDirClicked)
	//EVT_DIRCTRL_MENU_POPPED_UP(wxID_MENU_DIR,  ImageBrowser::DirMenuPopped)
	EVT_DIRCTRL_MENU_POPPED_UP(wxID_MENU_DIR, ImageBrowser::MenuPopped)
    EVT_KEY_DOWN(ImageBrowser::OnKeyDown)
END_EVENT_TABLE()


/*
 * ImageBrowser constructors
 */

ImageBrowser::ImageBrowser()
{
    Init();
}

ImageBrowser::ImageBrowser( wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
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
    return true;
}


/*
 * ImageBrowser destructor
 */

ImageBrowser::~ImageBrowser()
{
////@begin ImageBrowser destruction
////@end ImageBrowser destruction
}


/*
 * Member initialisation
 */

void ImageBrowser::Init()
{
////@begin ImageBrowser member initialisation
////@end ImageBrowser member initialisation
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
    wxDirItemData *data = dirTreeCtrl->GetRightClickItemData();
  
    cout << "ReNumberImages " << data->m_path << endl;
}


void ImageBrowser::MakeTopDirectory(wxCommandEvent &evt)
{
    wxDirItemData *data = dirTreeCtrl->GetRightClickItemData();

    cout << "Make Top " << data->m_path << endl;
}

void ImageBrowser::DeleteDirectory(wxCommandEvent &evt)
{
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
	dirTreeCtrl->GetMenu()->AppendSeparator();
	int id = dirTreeCtrl->NewMenuItem("Renumber Images");
	dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::ReNumberImages, this, id);

    id = dirTreeCtrl->NewMenuItem("Delete Directory");
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::DeleteDirectory, this, id);

    id = dirTreeCtrl->NewMenuItem("Make Top Directory");
    dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MakeTopDirectory, this, id);
}


typedef std::function< void(int) > classFuncPtr;

void ImageBrowser::SetAcceleratorTable(const wxAcceleratorTable &accel)
{
    wxWindow::SetAcceleratorTable(accel);
    thumbnailCanvas->SetAcceleratorTable(accel);
}



void ImageBrowser::CreateControls()
{    
////@begin ImageBrowser content construction
    // Generated by DialogBlocks, 28/11/2017 12:30:49 (unregistered)
    
	//wxPanel *mainPanel = new wxPanel(this, -1, wxPoint(0, 0), wxDefaultSize);



    wxString s1("Intel LPSS Driver");  
	wxString s2("Intel LPSS Driver");
	//s.Remove(7, 0);
    //GetFragment(s);
    //GetFragment(s);
    //GetFragment(s);
    //GetFragment(s);
	//GetFragment(s);
	//GetFragment(s);
	//GetFragment(s);

	cout << "Compare: " << CompareStringsNatural_(s1, s2) << endl;

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
																													  wxDIRCTRL_RCLICK_MENU           |
                                                                                                                      wxDIRCTRL_RCLICK_MENU_SORT_NAME |
                                                                                                                      wxDIRCTRL_RCLICK_MENU_SORT_DATE);

    //dirTreeCtrl->AddRightClickMenuItem("testFunc", this, (wxFrame::(*func)(wxCommandEvent &))ImageBrowser::testFunc);
    //int id = dirTreeCtrl->NewMenuItem("testFunc");
    //dirTreeCtrl->Bind(wxEVT_MENU, &ImageBrowser::MenuPopped, this, id);
	
    //int id = dirTreeCtrl->NewMenuItem("testFunc");
    //dirTreeCtrl->Bind(wxEVT_MENU, std::bind(&ImageBrowser::testFunc, this), id);
	
    //dirTreeCtrl = new BetterDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(320, 200), wxDIRCTRL_DIR_ONLY);
    //wxTreeCtrl* itemTreeCtrl4 = new wxTreeCtrl(splitter1, ID_DIRCTRL, wxDefaultPosition, wxSize(100, 100), wxTR_SINGLE|wxSIMPLE_BORDER);
	
	thumbnailCanvas = new ThumbnailCanvas(splitter1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize);
	thumbnailCanvas->SetScrollbars(10, 10, 50, 275);
	thumbnailCanvas->LoadThumbnails(".");

	splitter1->SplitVertically(dirTreeCtrl, thumbnailCanvas, 100);
	splitter1->SetSashGravity(0.0);

	LoadPrivateDirs();
	dirTreeCtrl->ExpandPath(wxGetCwd());
	 
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

void ImageBrowser::DirectoryWasDeleted(wxString path)
{
    wxFileName dir(path);
    dir.RemoveLastDir();

    path = dir.GetFullPath();

    dirTreeCtrl->CollapsePath(path);
    dirTreeCtrl->ExpandPath(  path);

    currentDirectory = path;
    thumbnailCanvas->HideImageViewer();
    thumbnailCanvas->LoadThumbnails(currentDirectory);
    thumbnailCanvas->Refresh();
}

void ImageBrowser::OnDirClicked(wxTreeEvent& event)
{
	cout << "OnDirClicked" << endl;

	if (dirTreeCtrl)
	{
        currentDirectory = dirTreeCtrl->GetPath(event.GetItem());
		cout << "Chose " << currentDirectory << endl;

		thumbnailCanvas->LoadThumbnails(currentDirectory);
		thumbnailCanvas->Refresh();
	}
	event.Skip();
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