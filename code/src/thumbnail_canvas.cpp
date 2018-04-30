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



wxBEGIN_EVENT_TABLE(ThumbnailCanvas, wxScrolledWindow)
    EVT_PAINT(ThumbnailCanvas::OnPaint)
    EVT_SIZE(ThumbnailCanvas::OnSize)
    EVT_MOUSE_EVENTS(ThumbnailCanvas::OnMouseEvent)
    EVT_KEY_DOWN(ThumbnailCanvas::OnKeyEvent)
    EVT_SET_FOCUS(ThumbnailCanvas::OnFocusEvent)
    EVT_KILL_FOCUS(ThumbnailCanvas::OnFocusKillEvent)
    EVT_IDLE(ThumbnailCanvas::OnIdle)
    EVT_MOUSEWHEEL(ThumbnailCanvas::OnMouseWheel)
    EVT_CLOSE(ThumbnailCanvas::OnClose)
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

        int w, h;
        int exitCode = ReadJpegHeader((const  char*)fileName.c_str(), &w, &h);
        cout << "Image " << w << "x" << h << endl;
        image.Create(w, h);
        cout << "a" << endl;
        image.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
        cout << "b" << endl;
        JpegRead(image.GetData());
        cout << "c" << endl;

        wxSize newSize = thumbnail.GetTnImageSize(image.GetSize(), thumbnail.tnSize);
        image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_BILINEAR);
        thumbnail.bitmap = wxBitmap(image);
        image.Destroy();
        thumbnail.imageLoaded = true;
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
  imageLoaded(false)
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

    wxRect rect(position.x         -  selectBorderSize     - 1,             // Top Left Coordinate
                position.y         -  selectBorderSize     - 1,
                tnSize.GetWidth()  + (selectBorderSize * 2 + 1),        // Size
                tnSize.GetHeight() + (selectBorderSize * 2 + 1));

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
				dc.SetBrush(wxBrush(wxS("black")));
				dc.SetPen(*wxBLACK_PEN);
			}
			else
			{
				dc.SetBrush(wxBrush(wxColor(48, 48, 48)));
				dc.SetPen(wxColor(32, 32, 32));
			}


            dc.DrawRectangle(position.x         - selectBorderSize     - 1,
                             position.y         - selectBorderSize     - 1,
                             tnSize.GetWidth()  + selectBorderSize * 2 + 1, 
                             tnSize.GetHeight() + selectBorderSize * 2 + 1);
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

            dc.DrawRectangle(position.x         - selectBorderSize     + 1,
                             position.y         - selectBorderSize     + 1,
                             tnSize.GetWidth()  + selectBorderSize * 2 - 2,
                             tnSize.GetHeight() + selectBorderSize * 2 - 2);

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
: wxScrolledWindow(parent, id, pos, size, wxSUNKEN_BORDER | wxVSCROLL | wxEXPAND | wxWANTS_CHARS | wxTAB_TRAVERSAL),
  fileNameList(),
  inFocus(true),
  cursorP(tnColumns, fileNameList),
  selectionStart(-1),
  backgroundColor(wxColor(64, 64, 64)),
  tnSize(200, 200),
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
    imageViewer = new ImageViewer(this, -1, _T("Image Viewer"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxFRAME_TOOL_WINDOW | wxCAPTION);
    imageViewer->SetFileNameList(&fileNameList);
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
    size_t n = thumbnails.size();
    size_t x, y;

    xSize -= tnSpacingX;

    tnColumns = xSize / (tnSize.GetWidth() + tnSpacingX);
    if (tnColumns < 1)                                                      // Can't have zero columns!
    {
        tnColumns = 1;
    }
    tnRows    = (thumbnails.size() / tnColumns) + 1;

    x = 0;
    y = 0;
    for (i = 0; i < n; i++)
    {
        int xPos = x * (tnSize.GetWidth()  + tnSpacingX) + tnSpacingX;
        int yPos = y * (tnSize.GetHeight() + tnSpacingY) + tnSpacingY;

        wxPoint pos(xPos, yPos);
        thumbnails[i].SetPosition(pos);

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


    switch (redrawType)
    {
        case REDRAW_ALL:
            //cout << "Redrawing ALL" << endl;
            for (int i = 0; i < n; i++)
            {
                selected = selectionSetP.Contains(i);
                if (cursorP.GetNumber() == i)
                {
                    onCursor = true;
                    cursorFileName = thumbnailPointers[i]->GetFullPath().GetFullName();
                    cursorImageSize = thumbnailPointers[i]->GetImageSize();
                    //STATUS_TEXT(0, "%s  (%d, %d)", cursorFileName, cursorImageSize.GetWidth(), cursorImageSize.GetHeight());
                }
                else
                {
                    onCursor = false;
                }

                thumbnailPointers[i]->Draw(dc, selected, onCursor);
            }
            break;

        case REDRAW_SELECTION:
            //redrawSetP.Print();

            for (int i = 0; i<redrawSetP.size(); i++)
            {
				int th = redrawSetP[i];
				if (i >= 0)
					thumbnailPointers[th]->Erase(dc);
            }

            for (int i=0; i<redrawSetP.size(); i++)
            {
                int th = redrawSetP[i];
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

                //cout << "  draw " << th << " " << selected << " " << onCursor << " " << cursor.GetNumber() << endl;

                thumbnailPointers[th]->Draw(dc, selected, onCursor, inFocus);
            }

            redrawSetP.Clear();
            break;
    }
    
    redrawType = REDRAW_ALL;
}


void ThumbnailCanvas::OnKeyEvent(wxKeyEvent &event)
{
    //cout << "OnKeyEvent(" << event.GetKeyCode() << ")" << endl;
    int cursorToErase = -1;

    switch (event.GetKeyCode())
    {
        case WXK_UP:     cursorP.Move( 0, -1);							break;
        case WXK_DOWN:   cursorP.Move( 0,  1);							break;
        case WXK_LEFT:   cursorP.Move(-1,  0);							break;
        case WXK_RIGHT:  cursorP.Move( 1,  0);							break;
        case WXK_RETURN: imageViewer->DisplayImage(cursorP.GetNumber()); break;

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

	redrawSetP.Print();

	redrawType = REDRAW_SELECTION;
    Refresh(DONT_ERASE_BACKGROUND);
}

void ThumbnailCanvas::OnFocusEvent(wxFocusEvent &event)
{
    //cout << "ThumbnailCanvas::OnFocusEvent" << endl;
	inFocus = true;

    if ((cursorP.GetNumber() > -1) && (cursorP.GetNumber() < thumbnails.size()))
    {
        //cursorNumberthumbnails[cursorNumber].Select();
        cursorP.SetupRedraw(redrawSetP);
        redrawType = REDRAW_SELECTION;
        Refresh(DONT_ERASE_BACKGROUND);
    }
    else
    {
		cursorP.SetTo(0);
    }
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
    }
}


void ThumbnailCanvas::CreateAntiAliasedBitmap()
{

}


void ThumbnailCanvas::ClearThumbnails()
{
    thumbnails.clear();
    thumbnailPointers.clear();
}

void ThumbnailCanvas::LoadThumbnails(wxString directory)
{
	//cout << "LoadThumbnails(" << directory << ")" << endl;

    ClearThumbnails();
    fileNameList.LoadFileList(directory);

    int n = fileNameList.files.size();
    thumbnails.reserve(n);
    thumbnailPointers.reserve(n);
    for (int i = 0; i<n; i++)
    {
        wxString fullPath = directory;
        fullPath += wxT("\\");
        fullPath += fileNameList.files[i].fileName.GetFullPath();

        thumbnails.emplace_back(wxPoint(0,0), fullPath);
    }

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

    RecalculateRowsCols();
	//cout << "LoadThumbnails(" << directory << ")  done" << endl;
}

bool ThumbnailCanvas::HandleThumbnailLoading()
{
    int i, n = loadingSet.size();

    //cout << "Checking" << endl;
    for (i = 0; i < n; i++)
    {
        int th = loadingSet[i];
        //cout << "  loadingSet[" << i << "] = " << th << endl;
        if (thumbnails[th].ImageIsLoaded())
        {
            //cout << "    removing" << endl;
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
        return false;
    else
        return true;                                // Means we've finished loading
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
    if (event.LeftDown())
    {
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

                redrawType = REDRAW_SELECTION;
                Refresh(DONT_ERASE_BACKGROUND);
            }

            dragState = TNC_DRAG_STATE_CLICKED;
            clickPoint = event.GetPosition();
        }
        return;
    }

    if (event.LeftDClick())
    {
        int th = GetThumbnailFromPosition(logicalPosition);
        imageViewer->DisplayImage(th);
    }

    //cout << "Done" << endl;
    //Update();
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
