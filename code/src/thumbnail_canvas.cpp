#include "thumbnail_canvas.h"
#include "imageviewer.h"

#include "wx/thread.h"
#include "wx/image.h"
#include "wx/imaggif.h"
#include "wx/dcmemory.h"
#include "wx/dcclient.h"
#include "wx/dataobj.h"
#include "wx/dir.h"
#include "wx/log.h"
#include "wx/tokenzr.h"
#include "status_bar.h"
#include "config_parser.h"
#include "vector_renderer.h"
#include "imagebrowser.h"
#include "wx/generic/dragimgg.h"
#include <wx/dnd.h>
#include "image_file_handler_registry.h"
#include "image_file_handler.h"
#include <wx/dataobj.h>
#define wxDragImage wxGenericDragImage

#include <iostream>
using namespace std;



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
    thumbnail.isLoading = true;
    //image.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    wxFileName fn(fileName);
    wxString   extension = fn.GetExt().Upper();

    ImageFileHandler *imageFileHandler = ImageFileHandlerRegistry::instance().GetImageFileHandlerFromExtension(extension);
    if (imageFileHandler)
        imageFileHandler->LoadThumbnail(fn.GetFullPath(), thumbnail);
    else
        thumbnail.imageLoaded = true;

    delete imageFileHandler;

    return (wxThread::ExitCode)0;
}

/*
wxThread::ExitCode ThumbnailLoader::Entry()
{
    wxLogNull logNo;													// ... instead logging is suspended while this object is in scope
    wxImage   image;


    thumbnail.isLoading = true;
    image.SetOption(wxIMAGE_OPTION_GIF_TRANSPARENCY, wxIMAGE_OPTION_GIF_TRANSPARENCY_UNCHANGED);
    wxFileName fn(fileName);
    wxString   extension = fn.GetExt().Upper();

    if ((extension == "JPG") ||
        (extension == "JPEG"))
    {
        cout << "Using JpegTurbo to load thumbnail " << fileName << endl;

		//jpeg_load_state *load_state = ReadJpegHeader((const  char*)fileName.c_str());
		int success = ReadJpegHeader(&jpegLoadState, (const  char*)fileName.c_str());
		jpeg_load_state *load_state = &jpegLoadState;

		int w = load_state->width, h = load_state->height;
	
		if (success)
        {
            image.Create(w, h);
            image.SetRGB(wxRect(0, 0, w, h), 128, 64, 0);
            JpegRead(image.GetData(), load_state);
        }
		else
		{
			image.Create(32, 32);
			unsigned char *data = image.GetData();
			for (int y = 0; y < 32; y++)
				for (int x = 0; x < 32; x++)
				{
					*data++ = x ^ y;
					*data++ = (x * 2) ^ (y * 2);
					*data++ = (x * 3) ^ (y * 3);
				}
		}

		thumbnail.imageSize = image.GetSize();
		wxSize newSize = thumbnail.GetTnImageSize(image.GetSize(), thumbnail.tnSize);
		image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_BILINEAR);
		thumbnail.bitmap = wxBitmap(image);
		image.Destroy();
		thumbnail.imageLoaded = true;
	}
    else if (extension == "PNG")
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
            //cout << "Failed to load " << fileName << endl;
        }
    }
    else if (extension == "WMV")
    {
        LONGLONG pos = 30 * 10000000;
        VideoThumbnailReader videoThumbnailReader;
        videoThumbnailReader.OpenFile(fileName.wchar_str());
        videoThumbnailReader.CreateBitmap((char*)image.GetData(), pos);
    }

    thumbnail.isLoading = false;
    return 0;
}
*/




wxThread::ExitCode ThumbnailHeaderReader::Entry()
{
    int i, n = thumbnailCanvas.GetNumThumbnails();

    for (i = 0; i < n; i++)
    {
        Thumbnail *tn = thumbnailCanvas.GetThumbnail(i);
        wxString path = tn->GetFullPath().GetFullPath();
        tn->FetchHeader();
        if ((i&7) == 0)
            thumbnailCanvas.ReadHeadersCompleted();
    }

    thumbnailCanvas.ReadHeadersCompleted();

    return 0;
}



Thumbnail::Thumbnail(const wxPoint &pos, wxFileName path, bool fetchHeader)
: fullPath(path),
  position(pos),
  isLoading(false),
  imageLoaded(false),
  hasBeenDrawn(false),
  imageSize(0, 0),
  thumbnailLoader(0)
{
    thumbnailLoader = new ThumbnailLoader(path.GetFullPath(), *this);

    if (fetchHeader)
        FetchHeader();
}

void Thumbnail::FetchHeader()
{
    if ((fullPath.GetExt().Upper() == "JPG") ||
        (fullPath.GetExt().Upper() == "JPEG"))
    {
		jpeg_load_state load_state;
		int success = ReadJpegHeaderOnly(&load_state, (const  char*)fullPath.GetFullPath().c_str());
		
		int w = load_state.width, h = load_state.height;
		if (success)
        {
            imageSizeTemp = GetTnImageSize(wxSize(w, h));
        }
    }
}


Thumbnail::~Thumbnail()
{
    //cout << "~Thumbnail(" << this << ")  " << endl;
    if (!imageLoaded)
    {
        //cout << "  image loaded" << endl;
        if (isLoading)
        {
            //cout << "  loader running" << endl;
            thumbnailLoader->Delete();
            //cout << "  killed" << endl;
        }
    }
    //cout << "  Destructing done" << endl << endl;
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

    wxRect rect(position.x         - (selectBorderSize + 3),             // Top Left Coordinate
                position.y         - (selectBorderSize + 3),
                tnSize.GetWidth()  + (selectBorderSize + 3) * 2,        // Size
                tnSize.GetHeight() + (selectBorderSize + 3) * 2);

    dc.DrawRectangle(rect);
}



void Thumbnail::Draw(wxPaintDC &dc, bool selected, bool cursor, bool inFocus)
{
    //cout << "Thumbnail::Draw(" << this << ")" << endl;
    wxRect textRectangle(position.x, position.y + tnSize.GetHeight()+5, tnSize.GetWidth(), 20);
	wxPen pen;

    if (cursor)
    {
		if (inFocus)
		{
            //cout << "InFocus" << endl;
			dc.SetBrush(wxBrush(wxS("blue")));
			dc.SetPen(*wxBLACK_PEN);
		}
		else
		{
            //cout << "Out Focus" << endl;
            dc.SetBrush(wxBrush(wxColor(48, 48, 48)));
			dc.SetPen(wxColor(32, 32, 32));
		}


        dc.DrawRectangle(position.x         - (selectBorderSize + 0),
                         position.y         - (selectBorderSize + 0),
                         tnSize.GetWidth()  + (selectBorderSize + 0) * 2, 
                         tnSize.GetHeight() + (selectBorderSize + 0) * 2);

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

        dc.DrawRectangle(position.x         - (selectBorderSize + 3),
                         position.y         - (selectBorderSize + 3),
                         tnSize.GetWidth()  + (selectBorderSize + 3) * 2,
                         tnSize.GetHeight() + (selectBorderSize + 3) * 2);

        dc.SetTextForeground(wxColor(255, 255, 255));
    }
    else
    {
        dc.SetTextForeground(wxColor(128, 128, 128));
    }
    
    //cout << "imageLoaded   = " << imageLoaded << endl;
    //cout << "bitmap.IsOk() = " << bitmap.IsOk() << endl;

    if (imageLoaded && bitmap.IsOk())
    {
        int x = position.x + (tnSize.GetWidth()  - bitmap.GetWidth() ) / 2;
        int y = position.y + (tnSize.GetHeight() - bitmap.GetHeight()) / 2;

        dc.DrawBitmap(bitmap, x, y, false);
    }
    else
    {
        dc.SetBrush(wxBrush(wxColor(32,32,32)));
        dc.SetPen(*wxBLACK_PEN);

        //cout << "tn: " << imageSizeTemp.x << ", " << imageSizeTemp.y << endl;
        if (imageSizeTemp.x*imageSizeTemp.y > 0)
        {
            //cout << "Drawing sized blank" << endl;
            int cx = position.x + (tnSize.x >> 1);
            int cy = position.y + (tnSize.y >> 1);

            dc.DrawRectangle(cx - (imageSizeTemp.x >> 1),
                             cy - (imageSizeTemp.y >> 1),
                             imageSizeTemp.x,
                             imageSizeTemp.y);
        }
        else
        {
            dc.DrawRectangle(position.x,
                             position.y + 3,
                             tnSize.GetWidth() - 6,
                             tnSize.GetHeight() - 6);
        }
    }
	wxString s = fullPath.GetFullName();
    DrawLabelClipped(dc, s, textRectangle);
}


bool Thumbnail::IsMouseInside(const wxPoint &mousePos)
{
    wxRect rect(position, tnSize);

    return rect.Contains(mousePos);
}


void Thumbnail::CreateGenericIcon()
{

}

void Thumbnail::SetImage(wxImage &image)
{
    imageSize = image.GetSize();
    wxSize newSize = GetTnImageSize(image.GetSize());
    image.Rescale(newSize.GetWidth(), newSize.GetHeight(), wxIMAGE_QUALITY_BILINEAR);
    bitmap = wxBitmap(image);
}





ThumbnailCanvas::ThumbnailCanvas(ImageBrowser *imgBrs, wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
: wxScrolledWindow(parent, id, pos, size, wxSUNKEN_BORDER | wxVSCROLL | wxEXPAND | wxWANTS_CHARS),
  fileNameList(),
  inFocus(false),
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
  draggingSet(256),
  maxLoading(1),
  imageViewer(0),
  imageBrowser(imgBrs)
{
    SetBackgroundColour(backgroundColor);
    ImageFileHandlerRegistry& imageFileHandlerRegistry = ImageFileHandlerRegistry::instance();
    wxArrayString &filtersList = imageFileHandlerRegistry.GetFiltersList();

    int i, n = filtersList.size();

    for (i = 0; i < n; i++)
    {
        fileNameList.AddFilter(filtersList[i].Lower());
    }

    Thumbnail::SetSize(tnSize);
    Thumbnail::SetSelectBorder(3);
    Thumbnail::SetLabelHeight(26);
    Thumbnail::SetBackgroundColor(backgroundColor);

    RecalculateRowsCols();
    
    
    DragAcceptFiles(true);
    Bind(wxEVT_DROP_FILES, &ThumbnailCanvas::OnDropFiles, this, -1);
}

void ThumbnailCanvas::SetImageViewer(ImageViewer *iv)
{
    imageViewer = iv;
    imageViewer->SetFileNameList(&fileNameList);
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
        //   busyInfo(_("Adding files, wait please..."));

        
        wxArrayString files;

        for (int i = 0; i < event.GetNumberOfFiles(); i++)
        {
            wxString name = dropped[i];
            wxFileName fileName = name;

            wxString destination = fileNameList.directory.GetName() + wxT("\\") + fileName.GetFullName();
            cout << "  copy " << name << " to " << destination << endl;
            wxCopyFile(name, destination);

            thumbnailIndex.push_back(thumbnails.size());
            waitingSet.AddSingle(thumbnails.size());
            thumbnails.emplace_back(wxPoint(0, 0), destination);
            fileNameList.AddFileToList(destination);
        }
        fileNameList.Resort();
        RecalculateRowsCols();
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
    size_t n = thumbnailIndex.size();
    size_t x, y;

    int tnJumpX = tnSize.GetWidth()  + tnSpacingX;                          // The distance from the corner of one ThumbNail
    int tnJumpY = tnSize.GetHeight() + tnSpacingY;                          // to the corner of the next

    xSize -= tnSpacingX;                                                    // Because there'll be a border at the left edge

    tnColumns = xSize / tnJumpX;                                            // Calculate the number of columns
    if (tnColumns < 1)                                                      // Can't have zero columns!
    {
        tnColumns = 1;
    }
    tnRows = (int)ceil(((double)thumbnailIndex.size() / tnColumns));        // Calculate the number of rows

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
        thumbnails[thumbnailIndex[i]].SetPosition(pos);

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
    int n = thumbnailIndex.size();
    bool selected;
    bool onCursor;
    wxPaintDC dc(this);
    PrepareDC(dc);
    wxString cursorFileName;
    wxSize   cursorImageSize;
    
    //cout << "ThumbnailCanvas::OnPaint" << endl;
    if (!n)
        return;

    //cout << "Redraw type " << redrawType << endl;
    switch (redrawType)
    {
        case REDRAW_ALL:
            //cout << "Redrawing ALL" << endl;
            for (int i = 0; i < n; i++)
            {
                //cout << "  " << i << endl;
                selected = selectionSetP.Contains(i);
                if (cursorP.GetNumber() == i)
                {
                    onCursor = true;
                    cursorFileName  = thumbnails[thumbnailIndex[i]].GetFullPath().GetFullName();
                    cursorImageSize = thumbnails[thumbnailIndex[i]].GetImageSize();
                    //STATUS_TEXT(0, "%s  (%d, %d)", cursorFileName, cursorImageSize.GetWidth(), cursorImageSize.GetHeight());
                }
                else
                {
                    onCursor = false;
                }
                //cout << "    Drawing" << endl;
                thumbnails[thumbnailIndex[i]].Draw(dc, selected, onCursor);
            }
            break;

        case REDRAW_SELECTION:
            //cout << "Redrawing Selection" << endl;
            //cout << "  redrawSetP.size() = " << redrawSetP.size() << endl;
            //redrawSetP.Print();

            for (int i = 0; i<redrawSetP.size(); i++)
            {

				int th = redrawSetP[i];
                //cout << "  th = " << th << endl;
                if (th >= 0)
                {
                    //cout << "    Erasing" << endl;
                    thumbnails[thumbnailIndex[th]].Erase(dc);
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
                    cursorFileName  = thumbnails[thumbnailIndex[th]].GetFullPath().GetFullName();
                    cursorImageSize = thumbnails[thumbnailIndex[th]].GetImageSize();
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
                    thumbnails[thumbnailIndex[th]].Draw(dc, selected, onCursor, inFocus);
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

    if ( (cursorNumber == -1) || (thumbnailIndex.size() == 0) || (cursorNumber >= thumbnails.size()) )
    {
        Scroll(0, 0);
        return;
    }

    int xPPU, yPPU;
    GetScrollPixelsPerUnit(&xPPU, &yPPU);

    Thumbnail *tn = &(thumbnails[thumbnailIndex[cursorNumber]]);
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
    bool skipping = true;
    int  pageJump = GetNumRows();

    if (HandleAsNavigationKey(event))
        return;

    //cout << "OnKeyEvent(" << event.GetKeyCode() << ")" << endl;
    int cursorToErase = -1;

    if (event.GetKeyCode() == 'D')
    {
        //cout << "ThumbnailCanvas(D)" << endl;
    }

    switch (event.GetKeyCode())
    {
        case WXK_UP:        cursorP.Move( 0,         -1); skipping = false;         break;
        case WXK_DOWN:      cursorP.Move( 0,          1); skipping = false;         break;
        case WXK_LEFT:      cursorP.Move(-1,          0); skipping = false;         break;
        case WXK_RIGHT:     cursorP.Move( 1,          0); skipping = false;         break;
        case WXK_PAGEUP:    cursorP.Move( 0,  -pageJump); skipping = false;         break;
        case WXK_PAGEDOWN:  cursorP.Move( 0,   pageJump); skipping = false;         break;

        case WXK_RETURN:    ActivateThumbnail(cursorP.GetNumber());                 return;
        case WXK_DELETE:    DeleteSelection();                                      return;

        default:
            event.Skip(skipping);
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

	//selectionSetP.Print();
	redrawSetP.AddFrom(selectionSetP);
	//redrawSetP.Print();

    //cout << "Redraw Selection" << endl;
    redrawType = REDRAW_SELECTION;
    Refresh(DONT_ERASE_BACKGROUND);
    HandleCursorScrolling();
    UpdateStatusBar_File();

    event.Skip(skipping);
}

void ThumbnailCanvas::OnFocusEvent(wxFocusEvent &event)
{
    //cout << "ThumbnailCanvas::OnFocusEvent" << endl;
	inFocus = true;

    if ((cursorP.GetNumber() > -1) && (cursorP.GetNumber() < thumbnailIndex.size()))
    {
        //cursorNumberthumbnails[cursorNumber].Select();
        cursorP.SetupRedraw(redrawSetP);
        //cout << "Redraw Selection Focus" << endl;
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

    if ((cursorP.GetNumber() > -1) && (cursorP.GetNumber() < thumbnailIndex.size()))
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
    thumbnailIndex.clear();
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

        long long scale = 1;
        long long si = 0;
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
    //cout << "ThumbnailCanvas::DirectoryWasDeleted(" << path << ")" << endl;

    if (path == fileNameList.directory.GetName())
    {
        //cout << "This directory was deleted" << endl;

        ClearThumbnails();

        selectionSetP.Clear();
        redrawSetP.Clear();
        waitingSet.Clear();
        loadingSet.Clear();
    }
}

// Clear the canvas and delete all thumbnails if we are in this directory
void ThumbnailCanvas::UnLoadThumbnails(wxString directory)
{
    cout << "ThumbnailCanvas::UnLoadThumbnails(" << directory << ")" << endl;
    cout << "  " << fileNameList.directory.GetName() << endl;

    if ( (                        directory.StartsWith(fileNameList.directory.GetNameWithSep()) ) ||
         ( fileNameList.directory.GetName().StartsWith(directory)        )
       )
    {
        cout << "  Unloading" << endl;
        ClearThumbnails();
        imageViewer->ClearCache();
        Scroll(0, 0);
    }
    cout << "  Done" << endl;
}

void ThumbnailCanvas::LoadThumbnails(wxString directory)
{
	cout << "LoadThumbnails(" << directory << ")" << endl;
    inFocus = false;
    ClearThumbnails();
    imageViewer->ClearCache();
    fileNameList.LoadFileList(directory);

    int n = fileNameList.files.size();
    thumbnails.reserve(n);
    thumbnailIndex.reserve(n);
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

        thumbnailIndex.push_back(thumbnails.size());
        thumbnails.emplace_back(wxPoint(0,0), fullPath);
        //cout << "  " << thumbnailIndex.back() << " at " << &thumbnails.back() << endl;
    }

    readHeadersCompleted = true;
    //readHeadersCompleted = false;
    //ThumbnailHeaderReader *thr = new ThumbnailHeaderReader(*this);
    //thr->Run();

    wxString hrs = HumanFileSize(totalDirectorySizeBytes.GetLo());
    STATUS_TEXT(STATUS_BAR_DIRECTORY_SUMMARY, "%d files (%s)", n, hrs.c_str());
    
    //for (int i = 0; i < n; i++)
    //{
    //    thumbnailPointers.push_back( &thumbnails[i] );
    //}

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
	cout << "LoadThumbnails(" << directory << ")  done" << endl;
}

void ThumbnailCanvas::StopLoadingThumbnails(wxString directory)
{
    if ((directory.StartsWith(fileNameList.directory.GetNameWithSep())) ||
        (fileNameList.directory.GetName().StartsWith(directory))
        )
    {
        waitingSet.Clear();
    }
}

bool ThumbnailCanvas::HandleThumbnailLoading()
{
    int i, n = loadingSet.size();

    //cout << "ThumbnailCanvas::HandleThumbnailLoading()" << endl;

    for (i = 0; i < n; i++)
    {
        int th = loadingSet[i];
        //cout << "  loadingSet[" << i << "] = " << th << endl;
        if (thumbnails[thumbnailIndex[th]].ImageIsLoaded())
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
            //cout << "Starting " << th << " loading " << &thumbnails[thumbnailIndex[th]] << endl;
            thumbnails[thumbnailIndex[th]].StartLoadingImage();
            loadingSet.AddSingle(th);
            waitingSet.RemoveSingle(th);
            i--;
            n--;
        }
        //cout << "  * starting " << th << endl;
    }

    if (readHeadersCompleted)
    {
        redrawType = REDRAW_ALL;
        Refresh(ERASE_BACKGROUND);
        readHeadersCompleted = false;
    }
    else if (redrawSetP.size())
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
    size_t n = thumbnailIndex.size();

    for (size_t i = 0; i<n; i++)
    {
        if (thumbnails[thumbnailIndex[i]].IsMouseInside(position))
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

void SetDebuggingText(wxString text);

void ThumbnailCanvas::OnMouseEvent(wxMouseEvent &event)
{
    wxPoint physicalPosition = event.GetPosition();
    wxPoint  logicalPosition = CalcUnscrolledPosition(physicalPosition);
    int th = GetThumbnailFromPosition(logicalPosition);

    wxString lines, s;

    if (event.LeftDClick())
    {
        SetFocus();
        if (th > -1)
        {
            ActivateThumbnail(cursorP.GetNumber());
            redrawType = REDRAW_ALL;
        }
        else        { /* failure! */ }
    }


    if (event.LeftDown() && event.ShiftDown())
    {
        lines.Printf(wxT("event.LeftDown() && event.ShiftDown()    %d\n"), th);
        SetFocus();

        cursorP.SetTo(th);
        cursorP.SetupRedraw(redrawSetP);
        redrawSetP.AddFrom(selectionSetP);      // Need to erase any previously selected thumbs
        selectionSetP.SetRange(selectionStart, th);
        s.Printf("selectionSetP.SetRange(%d, %d)\n", selectionStart, th);
        lines += s;
        lines += wxT("selectionSetP = ");                   lines += selectionSetP.SPrint();
        cout << "selectionSetP = ";
        selectionSetP.Print();
        dragState = TNC_DRAG_STATE_NONE;

        UpdateStatusBar_File();
        Refresh(DONT_ERASE_BACKGROUND);
    }
    else if (event.LeftDown() && event.ControlDown())
    {
        lines.Printf(wxT("event.LeftDown() && event.ShiftDown()    %d\n"), th);
        SetFocus();

        cursorP.SetTo(th);
        cursorP.SetupRedraw(redrawSetP);
        redrawSetP.AddFrom(selectionSetP);      // Need to erase any previously selected thumbs
        selectionSetP.ToggleSingle(th);
        s.Printf("selectionSetP.AddSingle(%d)\n", th);
        lines += s;
        lines += wxT("selectionSetP = ");                   lines += selectionSetP.SPrint();
        cout << "selectionSetP = ";
        selectionSetP.Print();
        dragState = TNC_DRAG_STATE_NONE;

        UpdateStatusBar_File();
        Refresh(DONT_ERASE_BACKGROUND);
    }
    else if (event.LeftDown())
    {
        SetFocus();

        if (th > -1)
        {
            lines.Printf(wxT("event.LeftDown()    %d\n"), th);

            if (!selectionSetP.Contains(th))                // Clicking on an unselected thumb
            {
                cursorP.SetTo(th);
                cursorP.SetupRedraw(redrawSetP);
                redrawSetP.AddFrom(selectionSetP);
                selectionSetP.Clear();
                selectionSetP.AddSingle(th);
                lines += wxT("selectionSetP.Clear()\n");
                selectionStart = th;
            }
            else                                            // Clicking on a SELECTED thumb
            {
                lines += wxT("selectionSetP.Contains(th)\n");
            }

            UpdateStatusBar_File();
            clickPoint = event.GetPosition();

            dragState  = TNC_DRAG_STATE_CLICKED;
            redrawType = REDRAW_SELECTION;
            Refresh(DONT_ERASE_BACKGROUND);

            s.Printf(wxT("selectionStart = %d\n"), selectionStart);  lines += s;
        }
    }

    if (event.Dragging())
    {
        cout << "Dragging! " << dragState << endl;
    }


    if (event.Dragging() && dragState != TNC_DRAG_STATE_NONE)
    {
        cout << "Drag 1" << endl;
        lines.Printf(wxT("event.Dragging() && dragState != TNC_DRAG_STATE_NONE    %d\n"), th);
        if (dragState == TNC_DRAG_STATE_CLICKED)
        {
            cout << "TNC_DRAG_STATE_CLICKED" << endl;
            // We will start dragging if we've moved beyond a couple of pixels

            int tolerance = 2;
            int dx = abs(event.GetPosition().x - clickPoint.x);
            int dy = abs(event.GetPosition().y - clickPoint.y);

            if (dx <= tolerance && dy <= tolerance)
                return;

            // Start the drag.
            dragState = TNC_DRAG_STATE_DRAGGING;
            if (selectionSetP.Contains(th))
            {
                draggingSet.CopyFrom(selectionSetP);
                lines += wxT("copied from selectionSetP\n");
            }
            else
            {
                draggingSet.Clear();
                draggingSet.AddSingle(th);
                lines += wxT("set to single\n");
            }
            cout << "Dragging set:";
            draggingSet.Print();
            cout << "Adding files:" << endl;

            int i, n = draggingSet.size();
            for (i = 0; i < n; i++)
            {
            }
            wxWindow::SetCursor(wxCursor(wxCURSOR_HAND));

            //wxPoint beginDragHotSpot =clickPoint - m_draggedShape->GetPosition();
        }
        else if (dragState == TNC_DRAG_STATE_DRAGGING)
        {
            cout << "TNC_DRAG_STATE_DRAGGING" << endl;

            dragingFilesDataObject = new wxFileDataObject();
            
            for (int i = 0; i < draggingSet.size(); i++)
            {
                int fNum = draggingSet[i];
                cout << "  " << fileNameList[fNum] << endl;
                dragingFilesDataObject->AddFile(fileNameList[fNum]);
            }

            wxDropSource dragSource(this);
            dragSource.SetData(*dragingFilesDataObject);
            wxDragResult result = dragSource.DoDragDrop(true);                  // Dragging starts now!
                                                                                // ====================
                                                                                // Items have been dropped!
            wxFileName fileName;
            for (int i = 0; i < draggingSet.size(); i++)                        // Check to see which files have disappeared, and remove them from the canvas.
            {
                int fNum = draggingSet[i];
                fileName = fileNameList[fNum];
                //cout << fileName.GetFullPath() << " " << fileName.Exists() << endl;

                if (!fileName.Exists())
                {
                    //cout << "  " << fileName.GetFullPath() << " was moved" << endl;
                    RemoveThumbNailFromCanvas(fNum);
                }
                else
                {
                    //cout << "  " << fileName.GetFullPath() << " still exists" << endl;
                }
            }

            //cout << "wxDragResult = " << result << endl;
            //if (result == wxDragNone)     cout << "wxDragNone" << endl;
            //if (result == wxDragCopy)     cout << "wxDragCopy" << endl;
            //if (result == wxDragMove)     cout << "wxDragMove" << endl;
            //if (result == wxDragLink)     cout << "wxDragLink" << endl;
            //if (result == wxDragCancel)   cout << "wxDragCancel" << endl;
        }
    }

    //lines += wxT("selectionSetP = ");                   lines += selectionSetP.SPrint();
    //lines += wxT("draggingSet   = ");                   lines += draggingSet.SPrint();
    //s.Printf(wxT("dragState     = %d\n"), dragState);   lines += s;

    //SetDebuggingText(lines);
    //cout << "Done" << endl;
    //Update();
}

void ThumbnailCanvas::ActivateThumbnail(int n)
{
    cout << "ThumbnailCanvas::ActivateThumbnail(" << n << ")" << endl;
    cout << "  " << fileNameList.directory.GetName() << endl << endl;

    if ( (n < 0) || (n > fileNameList.MaxFileNumber()) )
        return;

    wxFileName        path = fileNameList[n];
    wxString          extension = path.GetExt();
    ImageFileHandler* imageFileHandler = ImageFileHandlerRegistry::instance().GetImageFileHandlerFromExtension(extension);

    int actions = imageFileHandler->LoadImage(path.GetFullPath());

    if (actions & LOAD_IMAGE)
    {
        //imageViewer->DisplayImage(cursorP.GetNumber());
        imageViewer->DisplayImage(n);
    }

    if (actions & DELETE_FILE)
    {
        DeleteSelection();
    }

    if (actions & REFRESH_TREE)
    {
        cout << "  refreshing tree: " << path.GetPathWithSep() << endl;
        imageBrowser->RefreshDirTree(path.GetPathWithSep());
    }

    delete imageFileHandler;
}


void ThumbnailCanvas::ClearStatusBar()
{
    STATUS_TEXT(STATUS_BAR_DIRECTORY_SUMMARY, "");
    STATUS_TEXT(STATUS_BAR_FILE_SIZES,        "");
    STATUS_TEXT(STATUS_BAR_FILE_FORMAT,       "");
    STATUS_TEXT(STATUS_BAR_INFORMATION,       "");
}

void ThumbnailCanvas::UpdateStatusBar_File()
{
    if (selectionSetP.size() == 0)
    {
        if (thumbnailIndex.size() > 0)
        {
            int index = thumbnailIndex[cursorP.GetNumber()];
            Thumbnail *tn = &thumbnails[index];

            //cout << "Update Status: " << tn->GetFullPath().GetFullPath() << endl;

            wxFileName fn = tn->GetFullPath();
            wxDateTime date = fn.GetModificationTime();
            wxString   dateString = date.Format("%d/%m/%Y  %H:%M");
            wxSize     imageSize = tn->GetImageSize();

            STATUS_TEXT(STATUS_BAR_FILE_SIZES, "%s,  %s - (%d x %d)", fn.GetHumanReadableSize().c_str(), dateString, imageSize.GetWidth(), imageSize.GetHeight());
            STATUS_TEXT(STATUS_BAR_FILE_FORMAT, fn.GetFullName().c_str());
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
    //imageViewer->EnableClose();
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

void ThumbnailCanvas::RemoveThumbNailFromCanvas(int tn)
{
    //int tn = fileNameList.GetFileNumber(fileName);

    std::vector<int>::iterator iter = thumbnailIndex.begin();

    waitingSet.RemoveSingle(tn);
    loadingSet.RemoveSingle(tn);
    redrawSetP.Clear();

    for (int i = 0; i < tn; i++)
    {
        iter++;
    }
    thumbnailIndex.erase(iter);
    //cout << "Num thumbnails " << thumbnailIndex.size() << endl;

    RecalculateRowsCols();
    redrawType = REDRAW_ALL;
    Refresh(ERASE_BACKGROUND);
}

// Delete the image on disk, and delete the thumbnail pointer, but keep the thumbnail.
void ThumbnailCanvas::DeleteImage(int tn)
{
    //int tn = fileNameList.GetFileNumber(fileName);

    fileNameList.DeleteFileNumber(tn);
    RemoveThumbNailFromCanvas(tn);
}


void ThumbnailCanvas::FindNearestThumbnail()
{
    int i, n = thumbnailIndex.size();
    int bestCursor = -1;
    int bestDistance = 9999;
    int newCursorValue = n - selectionSetP.size();


    for (i = n-1; i>=0; i--)
    {
        if (!selectionSetP.Contains(i))
        {
            newCursorValue--;

            //int distance = abs(i - cursorP.GetNumber());
            int distance = selectionSetP.DistanceTo(i);

            if (distance < bestDistance)
            {
                bestDistance = distance;
                bestCursor = newCursorValue;
            }
        }
    }


    //cout << "bestCursor               " << bestCursor << endl;
    //cout << "thumbnailPointers.size() " << thumbnailIndex.size()  << endl;
    //cout << "selectionSetP.size()     " << selectionSetP.size()  << endl;
    
    if (bestCursor >= thumbnailIndex.size() - selectionSetP.size())
    {
        //cout << "decrementing " << endl;
        bestCursor--;
    }
    if (bestCursor >= 0)
    {
        //cout << "returning " << bestCursor << endl;
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
