#include "wx/scrolwin.h"
#include <vector>
#include <deque>
#include <iostream>
#include "wx/bitmap.h"
#include "wx/filename.h"
#include "file_name_list.h"
#include "wx/thread.h"


extern "C"
{
#include "jpeg_turbo.h"
};

class wxFileDataObject;
class wxPaintDC;
class ImageViewer;
class Thumbnail;
class ThumbnailCanvas;
class ImageBrowser;

//-----------------------------------------------------------------------------
// MyCanvas
//-----------------------------------------------------------------------------

#define IS_BETWEEN(x, min, max)     ((x>=min) && (x<=max))
#define ERASE_BACKGROUND            true
#define DONT_ERASE_BACKGROUND       false


class SortedVectorInts
{
public:
    SortedVectorInts(size_t reserveAmount)
    {
        v.reserve(reserveAmount);
        //Test();
    }

    void AddFrom(SortedVectorInts &src)
    {
        v.reserve(v.size() + src.v.size());

        size_t i, n = src.v.size();

        for (i = 0; i < n; i++)
        {
            v.push_back(src.v[i]);
        }
    }

    void CopyFrom(SortedVectorInts &src)
    {
        v.clear();
        AddFrom(src);
    }


    void AddSingle(int s)
    {
        std::vector<int>::iterator i;

        for (i = v.begin(); i<v.end(); i++)
        {
            if (*i == s)
            {
                return;
            }

            if (*i > s)
            {
                v.insert(i, s);
                return;
            }
        }
        v.push_back(s);
    }

    void RemoveSingle(int s)
    {
        int i, n = v.size();
        for (i = 0; i < n; i++)
        {
            if (v[i] == s)
            {
                for (int j=i; j<n-1; j++)
                {
                    v[j] = v[j + 1];
                }
                v.pop_back();
                return;
            }
        }
    }

    void ToggleSingle(int s)
    {
        std::vector<int>::iterator i;

        for (i = v.begin(); i<v.end(); i++)
        {
            if (*i == s)                                // If this is already selected
            {
                v.erase(i);
                return;                                 // and we're finished.
            }

            if (*i < s)                                 // If this is NOT already selected
            {
                v.insert(i, s);                         // then we can insert here.
                return;
            }
        }
        v.push_back(s);
    }


    void SetRange(int f, int l)
    {
        int first, last;

        if (f <= l)
        {
            first = f;
            last  = l;
        }
        else
        {
            first = l;
            last  = f;
        }

        v.reserve(last - first + 1);
        v.clear();

        for (int i = first; i <= last; i++)
        {
            v.push_back(i);
        }
    }

    void Clear() { v.clear(); }

    void Print()
    {
        std::cout << "[";
        for (size_t i=0; i<v.size(); i++)
        {
            std::cout << v[i] << ", ";
        }
        std::cout << "]" << std::endl;
    }

    wxString SPrint()
    {
        wxString s;
        wxString t;

        s.Printf(wxT("%d ["), v.size());
        for (size_t i = 0; i<v.size(); i++)
        {
            t.Printf(wxT("%d, "), v[i]);
            s += t;
        }
        s += wxT("]\n");

        return s;
    }

    bool Contains(int s)
    {
        int first, last, middle, n;

        n = v.size();
        first = 0;
        last = n - 1;
        middle = (first + last) >> 1;


        while (first <= last)
        {
            if (v[middle] < s)
            {
                first = middle + 1;
            }
            else if (v[middle] == s)
            {
                return true;
            }
            else
            {
                last = middle - 1;
            }

            middle = (first + last) >> 1;
        }

        return false;
    }


	void SelectFrom(int a)
	{
		//v.resize(1);
		//v[0] = a;
		v.clear();
		selectionStart = a;
	}

	void SelectTo(int selectionEnd)
	{
		SetRange(selectionStart, selectionEnd);
	}

    int DistanceTo(int i)
    {
        if (i <= v.front())
        {
            return v.front() - i;
        }

        if (i >= v.back())
        {
            return i - v.back();
        }

        return 0;
    }

    int size() { return v.size(); }
    int operator [](int i) { return v[i];}

    bool Test();
    bool IsNotEmpty() { return v.size() > 0; }

    std::vector<int>     v;
	int selectionStart;

private:

    void Delete(size_t i)
    {

    }
};



class TnCursor
{
public:

    TnCursor(int &cols, FileNameList &fnl)
    : number(0),
      previous(0),
      numColumns(cols),
      fileNameList(fnl)
    {}


    void Move(int dx, int dy)
    {
        previous = number;
        x += dx;
        y += dy;

        if (y <        0)  y = 0;
        if (x <        0)  x = 0;
        if (x >= numColumns)  x = numColumns  - 1;

        number = y*numColumns + x;

        if (number > fileNameList.MaxFileNumber())
        {
            number = fileNameList.MaxFileNumber();
            RecalculateXyFromCursor();
        }
    }

    
    void RecalculateCursorFromXY()
    {
        number = y*numColumns + x;
    }

    void RecalculateXyFromCursor()
    {
        y = number / numColumns;
        x = number - y*numColumns;
    }

    void SetTo(int n)
    {
        previous = number;
        number = n;
        RecalculateXyFromCursor();
    }

    void SetupRedraw(SortedVectorInts &redrawSet)
    {
        redrawSet.Clear();
        redrawSet.AddSingle(number);
        redrawSet.AddSingle(previous);
    }

    int GetNumber() { return number; }
    int GetX()      { return      x; }
    int GetY()      { return      y; }

 
private:
    int     number;
    int     previous;
    int     x, y;

    int             &numColumns;
    FileNameList    &fileNameList;
};



// Loads a single thumbnail, then quits.
class ThumbnailLoader : public wxThread
{
public:
    ThumbnailLoader(wxString fn, Thumbnail &tn)
    : wxThread(wxTHREAD_DETACHED),
      fileName(fn),
      thumbnail(tn)
    {
    }

protected:
    ExitCode Entry();

    wxString            fileName;
    Thumbnail          &thumbnail;
	jpeg_load_state		jpegLoadState;
};
 

// Reads the headers of all of the images, to get the image sizes.
class ThumbnailHeaderReader : public wxThread
{
public:
    ThumbnailHeaderReader(ThumbnailCanvas &tnc)
    : wxThread(wxTHREAD_DETACHED),
      thumbnailCanvas(tnc)
    {
    }


protected:
    ExitCode Entry();

    ThumbnailCanvas        &thumbnailCanvas;
};




class Thumbnail : public wxObject
{
public:
	Thumbnail(const wxPoint &pos, wxFileName filename, bool fetchHeader=false);
	~Thumbnail();

    void StartLoadingImage() { if (thumbnailLoader) thumbnailLoader->Run(); }

	void Draw(wxPaintDC &dc, bool selected = false, bool cursor = false, bool inFocus = true);

    static void   SetSize(wxSize size)                     { tnSize           = size;   }
    static wxSize GetSize()                                 { return tnSize; }
    static void   SetSelectBorder(int border)              { selectBorderSize = border; }
    static void   SetLabelHeight(int height)               { labelHeight      = height; }
    static void   SetBackgroundColor(const wxColor &color) { backgroundColor  = color;  }

    void CreateGenericIcon();

    void FetchHeader();
    void SetPosition(const wxPoint &pos)                 { position = pos; }
    void Erase(wxPaintDC &dc);
    bool IsMouseInside(const wxPoint &mousePos);

	wxSize GetTnImageSize(wxSize imageSize)
	{
		float toWidth = (float)tnSize.GetWidth() / (float)imageSize.GetWidth();
		float toHeight = (float)tnSize.GetHeight() / (float)imageSize.GetHeight();
		float scale;

		if (toWidth <= toHeight)	scale = toWidth;
		else						scale = toHeight;

		int newXsize = imageSize.GetWidth()  * scale;
		int newYsize = imageSize.GetHeight() * scale;

		if (newXsize < 1)	newXsize = 1;
		if (newYsize < 1)	newYsize = 1;

		return wxSize(newXsize, newYsize);
	}

    bool ImageIsLoaded()       { return imageLoaded; }
    bool HasBeenDrawn()        { return hasBeenDrawn;  }
    wxFileName  GetFullPath()  { return fullPath; }
    wxSize      GetImageSize() { return imageSize; }

    wxPoint GetPosition() { return position; }

    void SetImage(wxImage &image);
    void SetImage(wxBitmap &bm) { bitmap = bm; }
    wxBitmap& GetBitmapReference() { return bitmap; }

    //void SetImageSize(wxSize size);
    void FinishedLoading()  { imageLoaded = true; }
    friend class ThumbnailLoader;
    //friend class ThumbnailCanvas;
protected:

    void DrawLabelClipped(wxPaintDC &dc, wxString &label, wxRect &rectangle);

    bool                imageLoaded;
    bool                hasBeenDrawn;
    bool                isLoading;
	wxBitmap            bitmap;
    wxFileName          fullPath;
    wxDateTime          dateTime;
    wxSize              imageSize;
    wxSize              imageSizeTemp;
    ThumbnailLoader    *thumbnailLoader;
    wxPoint             position;
    static wxSize       tnSize;
    static int          selectBorderSize;
    static int          labelHeight;
    static wxColor      backgroundColor;
    static wxArrayInt   arrayIntStatic;             // This is used when rendering labels. Two thumbnails never use at the same time.
};



class ThumbnailCanvas : public wxScrolledWindow
{
    enum DRAG_STATE
    {
        TNC_DRAG_STATE_NONE     = 0,
        TNC_DRAG_STATE_CLICKED  = 1,
        TNC_DRAG_STATE_DRAGGING = 2
    };

    enum REDRAW_TYPE
    {
        REDRAW_ALL       = 0,
        REDRAW_SELECTION = 1

    };

public:
	ThumbnailCanvas(ImageBrowser *imgBrs, wxWindow *parent, wxWindowID, const wxPoint &pos, const wxSize &size);
	~ThumbnailCanvas();

	void OnPaint(wxPaintEvent &event);
	void CreateAntiAliasedBitmap();
	void ClearThumbnails();
	void LoadThumbnails(wxString directory);
    void UnLoadThumbnails(wxString directory);
    void ReLoadThumbnails();
    void StopLoadingThumbnails(wxString directory);

    //void KillAllThreads();
    void DirectoryWasDeleted(wxString path);

	void OnSize(wxSizeEvent &event);
	//void Paint();
    void OnMouseEvent(wxMouseEvent &event);
    void OnKeyEvent(wxKeyEvent &event);
    void OnIdle(wxIdleEvent &event);
    void OnMouseWheel(wxMouseEvent& event);

    void OnFocusEvent(wxFocusEvent &event);
    void OnFocusKillEvent(wxFocusEvent &event);

    void OnDropFiles(wxDropFilesEvent& event);

    int     GetThumbnailFromPosition(wxPoint &position);
    wxPoint GetThumbnailPosition(size_t n);

    void SetCursorTo(int n);

    void RecalculateRowsCols();
    bool HandleThumbnailLoading();
    void OnClose(wxCloseEvent &event);

    void HideImageViewer();

    void SetImageViewer(ImageViewer *iv);
    void SetAcceleratorTable(const wxAcceleratorTable &accel);
    void UpdateStatusBar_File();
    void ClearStatusBar();
    void SetCursor(int imageNumber);

    void RemoveThumbNailFromCanvas(int tn);
    void DeleteImage(int tn);
    void DeleteSelection();

    void ActivateThumbnail(int n);

    Thumbnail* GetThumbnail(int i)      { return &thumbnails[i];      }
    int        GetNumThumbnails()       { return  thumbnails.size();  }
    int        GetNumColumns()          { return  tnColumns;          }
    int        GetNumRows()             { return  int(GetSize().GetHeight() / (tnSize.GetHeight() + tnSpacingY)); }
    int        GetNumImagesPerPage()    { return  tnColumns * GetNumRows(); }

    void ReadHeadersCompleted() { readHeadersCompleted = true; }

private:
    void HandleCursorScrolling();

    //void ReportInt1(int pos, wxString str, int i);
    //void ReportInt2(int pos, wxString str, int i, int j);
    //void ReportInt3(int pos, wxString str, int i, int j, int k);
    //
    //void MoveCursor(int dx, int dy);

    void FindNearestThumbnail();

	std::vector<Thumbnail>	thumbnails;
    std::vector<int>	    thumbnailIndex;
    int                     selectionStart;
	bool                    inFocus;

    TnCursor                cursorP;
    SortedVectorInts        selectionSetP;                      // Selected thumbnails (pointers)
    SortedVectorInts        redrawSetP;                         // Thumbnails that need to be redrawn
    SortedVectorInts        waitingSet;                         // Thumbnails that need to be requested to load their images
    SortedVectorInts        loadingSet;                         // Thumbnails that are currently loading

    int                     maxLoading;
    int                     tnColumns, tnRows;
    wxColor                 backgroundColor;
	wxSize                  tnSize;
	wxSize                  mySize;
	int                     thBorder;
	FileNameList            fileNameList;
    DRAG_STATE              dragState;
    REDRAW_TYPE             redrawType;
    wxPoint                 clickPoint;
    SortedVectorInts        draggingSet;
    int                     draggedThumb;
    int                     tnSpacingX, tnSpacingY;
    wxULongLong             totalDirectorySizeBytes;
     
    ImageBrowser           *imageBrowser;
    ImageViewer            *imageViewer;
    //ConfigParser           *configParser;
    bool                    readHeadersCompleted;
    wxFileDataObject       *dragingFilesDataObject;

	wxDECLARE_EVENT_TABLE();
};
