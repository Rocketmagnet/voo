#include "thumbnail_canvas.h"
#include "imageviewer.h"

#include "natural_sort.h"
#include "wx/arrstr.h"
#include "wx/sizer.h"
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
#include "wx/menu.h"
#include "mathhelpers.h"

#define wxDragImage wxGenericDragImage

#include <iostream>
using namespace std;

extern void NoteTime(wxString s);


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
    //EVT_CONTEXT_MENU( ThumbnailCanvas::OnContextMenu)
    wxEND_EVENT_TABLE()
   
wxSize      Thumbnail::tnSize(100,100);
wxArrayInt  Thumbnail::arrayIntStatic;
int         Thumbnail::selectBorderSize;
int         Thumbnail::labelHeight;
wxColor     Thumbnail::backgroundColor;


class ThumbnailSortingFunctor
{
public:
    static const int SORT_NAME_FORWARDS = 1;
    static const int SORT_NUMS_FORWARDS = 2;
    static const int SORT_DATE_FORWARDS = 3;

    ThumbnailSortingFunctor(int st, const std::vector<Thumbnail>& tn)
        : sortType(st),
        thumbnails(tn)
    {
    }

    bool operator()(int a, int b)
    {
        if (sortType == SORT_NAME_FORWARDS)
        {
            //std::cout << a << "<" << b << std::endl;
            //std::cout << thumbnails[a].GetFileName().GetName() << std::endl;
            //std::cout << thumbnails[b].GetFileName().GetName() << std::endl;
            //return thumbnails[a].GetFileName().GetName() < thumbnails[b].GetFileName().GetName();
            return thumbnails[a].GetFileName().GetName().CmpNoCase(thumbnails[b].GetFileName().GetName()) < 0;
        }

        if (sortType == SORT_NUMS_FORWARDS)
        {
            //std::cout << a << "<" << b << std::endl;
            //std::cout << thumbnails[a].GetFileName().GetName() << std::endl;
            //std::cout << thumbnails[b].GetFileName().GetName() << std::endl;
            //return thumbnails[a].GetFileName().GetName() < thumbnails[b].GetFileName().GetName();
            //return thumbnails[a].GetFileName().GetName().CmpNoCase(thumbnails[b].GetFileName().GetName()) < 0;
            return (CompareStringsNatural_(thumbnails[a].GetFileName().GetName(), thumbnails[b].GetFileName().GetName()) < 0);
        }

        if (sortType == SORT_DATE_FORWARDS)
        {
            //std::cout << a << "<" << b << std::endl;
            //std::cout << thumbnails[a].GetDateTime().FormatISOCombined() << "<" << thumbnails[b].GetDateTime().FormatISOCombined() << std::endl;
            return thumbnails[a].GetDateTime().IsEarlierThan(thumbnails[b].GetDateTime());
        }

        return true;

    }

private:
    int                           sortType;
    const std::vector<Thumbnail>& thumbnails;
};


void SortedVectorInts::MergeInPlace(const SortedVectorInts& v2)
{
    int n1 = v.size();  // Size of the first vector
    int n2 = v2.size();  // Size of the second vector


    // Indices to traverse vectors
    int i = n1 - 1; // Last index of the original v1
    int j = n2 - 1; // Last index of v2
    int newCount = 0;

    while (i >= 0 && j >= 0)
    {
        if (v[i] > v2.Get(j))
        {
            i--;
        }
        else if (v[i] < v2.Get(j))
        {
            j--;
        }
        else
        {
            i--;
            j--;
        }
        newCount++;
    }

    if (i >= 0) newCount += i + 1;
    if (j >= 0) newCount += j + 1;

    i = n1 - 1; // Last index of the original v1
    j = n2 - 1; // Last index of v2
    int k = newCount - 1;

    v.resize(newCount);


    while (i >= 0 && j >= 0)
    {
        if (v[i] > v2.Get(j))
        {
            v[k--] = v[i--];  // Place the larger value at position k
        }
        else if (v[i] < v2.Get(j))
        {
            v[k--] = v2.Get(j--);  // Place the larger value at position k
        }
        else if (v[i] == v2.Get(j))
        {
            v[k--] = v[i--];
            j--;
        }
    }

    while (j >= 0)
    {
        v[k--] = v2.Get(j--);
    }

    while (i >= 0)
    {
        v[k--] = v[i--];
    }
}


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
    SetPriority(wxPRIORITY_MIN);
    thumbnail.isLoading = true;
    //NoteTime("starting");
    started             = true;

    wxFileName fn(fileName);
    wxString   extension = fn.GetExt().Upper();
        
    ImageFileHandler *imageFileHandler = ImageFileHandlerRegistry::instance().GetImageFileHandlerFromExtension(extension);

    if (imageFileHandler)
    {
        //cout << "Loading " << fn.GetFullPath() << endl;

        bool success = imageFileHandler->LoadThumbnail(fn.GetFullPath(), thumbnail);
        delete imageFileHandler;

        if (!success)
        {
            //cout << "Failed to load thumbnail, trying again" << endl;
            if ((extension == "JPEG") || (extension == "JPG"))
            {
                imageFileHandler = ImageFileHandlerRegistry::instance().GetImageFileHandlerFromExtension("png");
                bool  newSuccess = imageFileHandler->LoadThumbnail(fn.GetFullPath(), thumbnail);
                delete imageFileHandler;

                if (newSuccess)
                {
                    //cout << "  it was a PNG " << endl;
                    wxFileName ren = fn;
                    //ren.SetExt("png");
                    //wxRenameFile(fn.GetFullPath(), ren.GetFullPath());
                    thumbnail.fullPath = ren;
                }
            }
            else
            {
                wxImage image;
                //cout << "Creating XOR" << endl;
                image.Create(32, 32);
                unsigned char* data = image.GetData();
                for (int y = 0; y < 32; y++)
                    for (int x = 0; x < 32; x++)
                    {
                        *data++ = x ^ y;
                        *data++ = (x * 2) ^ (y * 2);
                        *data++ = (x * 3) ^ (y * 3);
                    }
                image.Destroy();
            }
        }
    }
    else
    {
        //cout << "Loaded OK\n";
        thumbnail.imageLoaded = true;
    }
    //NoteTime("stopped");

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



Thumbnail::Thumbnail(const wxPoint &pos, wxFileName path, bool fetchHeader, FileNameList &fileNameList, size_t uID)
: uniqueID(uID),
  fullPath(path),
  position(pos),
  isLoading(false),
  imageLoaded(false),
  hasBeenDrawn(false),
  imageSize(0, 0),
  thumbnailLoader(0)
{
    thumbnailLoader = new ThumbnailLoader(path.GetFullPath(), *this, fileNameList);

    if (fetchHeader)
        FetchHeader();
}

void Thumbnail::PauseLoadingThumbnail(int milliSeconds)
{
    //cout << "PauseLoadingThumbnail: " << thumbnailLoader << ": ";
    if (thumbnailLoader)
    {
        //cout << "loader exists. ";
        if (!imageLoaded)
        {
            //cout << "is alive";
            //NoteTime("Sleeping");
            thumbnailLoader->Sleep(1000);
            //NoteTime("done");
        }
    }

    //if (isLoading)
    //{
    //    cout << "loading ";
    //    if (!imageLoaded)
    //    {
    //        cout << "not finished";
    //        thumbnailLoader->Sleep(20);
    //    }
    //}
    //cout << endl;
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
        else
        {
            // Something went wrong with loading.
            // Sometimes a JPEG file will actually be a misnamed PNG file.
            // Let's check for that.
        }
    }
}


Thumbnail::~Thumbnail()
{
    if (!imageLoaded)
    {
        if (isLoading)
        {
            if (thumbnailLoader)
                thumbnailLoader->Delete();
        }
    }
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
    wxRect textRectangle(position.x, position.y + tnSize.GetHeight()+5, tnSize.GetWidth(), 20);
	wxPen pen;

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
    

    if (imageLoaded && bitmap.IsOk())
    {
        thumbnailLoader = 0;
        int x = position.x + (tnSize.GetWidth()  - bitmap.GetWidth() ) / 2;
        int y = position.y + (tnSize.GetHeight() - bitmap.GetHeight()) / 2;

        dc.DrawBitmap(bitmap, x, y, false);
    }
    else
    {
        dc.SetBrush(wxBrush(wxColor(32,32,32)));
        dc.SetPen(*wxBLACK_PEN);

        if (imageSizeTemp.x*imageSizeTemp.y > 0)
        {
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


void CmpTest(wxString a, wxString b)
{
    //cout << wxCmpNaturalGeneric(a, b) << " " << a << " - " << b << endl;
}


ThumbnailCanvas::ThumbnailCanvas(ImageBrowser* imgBrs, FileNameList& fNameList, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
: wxScrolledWindow(parent, id, pos, size, wxSUNKEN_BORDER | wxVSCROLL | wxEXPAND | wxWANTS_CHARS),
  fileNameList(fNameList),
  inFocus(false),
  sortingType(ThumbnailSortingFunctor::SORT_NUMS_FORWARDS),
  cursorP(tnColumns, fNameList),
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
  maxLoading(MAX_THUMBNAILS_LOADING),
  imageViewer(0),
  imageBrowser(imgBrs),
  popUpMenu(0),
  m_availableID(1),
  thumbnailLoadingActive(false)
{

    SetBackgroundColour(backgroundColor);
    //ImageFileHandlerRegistry& imageFileHandlerRegistry = ImageFileHandlerRegistry::instance();
    //wxArrayString &filtersList = imageFileHandlerRegistry.GetFiltersList();
    //
    //int i, n = filtersList.size();
    //
    //for (i = 0; i < n; i++)
    //{
    //    fileNameList.AddFilter(filtersList[i].Lower());
    //}

    Thumbnail::SetSize(tnSize);
    Thumbnail::SetSelectBorder(3);
    Thumbnail::SetLabelHeight(26);
    Thumbnail::SetBackgroundColor(backgroundColor);

    RecalculateRowsCols();
    
    
    DragAcceptFiles(true);
    Bind(wxEVT_DROP_FILES, &ThumbnailCanvas::OnDropFiles, this, -1);

    cout << wxCmpNatural("FtvwWJE.jpg", "ggnExrO.jpg");
    cout << wxCmpNatural("ggnExrO.jpg", "FtvwWJE.jpg");
    cout << wxCmpNatural("001.jpg", "01dc6e1118884954.jpg");
    cout << wxCmpNatural("001.jpg", "002.jpg");
    cout << wxCmpNatural("_MetArt-Appellato-cover.jpg", "001.jpg");
    cout << wxCmpNatural("_MetArt-Appellato-cover.jpg", "FtvwWJE.jpg");

}

void ThumbnailCanvas::SetImageViewer(ImageViewer *iv)
{
    imageViewer = iv;
    //imageViewer->SetFileNameList(&fileNameList);
}


void ThumbnailCanvas::OnDropFiles(wxDropFilesEvent& event)
{
    //cout << "ThumbnailCanvas::OnDropFiles()" << endl;

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
            //cout << "  copy " << name << " to " << destination << endl;
            wxCopyFile(name, destination);

            thumbnailIndex.push_back(thumbnails.size());
            waitingSet.AddSingle(thumbnails.size());
            thumbnails.emplace_back(wxPoint(0, 0), destination, false, fileNameList, GetUiniqueID());    // Fixme. Need a uniqueID
            fileNameList.AddFileToList(destination);
        }
        fileNameList.Resort();
        RecalculateRowsCols();
    }
}

void ThumbnailCanvas::OnDropFilesTree(wxDropFilesEvent& event)
{
    //cout << "ThumbnailCanvas::OnDropFilesTree()" << endl;
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
    //cout << "  " << GetViewStart().y << endl;
    //cout << "  " << GetVirtualSize().y << endl;

    if (!n)
        return;

    //cout << "Redraw type " << redrawType << endl;
    switch (redrawType)
    {
        case REDRAW_ALL:
            //cout << "Redrawing ALL" << endl;
            for (int i = 0; i < n; i++)
            {
                //cout << thumbnailIndex[i] << endl;
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
            //cout << "    redrawSetP.size() = " << redrawSetP.size() << endl;
            //cout << "    redrawSetP = ";        redrawSetP.Print();

            for (int i = 0; i<redrawSetP.size(); i++)
            {

				int th = redrawSetP[i];
                //cout << "  th = " << th << endl;
                if (th >= 0)
                {
                    thumbnails[thumbnailIndex[th]].Erase(dc);
                }
            }

            for (int i=0; i<redrawSetP.size(); i++)
            {
                int th = redrawSetP[i];
                //cout << "  redrawSetP.size() = " << redrawSetP.size() << endl;
                selected = selectionSetP.Contains(th);

                //selectionSetP.Print();
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

	cursorP.SetupRedraw(redrawSetP);
    redrawSetP.AddFrom(selectionSetP);

    bool selectionChange = false;
	if (event.ShiftDown())
	{
		selectionSetP.SelectTo(cursorP.GetNumber());
        selectionChange = true;
	}
	else
	{
        selectionSetP.SelectFrom(cursorP.GetNumber());
        selectionSetP.AddSingle(cursorP.GetNumber());
    }

    redrawSetP.AddFrom(selectionSetP);

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
    //UpdateStatusBar_File();
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
        //cout << "event.RequestMore()\n";
        //STATUS_TEXT(3, "Loading Images ...");
    }
    else
    {
        //cout << "event.RequestMore() stopped\n";
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
    //cout << "ThumbnailCanvas::UnLoadThumbnails(" << directory << ")" << endl;
    //cout << "  " << fileNameList.directory.GetName() << endl;

    if ( (                        directory.StartsWith(fileNameList.directory.GetNameWithSep()) ) ||
         ( fileNameList.directory.GetName().StartsWith(directory)        )
       )
    {
        //cout << "  Unloading" << endl;
        ClearThumbnails();
        imageViewer->ClearCache();
        Scroll(0, 0);
    }
    //cout << "  Done" << endl;
}


bool ThumbnailCanvas::CheckPasswordProtection(wxString directory)
{
    wxFileName passFile(directory);

    passFile.SetName(".pass");

    //cout << "passFile: " << passFile.GetFullPath() << endl;

    while (passFile.GetDirCount())
    {
        //cout << "  " << passFile.GetFullPath() << endl;
        if (passFile.Exists())
        {
            //cout << "    Exists\n";

            if (allowedDirectories.Index(passFile.GetPath()) == wxNOT_FOUND)
            {
                allowedDirectories.push_back(passFile.GetPath());
                PasswordDialog passwordDialog;

                if (passwordDialog.ShowModal() == wxID_OK)
                {

                }
            }
        }

        passFile.RemoveLastDir();
    }
    return false;
}


void ThumbnailCanvas::LoadThumbnails(wxString directory)
{
	cout << "LoadThumbnails(" << directory << ")" << endl;
    //CheckPasswordProtection(directory);

    inFocus = false;
    ClearThumbnails();
    imageViewer->ClearCache();
    imageViewer->ResetZoom();
    fileNameList.LoadFileList(directory);

    int n = fileNameList.files.size();
    thumbnails.reserve(n);thumbnailIndex.reserve(n);
    Scroll(0, 0);

    totalDirectorySizeBytes = 0;
    ResetUiniqueID();

    if (n > 0)
        SetCursor(0);

    for (int i = 0; i<n; i++)
    {
        wxString fullPath = directory;
        fullPath += wxT("\\");
        fullPath += fileNameList.files[i].fileName.GetFullPath();
        wxFileName fn(fullPath);
        wxDateTime dateTime = wxDateTime::Now();

        if (fn.IsOk())
        {
            dateTime = fn.GetModificationTime();
            totalDirectorySizeBytes += fn.GetSize();
        }

        thumbnailIndex.push_back(thumbnails.size());
        thumbnails.emplace_back(wxPoint(0,0), fullPath, false, fileNameList, GetUiniqueID());
        thumbnails.back().SetDateTime(dateTime);
        //cout << "  " << thumbnails.back().GetFileName().GetName() << endl;
    }

    readHeadersCompleted = true;

    wxString hrs = HumanFileSize(totalDirectorySizeBytes.GetLo());
    STATUS_TEXT(STATUS_BAR_DIRECTORY_SUMMARY, "%d files (%s)", n, hrs.c_str());
    
    if (n > 0)
    {
        waitingSet.SetRange(0, n - 1);
    }
	else
	{
		waitingSet.Clear();
	}
    loadingSet.Clear();
    redrawSetP.Clear();
    thumbnailLoadingActive = true;

    sortingType = ThumbnailSortingFunctor::SORT_NUMS_FORWARDS;
    SortThumbnails();
}

void ThumbnailCanvas::StopLoadingThumbnails(wxString directory)
{
    //cout << "StopLoadingThumbnails()" << endl;
    if ((directory.StartsWith(fileNameList.directory.GetNameWithSep())) ||
        (fileNameList.directory.GetName().StartsWith(directory))
        )
    {
        //thumbnailLoadingActive = false;
        waitingSet.Clear();
    }
}

void ThumbnailCanvas::ContinueLoadingThumbnails()
{
    maxLoading = MAX_THUMBNAILS_LOADING;
}

void ThumbnailCanvas::PauseLoadingThumbnails()
{
    maxLoading = 0;
}

int  ThumbnailCanvas::FindThumbnailIndex(int th)
{
    int i, n = thumbnailIndex.size();

    for (i = 0; i < n; i++)
    {
        if (thumbnailIndex[i] == th)
            return i;
    }

    return -1;
}

bool ThumbnailCanvas::HandleThumbnailLoading()
{
    int i, n = loadingSet.size();

    //cout << "ThumbnailCanvas::HandleThumbnailLoading()" << endl;


    for (i = 0; i < n; i++)                         // Check if any thumbnails have finished loading
    {
        int th = loadingSet[i];
        //cout << "  loadingSet[" << i << "] = " << th << endl;
        //if (thumbnails[thumbnailIndex[th]].ImageIsLoaded())
        if (thumbnails[th].ImageIsLoaded())
            {
            loadingSet.RemoveSingle(th);
            redrawSetP.AddSingle(FindThumbnailIndex(th));
            i--;
            n--;

            if (th == cursorP.GetNumber())          // Did we just load the selected thumbnail?
                UpdateStatusBar_File();             // Then we have the information we need to fill in the status bar.
        }
    }

    n = maxLoading - loadingSet.size();             // How many more thumbnails can we start loading?
    if (n > waitingSet.size())                      // But obviously not more than actually need to be loaded.
        n = waitingSet.size();

    for (i = 0; i < n; i++)                         // Start loading those thumbnails
    {
        int th = waitingSet[i];
        if (th >= 0)
        {
            //cout << "Starting " << th << " loading " << thumbnails[thumbnailIndex[th]].GetFileName().GetFullName() << endl;
            //thumbnails[thumbnailIndex[th]].StartLoadingImage();
            thumbnails[th].StartLoadingImage();
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

    //cout << "loadingSet.size() = " << loadingSet.size() << endl;
    //cout << "waitingSet.size() = " << waitingSet.size() << endl;

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

void ThumbnailCanvas::IndexLookup(SortedVectorInts& vec)
{
    for (int i=0; i<vec.size(); i++)
    {
        vec[i] = thumbnailIndex[vec[i]];
    }
}

void SetDebuggingText(wxString text);

void ThumbnailCanvas::OnMouseEvent(wxMouseEvent &event)
{
    wxPoint physicalPosition = event.GetPosition();
    wxPoint  logicalPosition =   CalcUnscrolledPosition(physicalPosition);
    int                   th = GetThumbnailFromPosition(logicalPosition);

    wxString lines, s;

    if (event.GetWheelRotation())
    {
        OnMouseWheel(event);
        return;
    }

    if (event.RightDown())
    {
        OnContextMenu(event);
        return;
    }

    if (event.LeftDClick())
    {
        SetFocus();
        if (th >= 0)
        {
            cursorP.SetTo(th);
            ActivateThumbnail(cursorP.GetNumber());
            redrawType = REDRAW_ALL;
        }
        else        { /* failure! */ }
    }


    if (event.LeftDown() && event.ShiftDown())
    {
        SetFocus();

        if (th >= 0)
        {
            cursorP.SetTo(th);
            cursorP.SetupRedraw(redrawSetP);
            redrawSetP.AddFrom(selectionSetP);      // Need to erase any previously selected thumbs
            selectionSetP.SelectTo(th);
            dragState = TNC_DRAG_STATE_NONE;

            UpdateStatusBar_File();
            Refresh(DONT_ERASE_BACKGROUND);
        }
    }
    else if (event.LeftDown() && event.ControlDown())
    {
        SetFocus();

        if (th >= 0)
        {
            redrawSetP.AddFrom(selectionSetP);      // Need to erase any previously selected thumbs
            selectionSetP.ToggleSingle(th);
            if (selectionSetP.size() > 0)
            {
                cursorP.SetTo(selectionSetP[0]);            // Cursor is always the first selected item
            }

            dragState = TNC_DRAG_STATE_NONE;

            UpdateStatusBar_File();
            Refresh(DONT_ERASE_BACKGROUND);
        }
    }
    else if (event.LeftDown())
    {
        SetFocus();

        if (th >= 0)
        {
            if (!selectionSetP.Contains(th))                // Clicking on an unselected thumb
            {
                cursorP.SetTo(th);
                cursorP.SetupRedraw(redrawSetP);
                redrawSetP.AddFrom(selectionSetP);
                selectionSetP.Clear();
                selectionSetP.SelectFrom(th);
                selectionSetP.AddSingle(th);
                redrawSetP.AddSingle(th);

                //selectionSetP.Print();
                //redrawSetP.Print();
            }
            else                                            // Clicking on a SELECTED thumb
            {
            }

            UpdateStatusBar_File();
            clickPoint = event.GetPosition();

            dragState  = TNC_DRAG_STATE_CLICKED;
            redrawType = REDRAW_SELECTION;
            Refresh(DONT_ERASE_BACKGROUND);
        }
    }

    //if (event.Dragging())
    //{
    //    cout << "Dragging! " << dragState << endl;
    //}


    if (event.Dragging() && dragState != TNC_DRAG_STATE_NONE)
    {
        if (dragState == TNC_DRAG_STATE_CLICKED)
        {
            // We will start dragging if we've moved beyond a couple of pixels

            int tolerance = 2;
            int dx = abs(event.GetPosition().x - clickPoint.x);
            int dy = abs(event.GetPosition().y - clickPoint.y);

            if (dx <= tolerance && dy <= tolerance)
                return;

            // Start the drag.
            dragState = TNC_DRAG_STATE_DRAGGING;
            imageBrowser->DragStart();
            if (selectionSetP.Contains(th))
            {
                draggingSet.CopyFrom(selectionSetP);
                IndexLookup(draggingSet);
            }
            else
            {
                draggingSet.Clear();
                draggingSet.AddSingle(th);
            }
            //draggingSet.Print();

            wxWindow::SetCursor(wxCursor(wxCURSOR_HAND));
        }
        else if (dragState == TNC_DRAG_STATE_DRAGGING)
        {
            dragingFilesDataObject = new wxFileDataObject();
            vector<wxFileName>  fileNamesToMaybeRemove;
            
            for (int i = 0; i < draggingSet.size(); i++)
            {
                int fNum = draggingSet[i];
                //cout << "  " << fileNameList[fNum] << endl;
                fileNamesToMaybeRemove.push_back(fileNameList[fNum]);
                dragingFilesDataObject->AddFile(fileNameList[fNum]);
            }

            wxDropSource dragSource(this);
            dragSource.SetData(*dragingFilesDataObject);
            wxDragResult result = dragSource.DoDragDrop(true);                  // Dragging starts now!
                                                                                // ====================
                                                                                // Items have been dropped!            
            RemoveThumbNailFromCanvasIfDeleted(fileNamesToMaybeRemove);
        }
    }

    wxString selectedFiles = "";
    wxString number;
    /*
    for (int i = 0; i < selectionSetP.size(); i++)
    {
        int selection = selectionSetP[i];
        int index = thumbnailIndex[selection];
        number.Printf("%d %d %d: ", i, selection, index);
        selectedFiles.Append(number);
        selectedFiles.Append(thumbnails[index].GetFileName().GetFullPath());
        selectedFiles.Append("\n");
    }

    selectedFiles.Append("\nRedraw:\n");
    for (int i = 0; i < redrawSetP.size(); i++)
    {
        int selection = redrawSetP[i];
        int index = thumbnailIndex[selection];
        number.Printf("%d %d %d: ", i, selection, index);
        selectedFiles.Append(number);
        selectedFiles.Append(thumbnails[index].GetFileName().GetFullPath());
        selectedFiles.Append("\n");
    }

    SetDebuggingText(selectedFiles);
    */

    //lines += wxT("selectionSetP = ");                   lines += selectionSetP.SPrint();
    //lines += wxT("draggingSet   = ");                   lines += draggingSet.SPrint();
    //s.Printf(wxT("dragState     = %d\n"), dragState);   lines += s;

    //SetDebuggingText(lines);
    //cout << "Done" << endl;
    //Update();
}

void ThumbnailCanvas::ActivateThumbnail(int n)
{
    //cout << "ThumbnailCanvas::ActivateThumbnail(" << n << ")" << endl;
    //cout << "  " << fileNameList.directory.GetName() << endl << endl;

    if ( (n < 0) || (n > fileNameList.MaxFileNumber()) )
        return;

    int               index            = thumbnailIndex[n];
    wxFileName        fileName         = fileNameList[index];
    wxString          extension        = fileName.GetExt();
    ImageFileHandler* imageFileHandler = ImageFileHandlerRegistry::instance().GetImageFileHandlerFromExtension(extension);

    //cout << "Activating " << fileName.GetFullPath() << endl;

    int actions = imageFileHandler->LoadImage(fileName.GetFullPath());

    //cout << "Actions = " << actions << endl;

    if (actions & LOAD_IMAGE)
    {
        //cout << "Displaying " << fileName.GetFullPath() << endl;
        //imageViewer->DisplayImage(cursorP.GetNumber());
        imageViewer->DisplayImage(fileName);
        
    }

    if (actions & DELETE_FILE)
    {
        DeleteSelection();
    }

    if (actions & REFRESH_TREE)
    {
        //cout << "  refreshing tree: " << fileName.GetPathWithSep() << endl;
        imageBrowser->RefreshDirTree(fileName.GetPathWithSep());
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
    //cout << "UpdateStatusBar_File()" << endl;
    //if (selectionSetP.size() == 0)
    //{
        //cout << "selectionSetP.size() == 0" << endl;

        if (thumbnailIndex.size() > 0)
        {
            //cout << "thumbnailIndex.size() > 0" << endl;
            int index = thumbnailIndex[cursorP.GetNumber()];
            Thumbnail *tn = &thumbnails[index];

            //cout << "  " << index << endl;
            //cout << "Update Status: " << tn->GetFullPath().GetFullPath() << endl;

            wxFileName fn         = tn->GetFullPath();

            if (!fn.IsOk())
            {
                STATUS_TEXT(STATUS_BAR_INFORMATION, "File name not OK");
                return;
            }

            wxDateTime date = fn.GetModificationTime();

            if (!date.IsValid())
            {
                STATUS_TEXT(STATUS_BAR_INFORMATION, "Date not OK");
                return;
            }

            wxString   dateString = date.Format("%d/%m/%Y  %H:%M");
            wxSize     imageSize = tn->GetImageSize();

            STATUS_TEXT(STATUS_BAR_FILE_SIZES, "%s,  %s - (%d x %d)", fn.GetHumanReadableSize().c_str(), dateString, imageSize.GetWidth(), imageSize.GetHeight());
            STATUS_TEXT(STATUS_BAR_FILE_FORMAT, fn.GetFullName().c_str());
            STATUS_TEXT(STATUS_BAR_INFORMATION, "cursor = %d, index = %d, sel = %d", cursorP.GetNumber(), index, selectionSetP[0]);
        }
    //}
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
    int scrollPos = GetScrollPos(wxVERTICAL) + event.GetWheelRotation() * 10;

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

void ThumbnailCanvas::SetCursor(int tn)
{
    cursorP.SetTo(tn);
    HandleCursorScrolling();
}

void ThumbnailCanvas::SetCursor(wxFileName fileName)
{
    int sortedIndex = GetSortedImageNumber(fileName);
    cursorP.SetTo(sortedIndex);
    HandleCursorScrolling();
}

int ThumbnailCanvas::FindThumbnailIndex(wxFileName fileName)
{
    for (int i = 0; i < thumbnails.size(); i++)                         // Find it in the thumb
    {
        if (thumbnails[i].GetFullPath() == fileName)
        {
            return i;
        }
    }
    return -1;
}

void ThumbnailCanvas::RemoveThumbNailFromCanvasIfDeleted(vector<wxFileName>& fileNamesToMaybeRemove)
{
    //cout << "RemoveThumbNailFromCanvas(" << fileName.GetFullPath() << ")" << endl;

    //int tn = fileNameList.GetFileNumber(fileName);

    std::vector<int>::iterator iter = thumbnailIndex.begin();

    //waitingSet.RemoveSingle(tn);
    //loadingSet.RemoveSingle(tn);
    //redrawSetP.Clear();

    // Delete the thumbnails from the main array using iterators
    // Rebuild the thumbnailIndex array from scratch, then sort it.

    for (int d = 0; d < fileNamesToMaybeRemove.size(); d++)                 // for each filename ot maybe remove
    {
        int index = FindThumbnailIndex(fileNamesToMaybeRemove[d]);          // Find it

        if (index >= 0)
        {
            thumbnails[index] = thumbnails.back();                          // and delete it.
            thumbnails.pop_back();
        }
    }

    // now rebuild the sorted index

    thumbnailIndex.clear();
    for (int i = 0; i < thumbnails.size(); i++)
    {
        thumbnailIndex.push_back(i);
    }
    SortThumbnails();
    //RecalculateRowsCols();
    //redrawType = REDRAW_ALL;
    //Refresh(ERASE_BACKGROUND);
}

// Delete the image on disk, and delete the thumbnail pointer, but keep the thumbnail.
void ThumbnailCanvas::DeleteImage(wxFileName fileName)
{
    //cout << "ThumbnailCanvas::DeleteImage(" << fileName.GetFullPath() << ")" << endl;
    int tn = FindThumbnailIndex(fileName);

    if (tn < 0)
    {
        //cout << "Can't find filename for delete image" << endl;
        return;
    }


    //cout << "Deleting " << fileName.GetFullPath() << endl;

    bool succesfullyDeleted = wxRemoveFile(fileName.GetFullPath());

    if (succesfullyDeleted)
    {
        thumbnails[tn] = thumbnails.back();                              // and delete it.
        thumbnails.pop_back();
    }

    thumbnailIndex.clear();
    for (int i = 0; i < thumbnails.size(); i++)
    {
        thumbnailIndex.push_back(i);
    }
    SortThumbnails();
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
    wxString debugString;

    //cout << "ThumbnailCanvas::DeleteSelection()" << endl;
    //cout << selectionSetP.size() << " files selected" << endl;

    if (selectionSetP.size() == 0)
    {
        selectionSetP.AddSingle(cursorP.GetNumber());
        //cout << selectionSetP.size() << " files selected" << endl;
    }

    int newCursor = 0;
    int firstSelection = selectionSetP.Front();
    int lastSelection  = selectionSetP.Back();
    //vector<wxFileName>  fileNamesToDelete;

    newCursor = lastSelection + 1 - selectionSetP.size();

    // List the filenames to delete


    //fileNamesToDelete.reserve(selectionSetP.size());

    for (int s= selectionSetP.size()-1; s>=0; s--)              // Loop through the selection back to front
    {
        int selectedItem = selectionSetP[s];
        int index = thumbnailIndex[selectedItem];

        wxString filenameToDelete = thumbnails[index].GetFullPath().GetFullPath();
        
        bool succesfullyDeleted = wxRemoveFile(filenameToDelete);

        if (succesfullyDeleted)
        {
            thumbnailIndex[selectedItem] = thumbnailIndex.back();                      // and delete each thumbnail.
            thumbnailIndex.pop_back();
        }
    }

    selectionSetP.Clear();
    SortThumbnails();

    for (int i = 0; i < thumbnailIndex.size(); i++)
    {
        int index = thumbnailIndex[i];
    }

    newCursor = clamp(newCursor, 0, (int)thumbnailIndex.size() - 1);
    cursorP.SetTo(newCursor);
}

/*
void ThumbnailCanvas::DeleteSelection()
{
    cout << "ThumbnailCanvas::DeleteSelection()" << endl;
    cout << selectionSetP.size() << " files selected" << endl;

    if (selectionSetP.size() == 0)
    {
        selectionSetP.AddSingle(cursorP.GetNumber());
        cout << selectionSetP.size() << " files selected" << endl;
    }

    // List the filenames to delete

    vector<wxFileName>  fileNamesToDelete;

    fileNamesToDelete.reserve(selectionSetP.size());

    for (int s = 0; s < selectionSetP.size(); s++)         // First, compile a list of the filenames to delete
    {
        int        selection = selectionSetP[s];
        int        index = thumbnailIndex[selection];
        wxFileName fileName = thumbnails[index].GetFullPath();
        fileNamesToDelete.push_back(fileName);
        cout << "  " << fileName.GetFullPath() << endl;
    }

    for (int d = 0; d < fileNamesToDelete.size(); d++)                          // for each filename to delete
    {
        wxFileName fileName = fileNamesToDelete[d];
        int        index = FindThumbnailIndex(fileName);                     // Find it

        if (index >= 0)
        {
            cout << "Deleting " << fileName.GetFullPath() << "  index " << index << endl;
            //fileNameList.DeleteFileName(fileName);
            thumbnails[index] = thumbnails.back();                              // and delete it.
            thumbnails.pop_back();
        }
    }

    // now rebuild the sorted index

    selectionSetP.Clear();
    thumbnailIndex.clear();
    for (int i = 0; i < thumbnails.size(); i++)
    {
        thumbnailIndex.push_back(i);
    }
    SortThumbnails();

    cout << "New list of images" << endl;
    for (int i = 0; i < thumbnails.size(); i++)
    {
        cout << "  " << thumbnails[i].GetFullPath().GetFullPath() << endl;
    }
}
*/
void ThumbnailCanvas::AddPopupMenuItem(const wxString& label, void(ThumbnailCanvas::* function)(wxCommandEvent&))
{
    if (!popUpMenu)
        return;

    int id = GetAvailableID();
    popUpMenu->Append(id, label);
    Bind(wxEVT_MENU, function, this, id);
}

void ThumbnailCanvas::OnContextMenu(wxMouseEvent& event)
{
    wxMenu menu;
    popUpMenu = &menu;
    wxON_BLOCK_EXIT_NULL(popUpMenu);

    AddPopupMenuItem("Rename Sequence",         &ThumbnailCanvas::RenameSequence);
    AddPopupMenuItem("Sort By Name",            &ThumbnailCanvas::SortThumbnailsByName);
    AddPopupMenuItem("Sort By Name (numbers)",  &ThumbnailCanvas::SortThumbnailsByNameNumbers);
    AddPopupMenuItem("Sort By Date",            &ThumbnailCanvas::SortThumbnailsByDate);

    wxCommandEvent event2(8);
    event2.SetString("File Sort Menu");
    GetEventHandler()->SafelyProcessEvent(event2);              // Create an event so that the parent is informed that the menu has
                                                                // opened. Then the parent can add menu items if they want.
    PopupMenu(popUpMenu);
}



void ThumbnailCanvas::RenameSequence(wxCommandEvent& evt)
{
    PasswordDialog passwordDialog;



    if (passwordDialog.ShowModal() == wxID_OK)
    {

    }
}

wxString ThumbnailCanvas::GetInfoString(wxFileName fileName)
{
    //cout << "ThumbnailCanvas::GetInfoString(" << fileName.GetFullPath() << ")" << endl;

    int        index = 0;
    wxString   infoString;
    wxString   nameString;
    wxString   dimensionsString;
    wxString   numberString;
    Thumbnail *thumbnail   = 0;
    int        imageNumber = 1;

    for (auto i: thumbnailIndex)
    {
        if (thumbnails[i].GetFullPath() == fileName.GetFullPath())
        {
            thumbnail = &thumbnails[i];
            index = i;
            break;
        }

        imageNumber++;
    }

    //cout << index << " " << thumbnail << endl;

    if (!thumbnail)
    {
        cout << "ERROR" << endl;
        return "ERROR";
    }

    nameString = fileName.GetFullPath();
    dimensionsString.Printf("\n%dx%d\n", thumbnail->GetImageSize().x, thumbnail->GetImageSize().y);
    numberString.Printf("%d of %d", (int)index, (int)thumbnailIndex.size());

    infoString = nameString + dimensionsString + numberString;

    return infoString;
}

void ThumbnailCanvas::SortThumbnails()
{
    ThumbnailSortingFunctor sortingFunctor(sortingType, thumbnails);
    sort(thumbnailIndex.begin(), thumbnailIndex.end(), sortingFunctor);

    //int i = 0;
    //for (auto index : thumbnailIndex)
    //{
    //    cout << i << ": " << index << endl;
    //    i++;
    //}

    RecalculateRowsCols();
    redrawType = REDRAW_ALL;
    Refresh(ERASE_BACKGROUND);
    wxWakeUpIdle();
}

void ThumbnailCanvas::SortThumbnailsByName(wxCommandEvent & evt)
{
    sortingType = ThumbnailSortingFunctor::SORT_NAME_FORWARDS;
    SortThumbnails();
}

void ThumbnailCanvas::SortThumbnailsByNameNumbers(wxCommandEvent& evt)
{
    sortingType = ThumbnailSortingFunctor::SORT_NUMS_FORWARDS;
    SortThumbnails();
}

void ThumbnailCanvas::SortThumbnailsByDate(wxCommandEvent& evt)
{
    sortingType = ThumbnailSortingFunctor::SORT_DATE_FORWARDS;
    SortThumbnails();
}

wxString ThumbnailCanvas::GetExtensionOfImageNumber(size_t imageNumber) const
{
    if (imageNumber >= thumbnailIndex.size())
    {
        return "";
    }

    size_t     index    = thumbnailIndex[imageNumber];

    if (index >= thumbnails.size())
    {
        return "";
    }

    wxFileName fileName = thumbnails[index].GetFileName();

    wxString ext = fileName.GetExt();
    return ext.MakeLower();
}

/*
wxFileName ThumbnailCanvas::Jump(size_t &currentImage, int delta, wxString viewableExtensions) const
{
    size_t originalImageNumber = currentImage;
    size_t maxFileNumber = thumbnailIndex.size() - 1;

    cout << "ThumbnailCanvas::Jump(" << currentImage << ", " << delta << ", " << viewableExtensions << ")" << endl;
    currentImage += delta;
    currentImage = clamp(currentImage, (size_t)0, maxFileNumber);
    //cout << "  nextImage = " << nextImage << endl;

    //bool currentImageIsViewable = viewableExtensions.Contains(GetExtensionOfImageNumber(currentImage));
    //if (currentImageIsViewable)
    //{
    //    return thumbnails[index].GetFileName();
    //}

    for (auto index : thumbnailIndex)
    {

    }

    while (true)
    {
        wxString extensionOfCurrentImage = GetExtensionOfImageNumber(currentImage);
        bool     currentImageIsViewable  = viewableExtensions.Contains(extensionOfCurrentImage);

        if (currentImageIsViewable)
        {
            int index = thumbnailIndex[currentImage];
            return thumbnails[index].GetFileName();
        }

        if (delta > 0)      currentImage++;
        else                currentImage--;

        bool wentOffTheEnd = currentImage > maxFileNumber;
        if (wentOffTheEnd)                                                  // Hit the end of the list? (start or end because it's unsigned)
        {
            currentImage = originalImageNumber;
            return wxFileName("");                                          // .. then just give up looking.
        }
    }
}
*/
int ThumbnailCanvas::GetSortedImageNumber(wxFileName fileName) const
{
    for (size_t i = 0; i < thumbnailIndex.size(); i++)
    {
        size_t index = thumbnailIndex[i];
        if (thumbnails[index].GetFileName() == fileName)
        {
            return i;
        }
    }
    return -1;
}



wxFileName ThumbnailCanvas::Jump(wxFileName& currentImage, int delta, wxString viewableExtensions) const
{
    int    maxFileNumber = thumbnailIndex.size() - 1;
    size_t originalImageNumber = 0;
    int    sortedIndex = GetSortedImageNumber(currentImage);

    if (sortedIndex < 0)                                                    // Error code. Something went really wrong here
        return currentImage;

    originalImageNumber = sortedIndex;                                      // remember this in case we can't seem to find the next image to jump to
    sortedIndex        += delta;
    sortedIndex         = clamp(sortedIndex, 0, maxFileNumber);

    while (true)
    {
        size_t index = thumbnailIndex[sortedIndex];

        wxString extensionOfCurrentImage = GetExtensionOfImageNumber(index);
        bool     currentImageIsViewable = viewableExtensions.Contains(extensionOfCurrentImage);

        if (currentImageIsViewable)
        {
            return thumbnails[index].GetFileName();
        }

        if (delta > 0)      sortedIndex++;
        else                sortedIndex--;

        bool wentOffTheEnd = sortedIndex > maxFileNumber;
        if (wentOffTheEnd)                                                  // Hit the end of the list? (start or end because it's unsigned)
        {
            sortedIndex = originalImageNumber;
            return currentImage;                                            // .. then just give up looking.
        }
    }
}


PasswordDialog::PasswordDialog()
 : wxDialog(NULL, wxID_ANY, wxT("Enter String"), wxDefaultPosition, wxSize(250, 230))
{
    wxBoxSizer* vbox  = new wxBoxSizer(wxVERTICAL);

    passCtrl  = new   wxTextCtrl(this, -1, "", wxPoint(100, 30), wxSize(100, 12));

    vbox->Add(passCtrl,  1, wxEXPAND);

    vbox->Add(CreateButtonSizer(wxOK | wxCANCEL), 1, wxEXPAND);

    SetSizer(vbox);

    Centre();
}

// libfftw3-3.lib;
