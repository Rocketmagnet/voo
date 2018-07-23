#include "thumbnail_canvas.h"
#include "imageviewer.h"

#include "wx/thread.h"
#include "wx/image.h"
#include "wx/imaggif.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
#include "wx/dir.h"
#include "wx/log.h"
#include "status_bar.h"

#include <iostream>
using namespace std;

extern "C"
{
    #include "jpeg_turbo.h"
};


wxBEGIN_EVENT_TABLE(  ThumbnailCanvas, wxScrolledWindow)
    EVT_PAINT(        ThumbnailCanvas::OnPaint)
    EVT_SIZE(         ThumbnailCanvas::OnSize)
    EVT_MOUSE_EVENTS( ThumbnailCanvas::OnMouseEvent)
    EVT_KEY_DOWN(     ThumbnailCanvas::OnKeyEvent)
    EVT_SET_FOCUS(    ThumbnailCanvas::OnFocusEvent)
    EVT_KILL_FOCUS(   ThumbnailCanvas::OnFocusKillEvent)
    EVT_IDLE(         ThumbnailCanvas::OnIdle)
    EVT_MOUSEWHEEL(   ThumbnailCanvas::OnMouseWheel)
    EVT_CLOSE(        ThumbnailCanvas::OnClose)
wxEND_EVENT_TABLE()
   
wxSize      Thumbnail::tnSize(100,100);
wxArrayInt  Thumbnail::arrayIntStatic;
int         Thumbnail::selectBorderSize;
int         Thumbnail::labelHeight;
wxColor     Thumbnail::backgroundColor;



bool SortedVectorInts::Test()
{
    AddSingle(5);
    AddSingle(1);
    AddSingle(2);
    AddSingle(6);
    AddSingle(0);
    AddSingle(2);
    Print();
    cout << "Contains -1 " << Contains(-1) << endl;
    cout << "Contains  0 " << Contains(0) << endl;
    cout << "Contains  2 " << Contains(2) << endl;
    cout << "Contains  3 " << Contains(3) << endl;
    cout << "Contains  6 " << Contains(6) << endl;
    cout << "Contains  7 " << Contains(7) << endl;

    RemoveSingle(1);
    cout << "Contains 1 " << Contains(1) << endl;
    Print();

    SetRange(10, 15);
    cout << "Contains 3 " << Contains(3) << endl;
    Print();

    Clear();
    cout << "Contains 3 " << Contains(3) << endl;
    Print();

    return true;
}


wxThread::ExitCode ThumbnailLoader::Entry()
{
    wxLogNull logNo;													// ... instead logging is suspended while this object is in scope
    wxImage   image;

    image.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    wxFileName fn(fileName);

    if ((fn.GetExt().Upper() == "JPG") ||
        (fn.GetExt().Upper() == "JPEG"))
    {
        cout << "Using JpegTurbo to load thumbnail " << fileName << endl;

        jpeg_load_state *load_state = ReadJpegHeader((const  char*)fileName.c_str());

        if (load_state)
        {
            int w = load_state->width, h = load_state->height;

            image.Create(w, h);
            image.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
            JpegRead(image.GetData(), load_state);

            thumbnail.imageSize = image.GetSize();
            wxSize newSize = thumbnail.GetTnImageSize(image.GetSize(), thumbnail.tnSize);
            image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_BILINEAR);
            thumbnail.bitmap = wxBitmap(image);
            image.Destroy();
            thumbnail.imageLoaded = true;
        }
    }
    else
    {
        if (image.LoadFile(fileName))
        {
            thumbnail.imageSize = image.GetSize();
            wxSize newSize = thumbnail.GetTnImageSize(image.GetSize(), thumbnail.tnSize);
            image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_BILINEAR);
            thumbnail.bitmap = wxBitmap(image);
            image.Destroy();
            thumbnail.imageLoaded = true;
        }
        else
        {
            cout << "Failed to load " << fileName << endl;
        }
    }
    return 0;
}




Thumbnail::Thumbnail(const wxPoint &pos, wxFileName path)
: fullPath(path),
  position(pos),
  imageLoaded(false),
  hasBeenDrawn(false),
  imageSize(0, 0),
  thumbnailLoader(0)
{
	thumbnailLoader = new ThumbnailLoader(path.GetFullPath(), *this);
}



Thumbnail::~Thumbnail()
{
	if (!imageLoaded)
		if (thumbnailLoader->IsRunning())
			thumbnailLoader->Kill();

    //cout << "Destructing" << endl;
}


void Thumbnail::DrawLabelClipped(wxPaintDC &dc, wxString &labelRef, wxRect &rectangle)
{
    wxString label(labelRef);                                       // Take a copy of the label, so we can truncate it.
    int availableWidth = rectangle.GetWidth();
    int labelWidth;

    arrayIntStatic.Clear();
    dc.GetPartialTextExtents(label, arrayIntStatic);                // Work out how much space the rendered label takes up
    labelWidth = arrayIntStatic.Last();                             // This is the width of the whole label

    int n = label.Len();
    for (int i=0; i<n; i++)                                         // Loop through the label until we find the length that just fits inside the rectangle
    {
        if (arrayIntStatic[i] > availableWidth)                     // Label too big? 
        {
            label.Truncate(i);                                      // truncate it here.
            labelWidth = availableWidth;                            // And it ends up being the full width.
            break;
        }
    }


    //wxPoint textPosition(rectangle.GetLeft(), rectangle.GetTop()+tnSize.GetHeight() + 5);

    //wxRect textRectangle(textPosition, wxSize(tnSize.GetWidth(), 20));

    dc.SetTextForeground(wxColour(200, 200, 200));
    dc.DrawLabel(label, rectangle, wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL);
     

}


void Thumbnail::Erase(wxPaintDC &dc)
{
    dc.SetBrush(wxBrush(backgroundColor));
    dc.SetPen(  wxPen(  backgroundColor, 1));

    wxRect rect(position.x         -  selectBorderSize     - 3,             // Top Left Coordinate
                position.y         -  selectBorderSize     - 3,
                tnSize.GetWidth()  + (selectBorderSize * 2 + 3),        // Size
                tnSize.GetHeight() + (selectBorderSize * 2 + 3));

    dc.DrawRectangle(rect);
}



void Thumbnail::Draw(wxPaintDC &dc, bool selected, bool cursor, bool inFocus)
{
    wxRect textRectangle(position.x, position.y + tnSize.GetHeight()+5, tnSize.GetWidth(), 20);
	wxPen pen;

    if (imageLoaded && bitmap.IsOk())
    {
        if (cursor)
        {
			if (inFocus)
			{
				dc.SetBrush(wxBrush(wxS("blue")));
				dc.SetPen(*wxBLACK_PEN);
			}
			else
			{
				dc.SetBrush(wxBrush(wxColor(48, 48, 48)));
				dc.SetPen(wxColor(32, 32, 32));
			}


            dc.DrawRectangle(position.x         - selectBorderSize     - 3,
                             position.y         - selectBorderSize     - 3,
                             tnSize.GetWidth()  + selectBorderSize * 2 + 3, 
                             tnSize.GetHeight() + selectBorderSize * 2 + 3);
        }

        if (selected)
        {
			if (inFocus)
			{
				dc.SetBrush(wxBrush(wxS("blue")));
				dc.SetPen(wxColor(32, 32, 196));
			}
			else
			{
				dc.SetBrush(wxBrush(wxColor(64, 64, 128)));
				dc.SetPen(wxColor(64, 64, 128));
			}

            dc.DrawRectangle(position.x         - selectBorderSize     + 3,
                             position.y         - selectBorderSize     + 3,
                             tnSize.GetWidth()  + selectBorderSize * 2 - 3,
                             tnSize.GetHeight() + selectBorderSize * 2 - 3);

            dc.SetTextForeground(wxColor(255, 255, 255));
        }
        else
        {
            dc.SetTextForeground(wxColor(128, 128, 128));
        }

        int x = position.x + (tnSize.GetWidth()  - bitmap.GetWidth() ) / 2;
        int y = position.y + (tnSize.GetHeight() - bitmap.GetHeight()) / 2;

        dc.DrawBitmap(bitmap, x, y, false);
    }
    else
    {
        dc.SetBrush(wxBrush(wxColor(16,16,16)));
        dc.SetPen(*wxBLACK_PEN);

        dc.DrawRectangle(position.x, position.y, tnSize.GetWidth(), tnSize.GetHeight());
    }

    DrawLabelClipped(dc, fullPath.GetFullName(), textRectangle);
}


bool Thumbnail::IsMouseInside(const wxPoint &mousePos)
{
    wxRect rect(position, tnSize);

    return rect.Contains(mousePos);
}



ThumbnailCanvas::ThumbnailCanvas(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
: wxScrolledWindow(parent, id, pos, size, wxSUNKEN_BORDER | wxVSCROLL | wxEXPAND | wxWANTS_CHARS),
  fileNameList(),
  inFocus(true),
  cursorP(tnColumns, fileNameList),
  selectionStart(0),
  backgroundColor(wxColor(64, 64, 64)),
  tnSize(150, 150),
  thBorder(5),
  redrawType(REDRAW_ALL),
  tnSpacingX(20),
  tnSpacingY(35),
  selectionSetP(256),
  redrawSetP(256),
  waitingSet(256),
  loadingSet(8),
  maxLoading(1)
{
    SetBackgroundColour(backgroundColor);

    fileNameList.AddFilter(_T("*.png"));
    fileNameList.AddFilter(_T("*.gif"));
    fileNameList.AddFilter(_T("*.jpg"));
    fileNameList.AddFilter(_T("*.jpeg"));

    Thumbnail::SetSize(tnSize);
    Thumbnail::SetSelectBorder(3);
    Thumbnail::SetLabelHeight(26);
    Thumbnail::SetBackgroundColor(backgroundColor);

    RecalculateRowsCols();

    //imageViewer = new ImageViewer(this, -1, _T("Image Viewer"), wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP);
    imageViewer = new ImageViewer(this, -1, _T("Image Viewer"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER );
    imageViewer->SetFileNameList(&fileNameList);
    
    DragAcceptFiles(true);
    //Connect(wxEVT_DROP_FILES, ThumbnailCanvas::OnDropFiles, NULL, ThumbnailCanvas::OnDropFiles);
    //Connect(wxEVT_DROP_FILES, &ThumbnailCanvas::OnDropFiles, NULL, this);
    //Connect(wxEVT_DROP_FILES, )
    Bind(wxEVT_DROP_FILES, &ThumbnailCanvas::OnDropFiles, this, -1);
}

void ThumbnailCanvas::OnDropFiles(wxDropFilesEvent& event)
{
    cout << "ThumbnailCanvas::OnDropFiles()" << endl;

    if (event.GetNumberOfFiles() > 0)
    {
        wxString* dropped = event.GetFiles();
        wxASSERT(dropped);

        wxBusyCursor busyCursor;
        wxWindowDisabler disabler;
        //wxBusyInfo busyInfo(_("Adding files, wait please..."));

        
        wxArrayString files;

        for (int i = 0; i < event.GetNumberOfFiles(); i++)
        {
            wxString name = dropped[i];
            wxFileName fileName = name;

            wxString destination = fileNameList.directory.GetName() + wxT("\\") + fileName.GetFullName();
            cout << "  copy " << name << " to " << destination << endl;
            wxCopyFile(name, destination);

            thumbnails.emplace_back(wxPoint(0, 0), destination);
            thumbnailPointers.push_back(&thumbnails.back());
            waitingSet.AddSingle(thumbnails.size()-1);
            fileNameList.AddFileToList(destination);
        }
        fileNameList.Resort();
        RecalculateRowsCols();
        //wxTextCtrl* textCtrl = dynamic_cast<wxTextCtrl*>(event.GetEventObject());
        //wxASSERT(textCtrl);
        //textCtrl->Clear();
        //for (size_t i = 0; i < files.size(); i++) {
        //    *textCtrl << files[i] << wxT('\n');
        //}
    }
}

ThumbnailCanvas::~ThumbnailCanvas()
{
}


// This would be called if the window was resized.
// We can recalculate them without changing which ones are selected.
//   Recalculate positions of all thumbnails
//   Recalculate tnColumns, tnRows
// 
void ThumbnailCanvas::RecalculateRowsCols()
{
    int xSize = GetSize().GetWidth();
    size_t i = 0;
    size_t n = thumbnailPointers.size();
    size_t x, y;

    int tnJumpX = tnSize.GetWidth()  + tnSpacingX;                          // The distance from the corner of one ThumbNail
    int tnJumpY = tnSize.GetHeight() + tnSpacingY;                          // to the corner of the next

    xSize -= tnSpacingX;                                                    // Because there'll be a border at the left edge

    tnColumns = xSize / tnJumpX;                                            // Calculate the number of columns
    if (tnColumns < 1)                                                      // Can't have zero columns!
    {
        tnColumns = 1;
    }
    tnRows    = (int)ceil(((double)thumbnailPointers.size() / tnColumns));         // Calculate the number of rows

    int virtualX = xSize;                                                   // 
    int virtualY = (tnRows) * tnJumpY + tnSpacingY;                         // 
    SetVirtualSize(virtualX, virtualY);                                     // Set the size of the scroll window

    x = 0;
    y = 0;
    for (i = 0; i < n; i++)                                                 // Now lay out the ThumbNails on the virtual window
    {
        int xPos = x * (tnSize.GetWidth()  + tnSpacingX) + tnSpacingX;
        int yPos = y * (tnSize.GetHeight() + tnSpacingY) + tnSpacingY;

        wxPoint pos(xPos, yPos);
        thumbnailPointers[i]->SetPosition(pos);

        x++;
        if (x == tnColumns)
        {
            x = 0;
            y++;
        }
    }

    redrawSetP.Clear();
    redrawType = REDRAW_ALL;
}



void ThumbnailCanvas::OnPaint(wxPaintEvent &event)
{
    int n = thumbnails.size();
    bool selected;
    bool onCursor;
    wxPaintDC dc(this);
    PrepareDC(dc);
    wxString cursorFileName;
    wxSize   cursorImageSize;

    cout << "ThumbnailCanvas::OnPaint" << endl;
    if (!n)
        return;

    //cout << "Redraw type " << redrawType << endl;
    switch (redrawType)
    {
        case REDRAW_ALL:
            cout << "Redrawing ALL" << endl;
            for (int i = 0; i < n; i++)
            {
                cout << "  " << i << endl;
                selected = selectionSetP.Contains(i);
                if (cursorP.GetNumber() == i)
                {
                    onCursor = true;
                    cursorFileName  = thumbnailPointers[i]->GetFullPath().GetFullName();
                    cursorImageSize = thumbnailPointers[i]->GetImageSize();
                    //STATUS_TEXT(0, "%s  (%d, %d)", cursorFileName, cursorImageSize.GetWidth(), cursorImageSize.GetHeight());
                }
                else
                {
                    onCursor = false;
                }
                cout << "    Drawing" << endl;
                thumbnailPointers[i]->Draw(dc, selected, onCursor);
            }
            break;

        case REDRAW_SELECTION:
            //cout << "Redrawing Selection" << endl;
            //cout << "  redrawSetP.size() = " << redrawSetP.size() << endl;
            redrawSetP.Print();

            for (int i = 0; i<redrawSetP.size(); i++)
            {

				int th = redrawSetP[i];
                //cout << "  th = " << th << endl;
                if (th >= 0)
                {
                    //cout << "    Erasing" << endl;
                    thumbnailPointers[th]->Erase(dc);
                }
            }

            for (int i=0; i<redrawSetP.size(); i++)
            {
                int th = redrawSetP[i];
                //cout << "  redrawSetP.size() = " << redrawSetP.size() << endl;
                //cout << "  th = " << th << endl;
                selected = selectionSetP.Contains(th);

				if (cursorP.GetNumber() == th)
                {
                    onCursor = true;
                    cursorFileName  = thumbnailPointers[th]->GetFullPath().GetFullName();
                    cursorImageSize = thumbnailPointers[th]->GetImageSize();
                    //STATUS_TEXT(0, "%s  (%d, %d)", cursorFileName, cursorImageSize.GetWidth(), cursorImageSize.GetHeight());
                }
                else
                {
                    onCursor = false;
                }


                if (th >= 0)
                {
                    //cout << "    Drawing" << endl;
                    //cout << "  draw " << th << " " << selected << " " << onCursor << " " << cursorP.GetNumber() << endl;
                    thumbnailPointers[th]->Draw(dc, selected, onCursor, inFocus);
                }
            }

            redrawSetP.Clear();
            break;
    }
    
    //cout << "Redraw All" << endl;
    redrawType = REDRAW_ALL;
}

void ThumbnailCanvas::HandleCursorScrolling()
{
    int cursorNumber = cursorP.GetNumber();

    if ((cursorNumber == -1) || (thumbnailPointers.size() == 0))
    {
        Scroll(0, 0);
        return;
    }

    int xPPU, yPPU;
    GetScrollPixelsPerUnit(&xPPU, &yPPU);

    Thumbnail *tn = thumbnailPointers[cursorNumber];
    wxPoint thPosition = tn->GetPosition();

    // Check if cursor is off the top of the screen
    int logical_X, logical_Y;
    CalcScrolledPosition(thPosition.x, thPosition.y, &logical_X, &logical_Y);

    if (logical_Y < 35)
    {
        Scroll(0, (thPosition.y - 35) / xPPU);
        return;
    }

    // Check if cursor is off the bottom of the screen
    CalcScrolledPosition(thPosition.x, thPosition.y + tnSize.y+35, &logical_X, &logical_Y);

    int overlap = logical_Y - GetSize().GetHeight();

    if (overlap > 0)
    {
        wxPoint scrollPosition = CalcUnscrolledPosition(wxPoint(0, 0));
        int newScrollPosY = scrollPosition.y + overlap;
        Scroll(0, (newScrollPosY) / xPPU);
    }
}

void ThumbnailCanvas::OnKeyEvent(wxKeyEvent &event)
{
    if (HandleAsNavigationKey(event))
        return;

    cout << "OnKeyEvent(" << event.GetKeyCode() << ")" << endl;
    int cursorToErase = -1;

    if (event.GetKeyCode() == 'D')
    {
        cout << "ThumbnailCanvas(D)" << endl;
    }

    switch (event.GetKeyCode())
    {
        case WXK_UP:     cursorP.Move( 0, -1);							    break;
        case WXK_DOWN:   cursorP.Move( 0,  1);							    break;
        case WXK_LEFT:   cursorP.Move(-1,  0);							    break;
        case WXK_RIGHT:  cursorP.Move( 1,  0);							    break;
        case WXK_RETURN: imageViewer->DisplayImage(cursorP.GetNumber());    break;

        case WXK_DELETE: DeleteSelection();                                 return;

        default:
			//cout << "Key: " << event.GetKeyCode() << endl;
            event.Skip();
            return;
    }

	selectionStart = cursorP.GetNumber();

	cursorP.SetupRedraw(redrawSetP);
	redrawSetP.AddFrom(selectionSetP);

	if (event.ShiftDown())
	{
		selectionSetP.SelectTo(cursorP.GetNumber());
	}
	else
	{
		selectionSetP.SelectFrom(cursorP.GetNumber());
	}

	selectionSetP.Print();
	redrawSetP.AddFrom(selectionSetP);
	//redrawSetP.Print();

    //cout << "Redraw Selection" << endl;
    redrawType = REDRAW_SELECTION;
    Refresh(DONT_ERASE_BACKGROUND);
    HandleCursorScrolling();
    UpdateStatusBar_File();

    event.Skip(true);
}

void ThumbnailCanvas::OnFocusEvent(wxFocusEvent &event)
{
    //cout << "ThumbnailCanvas::OnFocusEvent" << endl;
	inFocus = true;

    if ((cursorP.GetNumber() > -1) && (cursorP.GetNumber() < thumbnails.size()))
    {
        //cursorNumberthumbnails[cursorNumber].Select();
        cursorP.SetupRedraw(redrawSetP);
        cout << "Redraw Selection Focus" << endl;
        //redrawType = REDRAW_SELECTION;
        redrawType = REDRAW_ALL;
        Refresh(DONT_ERASE_BACKGROUND);
    }
    else
    {
		cursorP.SetTo(0);
    }
    UpdateStatusBar_File();
}

void ThumbnailCanvas::OnFocusKillEvent(wxFocusEvent &event)
{
	inFocus = false;

    //cout << "ThumbnailCanvas::OnFocusKillEvent" << endl;

    if ((cursorP.GetNumber() > -1) && (cursorP.GetNumber() < thumbnails.size()))
    {
		//thumbnails[cursorNumber].UnSelect();
		cursorP.SetupRedraw(redrawSetP);
		redrawType = REDRAW_SELECTION;
        Refresh(DONT_ERASE_BACKGROUND);         // Don't erase background
    }
    else
    {
		cursorP.SetTo(0);
    }

    //cout << "done" << endl;
} 


void ThumbnailCanvas::OnIdle(wxIdleEvent &event)
{
    if (HandleThumbnailLoading())
    {
        event.RequestMore();
        //STATUS_TEXT(3, "Loading Images ...");
    }
    else
    {
        //STATUS_TEXT(3, "");
    }
}


void ThumbnailCanvas::CreateAntiAliasedBitmap()
{

}


void ThumbnailCanvas::ClearThumbnails()
{
    thumbnails.clear();
    thumbnailPointers.clear();
    SetCursor(-1);
    ClearStatusBar();
}

wxString HumanFileSize(int bytes)
{
    wxString s;
    if (bytes < 1000)
    {
        s.Printf("%dB", bytes);
    }
    else
    {
        char mags[] = "  KMGT";

        int scale = 1;
        int si = 0;
        while (scale <= bytes)
        {
            scale *= 1000;
            si++;
        }
        scale /= 1000;

        s.Printf("%5.1f%cB", (double)bytes / (double)scale, mags[si]);
    }
    return s;
}


void ThumbnailCanvas::DirectoryWasDeleted(wxString path)
{
    cout << "ThumbnailCanvas::DirectoryWasDeleted(" << path << ")" << endl;

    if (path == fileNameList.directory.GetName())
    {
        cout << "This directory was deleted" << endl;

        ClearThumbnails();

        selectionSetP.Clear();
        redrawSetP.Clear();
        waitingSet.Clear();
        loadingSet.Clear();
    }
}


void ThumbnailCanvas::LoadThumbnails(wxString directory)
{
	//cout << "LoadThumbnails(" << directory << ")" << endl;

    ClearThumbnails();
    fileNameList.LoadFileList(directory);

    int n = fileNameList.files.size();
    //thumbnails.reserve(n);
    //thumbnailPointers.reserve(n);
    Scroll(0, 0);

    totalDirectorySizeBytes = 0;

    if (n > 0)
        SetCursor(0);

    for (int i = 0; i<n; i++)
    {
        wxString fullPath = directory;
        fullPath += wxT("\\");
        fullPath += fileNameList.files[i].fileName.GetFullPath();
        wxFileName fn(fullPath);
        totalDirectorySizeBytes += fn.GetSize();
        thumbnails.emplace_back(wxPoint(0,0), fullPath);
    }

    wxString hrs = HumanFileSize(totalDirectorySizeBytes.GetLo());
    STATUS_TEXT(STATUS_BAR_DIRECTORY_SUMMARY, "%d files (%s)", n, hrs.c_str());
    
    for (int i = 0; i < n; i++)
    {
        thumbnailPointers.push_back( &thumbnails[i] );
    }

    if (n > 0)
    {
        //cout << "waitingSet.SetRange(0, " << n - 1 << ")" << endl;
        waitingSet.SetRange(0, n - 1);
    }
	else
	{
		waitingSet.Clear();
	}
    loadingSet.Clear();
    redrawSetP.Clear();

    RecalculateRowsCols();
	//cout << "LoadThumbnails(" << directory << ")  done" << endl;
}

bool ThumbnailCanvas::HandleThumbnailLoading()
{
    int i, n = loadingSet.size();

    //cout << "ThumbnailCanvas::HandleThumbnailLoading()" << endl;

    for (i = 0; i < n; i++)
    {
        int th = loadingSet[i];
        //cout << "  loadingSet[" << i << "] = " << th << endl;
        if (thumbnails[th].ImageIsLoaded())
        {
            //cout << th << " is complete" << endl;
            loadingSet.RemoveSingle(th);
            redrawSetP.AddSingle(th);
            i--;
            n--;
        }
    }

    //cout << n << " images loading" << endl;

    n = maxLoading - loadingSet.size();
    if (n > waitingSet.size())
        n = waitingSet.size();

    //cout << "Starting " << n << " more images loading" << endl;

    for (i = 0; i < n; i++)
    {
        int th = waitingSet[i];
        if (th >= 0)
        {
            thumbnails[th].StartLoadingImage();
            loadingSet.AddSingle(th);
            waitingSet.RemoveSingle(th);
            i--;
            n--;
        }
        //cout << "  * starting " << th << endl;
    }

    if (redrawSetP.size())
    {
        redrawType = REDRAW_SELECTION;
        Refresh(DONT_ERASE_BACKGROUND);
    }

    if (loadingSet.size() || waitingSet.size())
        return true;
    else
        return false;                                // Means we've finished loading
}

void ThumbnailCanvas::OnSize(wxSizeEvent &event)
{
    //mySize = event.GetSize();
    //SetSize(mySize);

    RecalculateRowsCols();

    Refresh(ERASE_BACKGROUND);
}



int ThumbnailCanvas::GetThumbnailFromPosition(wxPoint &position)
{
    size_t n = thumbnails.size();

    for (size_t i = 0; i<n; i++)
    {
        if (thumbnails[i].IsMouseInside(position))
            return i;
    }

    return -1;
}


/*
void ThumbnailCanvas::SetCursorTo(int n)
{
    cout << "SetCursorTo(" << n << ")  erase: " << cursorNumber << endl;

    if (n >= 0)
    {
        redrawSet.CopyFrom(selectionSet);
        redrawSet.AddSingle(n);
        redrawSet.AddSingle(cursorNumber);
        cursorNumber = n;
        RecalculateXyFromCursor();
        redrawType = REDRAW_SELECTION;
        cout << "Expecting to redraw " << n << " and " << cursorNumber << endl;
        redrawSet.Print();
        Refresh(DONT_ERASE_BACKGROUND);
    }
}
*/

void ThumbnailCanvas::OnMouseEvent(wxMouseEvent &event)
{
    wxPoint physicalPosition = event.GetPosition();
    wxPoint  logicalPosition = CalcUnscrolledPosition(physicalPosition);

    //ReportInt2(1, "Mouse: (%d, %d)", logicalPosition.x, logicalPosition.y);
    //cout << "OnMouseEvent" << endl;
    if (event.LeftDClick())
    {
        cout << "Dclick" << endl;
        int th = GetThumbnailFromPosition(logicalPosition);
        imageViewer->DisplayImage(th);
        cout << "Redraw All" << endl;
        redrawType = REDRAW_ALL;
    }

    if (event.LeftDown())
    {
        cout << "Click" << endl;
        SetFocus();

        int th = GetThumbnailFromPosition(logicalPosition);
        
        if (th > -1)
        {
            if (event.ShiftDown())
            {
                //cout << "Selecting range " << selectionStart << "-" << th << endl;
				int selectionEnd = th;
				redrawSetP.SetRange(selectionEnd, th);
                selectionSetP.SetRange(selectionStart, selectionEnd);
                redrawType = REDRAW_SELECTION;
                cout << "Redraw Selection" << endl;
                Refresh(DONT_ERASE_BACKGROUND);
            }
            else
            {
                //cout << "Redrawing:" << endl;
                //cout << "  selectionSet: ";  selectionSet.Print();
                //cout << "     redrawSet: ";     redrawSet.Print();

                cursorP.SetTo(th);
                cursorP.SetupRedraw(redrawSetP);
                redrawSetP.AddFrom(selectionSetP);
                selectionSetP.Clear();
                selectionStart = th;
                UpdateStatusBar_File();

                cout << "Redraw Selection" << endl;
                redrawType = REDRAW_SELECTION;
                Refresh(DONT_ERASE_BACKGROUND);
            }

            dragState = TNC_DRAG_STATE_CLICKED;
            clickPoint = event.GetPosition();
        }
        return;
    }


    //cout << "Done" << endl;
    //Update();
}

void ThumbnailCanvas::ClearStatusBar()
{
    STATUS_TEXT(STATUS_BAR_DIRECTORY_SUMMARY, "");
    STATUS_TEXT(STATUS_BAR_FILE_INFO,         "");
    STATUS_TEXT(STATUS_BAR_FILE_FORMAT,       "");
    STATUS_TEXT(STATUS_BAR_FILE_DIMENSIONS,   "");
}

void ThumbnailCanvas::UpdateStatusBar_File()
{
    if (selectionSetP.size() == 0)
    {
        if (thumbnailPointers.size() > 0)
        {
            Thumbnail *tn = thumbnailPointers[cursorP.GetNumber()];

            cout << "Update Status: " << tn->GetFullPath().GetFullPath() << endl;

            wxFileName fn = tn->GetFullPath();
            wxDateTime date = fn.GetModificationTime();
            wxString   dateString = date.Format("%d/%m/%Y  %H:%M");
            wxSize     imageSize = tn->GetImageSize();

            STATUS_TEXT(STATUS_BAR_FILE_INFO, "%s,  %s", fn.GetHumanReadableSize().c_str(), dateString);
            STATUS_TEXT(STATUS_BAR_FILE_FORMAT, fn.GetFullName().c_str());
            STATUS_TEXT(STATUS_BAR_FILE_DIMENSIONS, "(%d x %d)", imageSize.GetWidth(), imageSize.GetHeight());
        }
    }
}

wxPoint ThumbnailCanvas::GetThumbnailPosition(size_t n)
{
    int xSize = GetSize().GetX();

    int x = 50, y = 50;
    int maxX = xSize - tnSize.GetWidth() - 20;

    for (int i = 0; i<n; i++)
    {
        x += tnSize.GetWidth() + 20;
        if (x > maxX)
        {
            x = 50;
            y += tnSize.GetHeight() + 40;
        }
    }

    return wxPoint(x, y);
}

 
void ThumbnailCanvas::OnMouseWheel(wxMouseEvent& event)
{
    //cout << "OnMouseWheel(" << event.GetWheelRotation() << ")" << endl;
    int scrollPos = GetScrollPos(wxVERTICAL) + event.GetWheelRotation();

    if (scrollPos < 0)
        scrollPos = 0;

    if (scrollPos < GetScrollRange(wxVERTICAL))
        scrollPos = GetScrollRange(wxVERTICAL);

    SetScrollPos(wxVERTICAL, scrollPos);
    event.Skip();
}


void ThumbnailCanvas::OnClose(wxCloseEvent &event)
{
    imageViewer->EnableClose();
    Destroy();
}

void ThumbnailCanvas::SetAcceleratorTable(const wxAcceleratorTable &accel)
{
    wxWindow::SetAcceleratorTable(accel);
    imageViewer->SetAcceleratorTable(accel);
}

void ThumbnailCanvas::HideImageViewer()
{
    imageViewer->Disappear();
}

void ThumbnailCanvas::SetCursor(int imageNumber)
{
    cursorP.SetTo(imageNumber);
    HandleCursorScrolling();
}

// Delete the image on disk, and delete the thumbnail pointer, but keep the thumbnail.
void ThumbnailCanvas::DeleteImage(int tn)
{
    wxRemoveFile(thumbnailPointers[tn]->GetFullPath().GetFullPath());
    
    std::deque<Thumbnail*>::iterator iter = thumbnailPointers.begin();
    
    waitingSet.RemoveSingle(tn);
    loadingSet.RemoveSingle(tn);
    redrawSetP.Clear();

    for (int i=0; i < tn; i++)
    {
        iter++;
    }
    thumbnailPointers.erase(iter);

    RecalculateRowsCols();
    redrawType = REDRAW_ALL;
    Refresh(ERASE_BACKGROUND);
}


void ThumbnailCanvas::FindNearestThumbnail()
{
    int i, n = thumbnailPointers.size();
    int bestCursor = -1;
    int bestDistance = 9999;
    int newCursorValue = -1;


    for (i = 0; i < n; i++)
    {
        if (!selectionSetP.Contains(i))
        {
            newCursorValue++;

            int distance = abs(i - cursorP.GetNumber());

            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestCursor = newCursorValue;
            }
        }
    }

    cout << "bestCursor               " << bestCursor << endl;
    cout << "thumbnailPointers.size() " << thumbnailPointers.size()  << endl;
    cout << "selectionSetP.size()     " << selectionSetP.size()  << endl;
    
    if (bestCursor >= thumbnailPointers.size() - selectionSetP.size())
    {
        cout << "decrementing " << endl;
        bestCursor--;
    }
    if (bestCursor >= 0)
    {
        cout << "returning " << bestCursor << endl;
        cursorP.SetTo(bestCursor);
    }
}


void ThumbnailCanvas::DeleteSelection()
{
    int i, n;
    
    if (!selectionSetP.size())                              // If there are no images selected, 
    {                                                       // 
        selectionSetP.AddSingle(cursorP.GetNumber());       // then just select the one the cursor is on.
    }

    n = selectionSetP.size();    
    FindNearestThumbnail();                                 // Move the cursor to the nearest unselected one

    for (i = n-1; i >= 0; i--)
    {
        int tn = selectionSetP[i];
        DeleteImage(tn);
    }

    selectionSetP.Clear();
    
    //wxRemoveFile(pathName + fn)
}
