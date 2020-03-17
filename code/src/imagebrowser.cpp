//////////////////////////////////////////////////////////////////////////////
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
#include "imageviewer.h"
#include "image_browserapp.h"
#include "file_name_list.h"
#include "status_bar.h"
#include <chrono>
#include <thread>

#include <functional>
#include <iostream>
#include <fstream>
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


/*
 * ImageBrowser event table definition
 */

BEGIN_EVENT_TABLE(ImageBrowser, wxFrame)
	EVT_DIRCTRL_SELECTIONCHANGED(ID_DIRECTORY_CTRL, ImageBrowser::OnDirClicked)
	//EVT_DIRCTRL_MENU_POPPED_UP(wxID_MENU_DIR,       ImageBrowser::DirMenuPopped)
    EVT_DIRCTRL_SHOWING_POPUP_MENU(0,               ImageBrowser::MenuPopped)
    EVT_KEY_DOWN(                                   ImageBrowser::OnKeyDown)
    EVT_MENU(ID_DELETE_DIRECTORY,                   ImageBrowser::OnDeleteDirectory)
    EVT_MENU(ID_ARCHIVE_DIRECTORY,                  ImageBrowser::OnArchiveDirectory)
    EVT_MENU(ID_RANDOM_DIRECTORY,                   ImageBrowser::JumpToRandomDirectory)
    EVT_MENU(ID_TOUCH_DIRECTORY,                    ImageBrowser::TouchDirectory)
    EVT_TIMER(555, ImageBrowser::OnDecorationTimer)
END_EVENT_TABLE()

//BEGIN_EVENT_TABLE(wxGenericTreeCtrl, wxFrame)
//    EVT_DIRCTRL_SELECTIONCHANGED(ID_DIRECTORY_CTRL, ImageBrowser::OnDirClicked)
//    //EVT_DIRCTRL_MENU_POPPED_UP(wxID_MENU_DIR,       ImageBrowser::DirMenuPopped)
//    EVT_DIRCTRL_SHOWING_POPUP_MENU(0, ImageBrowser::MenuPopped)
//    EVT_KEY_DOWN(ImageBrowser::OnKeyDown)
//    EVT_MENU(ID_DELETE_DIRECTORY, ImageBrowser::OnDeleteDirectory)
//    EVT_MENU(ID_ARCHIVE_DIRECTORY, ImageBrowser::OnArchiveDirectory)
//    EVT_MENU(ID_RANDOM_DIRECTORY, ImageBrowser::JumpToRandomDirectory)
//    EVT_TIMER(555, ImageBrowser::OnDecorationTimer)
//END_EVENT_TABLE()

/*
 * ImageBrowser constructors
 */

ImageBrowser::ImageBrowser()
: allowTreeDecoration(false),
  decorationTimer(this, 555),
  imageResizerPermanent(resizerEntries)
{
    //cout << "ImageBrowser::ImageBrowser() " << this << endl;
    Init();
}

ImageBrowser::ImageBrowser(Image_BrowserApp* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style )
: allowTreeDecoration(false),
  decorationTimer(this, 555),
  image_BrowserApp(parent),
  imageResizerPermanent(resizerEntries)
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

    srand(time(0));
    CreateControls();
    //cout << "ImageBrowser::Create done CreateControls()" << endl;
    Centre();
////@end ImageBrowser creation

    decorationTimer.StartOnce(1000);
    imageResizerPermanent.Run();

    //ResizerEntry re;
    //re.fileName = wxFileName("C:\data\Test1.txt");
    //re.xSize = 666;
    //re.ySize = 999;
    //resizerEntries.emplace_back(re);
    //
    //re.fileName = wxFileName("C:\data\Test2.txt");
    //re.xSize = 222;
    //re.ySize = 111;
    //resizerEntries.emplace_back(re);

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
    FileNameList fileNameList;
    wxImage      image;
    wxString     s;

    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    thumbnailCanvas->StopLoadingThumbnails(data->m_path);


    int rescaleX = GetConfigParser()->GetIntWithDefault("rescaleX", 3000);
    int rescaleY = GetConfigParser()->GetIntWithDefault("rescaleY", 3000);
    wxString directory = data->m_path;

    //ChooseRescaleSize *custom = new ChooseRescaleSize(*imageResizer);
    ChooseRescaleSize custom(rescaleX, rescaleY);

    if (custom.ShowModal() == wxID_OK)
    {
        int maxWidth  = custom.GetWidth();
        int maxHeight = custom.GetHeight();

        GetConfigParser()->SetInt("rescaleX", maxWidth);            // Save the settings
        GetConfigParser()->SetInt("rescaleY", maxHeight);
        GetConfigParser()->Write();


        fileNameList.AddFilter(wxT("*.jpg"));           // We ony rescale JPEGs
        fileNameList.AddFilter(wxT("*.jpeg"));
        fileNameList.LoadFileList(directory);

        int n = fileNameList.NumFiles();

        for (int i = 0; i < n; i++)
        {
            wxFileName fullPath = fileNameList.files[i].fileName.GetFullPath();
            //cout << "  " << i << ": " << fullPath.GetFullPath() << endl;

            resizerEntries.emplace_back(fullPath, maxWidth, maxHeight);
        }
    }
    else
    {
        //cout << "Hit Cancel" << endl;
    }


}

/*
void ImageBrowser::MenuRescaleImages(wxCommandEvent &event)
{
    wxTreeItemId id = dirTreeCtrl->GetPopupMenuItem();
    wxDirItemData *data = (wxDirItemData*)(dirTreeCtrl->GetTreeCtrl()->GetItemData(id));

    thumbnailCanvas->StopLoadingThumbnails(data->m_path);


    int rescaleX = GetConfigParser()->GetIntWithDefault("rescaleX", 3000);
    int rescaleY = GetConfigParser()->GetIntWithDefault("rescaleY", 3000);
    ImageResizer* imageResizer = new ImageResizer(data->m_path, rescaleX, rescaleY);

    //ChooseRescaleSize *custom = new ChooseRescaleSize(*imageResizer);
    ChooseRescaleSize custom(*imageResizer);
    
    if (custom.ShowModal() == wxID_OK)
    {
        //cout << "Hit OK" << endl;
        imageResizer->maxWidth  = custom.GetWidth();
        imageResizer->maxHeight = custom.GetHeight();

        GetConfigParser()->SetInt("rescaleX", imageResizer->maxWidth);
        GetConfigParser()->SetInt("rescaleY", imageResizer->maxHeight);
        GetConfigParser()->Write();
        
        imageResizer->Run();
    }
    else
    {
        //cout << "Hit Cancel" << endl;
    }
}
*/
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


    //wxWindowID id = wxNewId();

    wxWindowID id = wxNewId();
    menu->Append(id, "Renumber Images");
    cout << "ID: " << id << endl;
    dirTreeCtrl->Bind(wxEVT_MENU,       &ImageBrowser::ReNumberImages, this, id);

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
    static wxString debuggingText = "Debugging Text";
    debuggingText.Append(text + "\n");

    if (debuggingWindow)
        debuggingWindow->SetValue(debuggingText);
}

#include "Iris_01.xpm"

void ImageBrowser::OnDropDirFiles(wxDropFilesEvent& event)
{
    int n = event.GetNumberOfFiles();
    wxString* s = event.GetFiles();
    cout << "ImageBrowser::OnDropDirFiles(" << ")" << endl;

    wxTreeCtrl *treeCtrl = dirTreeCtrl->GetTreeCtrl();

    wxPoint point = event.GetPosition();
    wxTreeItemId treeItemId = treeCtrl->HitTest(point);
    //cout << "treeItemId = " << treeItemId << endl;

    if (treeItemId)
    {
        wxDirItemData* data = (wxDirItemData*)(treeCtrl->GetItemData(treeItemId));

        for (int i = 0; i < n; i++)
        {
            cout << "  " << s[i] << endl;

            wxFileName fileName = s[i];

            wxString destination = data->m_path + fileName.GetFullName();
            cout << "  copy " << fileName.GetFullPath() << " to " << destination << endl;
            //wxCopyFile(name, destination);
        }

        //cout << "data = " << data << endl;
        //cout << "  Target = " << data->m_path << endl;
    }
}


void ImageBrowser::CreateControls()
{    
    ImageBrowser* itemFrame1 = this;
     
    itemFrame1->SetIcon(wxIcon(voo_icon));
    
	splitter1 = new wxSplitterWindow(this, -1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
	splitter1->SetSize(GetClientSize());
	splitter1->SetSashGravity(1.0);

    sBarGlobal  = new wxStatusBar(this, ID_STATUSBAR, wxST_SIZEGRIP | wxNO_BORDER);
    sBarGlobal->SetFieldsCount(4);
    this->SetStatusBar(sBarGlobal);

	dirTreeCtrl = new wxGenericDirCtrl(splitter1, ID_DIRECTORY_CTRL, _T("C:\\"), wxDefaultPosition, wxSize(640, 200), wxDIRCTRL_DIR_ONLY              |
																													  wxDIRCTRL_EDIT_LABELS           |
																													  wxDIRCTRL_POPUP_MENU           |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_NAME |
                                                                                                                      wxDIRCTRL_POPUP_MENU_SORT_DATE);

    treeCtrl = dirTreeCtrl->GetTreeCtrl();
    treeCtrl->Bind(wxEVT_TREE_ITEM_EXPANDED, &ImageBrowser::TreeExpanded, this, -1);

    treeCtrl->DragAcceptFiles(true);
    treeCtrl->Bind(wxEVT_DROP_FILES, &ImageBrowser::OnDropDirFiles, this, -1);

    if (1)
    {
        wxFrame *debuggingFrame = new wxFrame(this, -1, wxT("Debugging"), wxPoint(200, 600), wxSize(400, 400));
        debuggingWindow = new wxTextCtrl(debuggingFrame, -1, wxT("Test"), wxPoint(0, 0), wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
        wxBoxSizer *debugSizer = new wxBoxSizer(wxHORIZONTAL);
        debuggingFrame->SetSizer(debugSizer);
        debugSizer->Add(debuggingWindow, 1, wxEXPAND);
        debuggingFrame->Show(true);
    }

    imageViewer     = new ImageViewer(this, -1, wxT("Image Viewer"), wxDefaultPosition, wxDefaultSize, 0);
    thumbnailCanvas = new ThumbnailCanvas(this, splitter1, ID_SCROLLEDWINDOW, wxDefaultPosition, wxDefaultSize);
    thumbnailCanvas->SetImageViewer(imageViewer);
    imageViewer->SetThumbnailCanvas(thumbnailCanvas);
	thumbnailCanvas->SetScrollbars(10, 10, 50, 275);
	thumbnailCanvas->LoadThumbnails(".");

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

    wxAcceleratorEntry entries[3];
    entries[0].Set(wxACCEL_CTRL, (int) 'D', ID_DELETE_DIRECTORY);
    entries[1].Set(wxACCEL_CTRL, (int) 'K', ID_ARCHIVE_DIRECTORY);
    entries[2].Set(wxACCEL_CTRL, (int) 'R', ID_RANDOM_DIRECTORY);
    entries[1].Set(wxACCEL_CTRL, (int) 'T', ID_TOUCH_DIRECTORY);
    wxAcceleratorTable accel(3, entries);
    SetAcceleratorTable(accel);
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
}


void ImageBrowser::OnDirClicked(wxTreeEvent& event)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    if (dirTreeCtrl)
	{
        wxTreeItemId id = event.GetItem();
        currentDirectory = dirTreeCtrl->GetPath(id);
    
        dirTreeCtrl->GetTreeCtrl()->SetScrollPos(wxHORIZONTAL, 0, true);

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
    
    if (allowTreeDecoration)
    {
        GreyEmptyDirectories(*treeCtrl, id, knownDirList);
    }
}


void ImageBrowser::JumpToRandomDirectory(wxCommandEvent &event)
{
    static const int HAS_FILES = 1;
    static const int HAS_DIRS  = 2;

    int n = knownDirList.size();
    int r = rand() % n;

    if (!n)
        return;

    wxFileName fn(knownDirList[r]);
    wxString sub;
    bool success;

    for (int i = 0; i < 10; i++)
    {
        wxDir path(fn.GetFullPath());

        int contents = (path.HasFiles()*HAS_FILES) | (path.HasSubDirs()*HAS_DIRS);
        
        switch (contents)
        {
        case HAS_FILES:
            dirTreeCtrl->ExpandPath(fn.GetFullPath());
            return;

        case HAS_DIRS:
            success = path.GetFirst(&sub);
            if (success)
            {
                fn.AppendDir(sub);
            }
            break;

        case HAS_FILES + HAS_DIRS:
            dirTreeCtrl->ExpandPath(fn.GetFullPath());
            return;

        default:
            r = rand() % n;
            fn = knownDirList[r];
            break;
        }
    }

    dirTreeCtrl->ExpandPath(fn.GetFullPath());
}

void ImageBrowser::TouchDirectory(wxCommandEvent& event)
{
    wxFileName currentDirectory = GetCurrentDir();
    currentDirectory.Touch();
}


void ImageBrowser::OnKeyDown(wxKeyEvent &event)
{
    cout << "ImageBrowser::OnKeyDown(" << event.GetKeyCode() << ")" << endl;
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
    }
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
        return false;
    }

    bool cont = dir.GetFirst(&fn);

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

            wxString subPath = paths[i--];

            // if the next filename is actually a directory
            if (wxDirExists(subPath + wxFILE_SEP_PATH))
            {
                // delete this directory
                tabs += "    ";
                bool success = RemoveDirectory(subPath + wxFILE_SEP_PATH);

                if (success)
                {
                    paths.Remove(subPath);
                }
            }
            else
            {
                // otherwise attempt to delete this file
                if (!wxRemoveFile(subPath))
                { // failed
                    cout << tabs << "failed to delete file: " << subPath << endl;
                }
                else
                { // success
                    paths.Remove(subPath);
                }
            }
        } while ((attemptsRemaining--) && (paths.size()>0));
    }

    bool success = wxRmDir(pathName);

    tabs = tabs.Left(tabs.Length() - 4);
    return true;
}


void ImageBrowser::OnDeleteDirectory(wxCommandEvent &event)
{
    wxFileName parentPath  = currentDirectory;
    parentPath.RemoveLastDir();

    wxString pathToDelete = currentDirectory;

    //wxString pathToDelete;
    //wxString firstFile     = wxFindFirstFile(parentPath.GetFullPath());
    //bool     parentIsEmpty = firstFile.empty();
    //
    //if (parentIsEmpty)
    //{
    //    pathToDelete = parentPath.GetFullPath();
    //}
    //else
    //{
    //    pathToDelete = currentDirectory;
    //}

    thumbnailCanvas->StopLoadingThumbnails(pathToDelete);
    thumbnailCanvas->UnLoadThumbnails(pathToDelete);

    //return;
    bool success = DeleteDirectory(pathToDelete);

    //std::this_thread::sleep_for(std::chrono::milliseconds(50));       // Doesn't seem to help

    if (success)
    {
        dirTreeCtrl->SelectPath(pathToDelete);
        wxTreeItemId id = dirTreeCtrl->GetTreeCtrl()->GetSelection();
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));         // Doesn't seem to help
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
    return image_BrowserApp->GetConfigParser();
}

void ImageBrowser::ReportDirectoryInfo(wxString path, wxTreeItemId id, int flags)
{
}

void LoadImage2(wxImage &image, wxString fileName)
{
    image.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    wxFileName fn(fileName);
    wxString   extension = fn.GetExt().Upper();
    jpeg_load_state jpegLoadState;

    if ((extension == "JPG") ||
        (extension == "JPEG"))
    {
        int success = ReadJpegHeader(&jpegLoadState, (const  char*)fileName.c_str());
        jpeg_load_state *load_state = &jpegLoadState;

        int w = load_state->width, h = load_state->height;

        if (success)
        {
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

void ImageResizerPermanent::SaveState()
{
    wxTextFile out("rescaling.txt");

    out.Open();
    out.Clear();

    int i, n = resizerEntries.size();

    for (i=0; i<n; i++)
    {
        wxString record;
        record.Printf("%s\n%d\n%d", resizerEntries[i].fileName.GetFullPath(),
                                    resizerEntries[i].xSize,
                                    resizerEntries[i].ySize);

        out.AddLine(record);
    }
    out.Write();
    out.Close();
}

void ImageResizerPermanent::LoadState()
{
    wxTextFile in("rescaling.txt");

    if (!in.Exists())
        return;

    in.Open();

    int i, n = in.GetLineCount();

    for (i = 0; i < n; i+=3)
    {
        wxString fileName, xSizeString, ySizeString;
        long     xSize, ySize;

        fileName    = in.GetLine(i + 0);
        xSizeString = in.GetLine(i + 1);
        ySizeString = in.GetLine(i + 2);

        bool xSuccess = xSizeString.ToLong(&xSize);
        bool ySuccess = ySizeString.ToLong(&ySize);

        if (!xSuccess || !ySuccess)
            break;

        resizerEntries.emplace_back(fileName, xSize, ySize);
    }
    in.Close();
}

wxThread::ExitCode ImageResizerPermanent::Entry()
{
    const int X_OVERSIZE = 1;
    const int Y_OVERSIZE = 2;
    const int RESCALED   = 4;

    LoadState();

    while (1)
    {
        if (TestDestroy())
        {
            break;
        }

        ResizerEntry resizerEntry     = resizerEntries.pop_front();
        wxFileName   fullPath         = resizerEntry.fileName.GetFullPath();
        wxString     fileNameFragment = fullPath.GetName().Left(10) + "..." + fullPath.GetExt();
        int          maxWidth         = resizerEntry.xSize;
        int          maxHeight        = resizerEntry.ySize;
        int          imagesRemaining  = resizerEntries.size();
        wxImage      image;

        STATUS_TEXT(STATUS_BAR_INFORMATION, "Loading %s %d remain", fileNameFragment, imagesRemaining);

        LoadImage2(image, fullPath.GetFullPath());

        if (!image.IsOk())
        {
            continue;
        }

        float imageWidth  = image.GetSize().GetWidth();
        float imageHeight = image.GetSize().GetHeight();
        int   newWidth    = image.GetSize().GetWidth();
        int   newHeight   = image.GetSize().GetHeight();

        float ratioX = imageWidth  / (float)maxWidth;
        float ratioY = imageHeight / (float)maxHeight;

        int   flags = 0;

        if (ratioX > 1.0)   flags |= X_OVERSIZE;
        if (ratioY > 1.0)   flags |= Y_OVERSIZE;


        switch (flags)
        {
        default:
        case 0:
            break;

        case X_OVERSIZE:
            flags |= RESCALED;
            newWidth  = int(imageWidth / ratioX + 0.5);
            newHeight = int(imageHeight / ratioX + 0.5);
            break;

        case Y_OVERSIZE:
            flags |= RESCALED;
            newWidth  = int(imageWidth / ratioY + 0.5);
            newHeight = int(imageHeight / ratioY + 0.5);
            break;

        case X_OVERSIZE | Y_OVERSIZE:
            flags |= RESCALED;
            if (ratioX > ratioY)
            {
                newWidth  = int(imageWidth / ratioX + 0.5);
                newHeight = int(imageHeight / ratioX + 0.5);
            }
            else
            {
                newWidth  = int(imageWidth / ratioY + 0.5);
                newHeight = int(imageHeight / ratioY + 0.5);
            }
            break;
        }

        if (flags & RESCALED)
        {
            STATUS_TEXT(STATUS_BAR_INFORMATION, "Rescaling %s %d remain", fileNameFragment, imagesRemaining);
            image.Rescale(newWidth, newHeight, wxIMAGE_QUALITY_HIGH);
        }

        STATUS_TEXT(STATUS_BAR_INFORMATION, "Saving %s %d remain", fileNameFragment, imagesRemaining);

        JpegWrite(fullPath.GetFullPath(), newWidth, newHeight, image.GetData());
        image.Destroy();
        SaveState();
    }

    STATUS_TEXT(STATUS_BAR_INFORMATION, "  ");
    return 0;
}


wxThread::ExitCode ImageResizer::Entry()
{
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
        //cout << "  " << i << ": " << fullPath.GetFullPath() << endl;

        STATUS_TEXT(STATUS_BAR_INFORMATION, "Loading %d/%d", i, n);

        LoadImage2(image, fullPath.GetFullPath());

        if (!image.IsOk())
        {
            continue;
        }

        float imageWidth  = image.GetSize().GetWidth();
        float imageHeight = image.GetSize().GetHeight();
        int   newWidth    = image.GetSize().GetWidth();
        int   newHeight   = image.GetSize().GetHeight();

        float ratioX = imageWidth  / (float)maxWidth;
        float ratioY = imageHeight / (float)maxHeight;

        int   flags = 0;

        if (ratioX > 1.0)   flags |= X_OVERSIZE;
        if (ratioY > 1.0)   flags |= Y_OVERSIZE;


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

int ChooseRescaleSize::GetWidth()
{
    long width;
    widthCtrl->GetValue().ToLong(&width);
    return width;
}

int ChooseRescaleSize::GetHeight()
{
    long height;
    heightCtrl->GetValue().ToLong(&height);
    return height;
}

ChooseRescaleSize::ChooseRescaleSize(int xs, int ys)
: wxDialog(NULL, wxID_ANY, wxT("Select maximum size"), wxDefaultPosition, wxSize(250, 230))
{
    wxBoxSizer* vbox  = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* hbox2 = new wxBoxSizer(wxHORIZONTAL);


    wxString widthString;
    wxString heightString;

     widthString.Printf("%d", xs);
    heightString.Printf("%d", ys);

    wxStaticText *labelWidth  = new wxStaticText(this, -1, ("Max Width: "),  wxPoint( 15, 30), wxSize(100, 12));
    wxStaticText *labelHeight = new wxStaticText(this, -1, ("Max Height: "), wxPoint( 15, 30), wxSize(100, 12));
    widthCtrl   = new   wxTextCtrl(this, -1, widthString,  wxPoint(100, 30), wxSize(100, 12));
    heightCtrl  = new   wxTextCtrl(this, -1, heightString, wxPoint(100, 30), wxSize(100, 12));

    hbox1->Add(labelWidth,  1, wxEXPAND);
    hbox1->Add(widthCtrl,   1, wxEXPAND);
    hbox2->Add(labelHeight, 1, wxEXPAND);
    hbox2->Add(heightCtrl,  1, wxEXPAND);

    vbox->Add(hbox1, 1, wxEXPAND);
    vbox->Add(hbox2, 1, wxEXPAND);
    vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND);

    SetSizer(vbox);

    Centre();
}

