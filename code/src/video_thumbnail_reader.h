#pragma once


template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = 0;
    }
}

struct RECTANGLE
{
    RECTANGLE()
    : left(0),
      right(0),
      top(0),
      bottom(0)
    {}

    RECTANGLE(int l, int r, int t, int b)
    : left(l),
      right(r),
      top(t),
      bottom(b)
    {}

    void SetValues(int l, int r, int t, int b)
    {
        left = l;
        right = r;
        top = t;
        bottom = b;

    }

    int    left;
    int    top;
    int    right;
    int    bottom;
};

struct VideoFormatInfo
{
    int             imageWidthPels;
    int             imageHeightPels;
    bool            bTopDown;
    RECTANGLE       rcPicture;    // Corrected for pixel aspect ratio

    VideoFormatInfo()
    : imageWidthPels(0),
      imageHeightPels(0),
      bTopDown(false)
    {
    }
};

class IMFSourceReader;
class wxImage;

class VideoThumbnailReader
{
private:

    IMFSourceReader *m_pReader;
    VideoFormatInfo *m_format;

public:

    VideoThumbnailReader();
    ~VideoThumbnailReader();


    int     OpenFile(const wchar_t* wszFileName);
    int     GetDuration(long long *phnsDuration);

    void    CreateBitmap(char *imageData, int width, int height, long long& hnsPos);
    int     GetNewWidth()  { return newWidth;  }
    int     GetNewHeight() { return newHeight; }

private:
    static  bool mfStarted;
    int     CanSeek(bool *pbCanSeek);
    int     SelectVideoStream();
    int     GetVideoFormat(VideoFormatInfo *pFormat);

    int     newWidth, newHeight;
};

