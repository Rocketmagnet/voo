
#include "video_thumbnail_reader.h"

#include <windows.h>
#include <windowsx.h>

// Media Foundation 
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <assert.h>

#include <fstream>

#include <propvarutil.h>
#include <iostream>
using namespace std;

const LONGLONG SEEK_TOLERANCE     = 10000000;
const LONGLONG MAX_FRAMES_TO_SKIP = 10;

#include <fstream>

bool VideoThumbnailReader::mfStarted = false;
char *bitmap = 0;

bool Inside(int xx, int yy, int x1, int y1, int x2, int y2)
{
    if (xx <  x1)   return false;
    if (xx >= x2)   return false;

    if (yy <  y1)   return false;
    if (yy >= y2)   return false;

    return true;
}


std::ifstream::pos_type filesize(const char* filename)
{
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return in.tellg();
}

bool write_32bit_file_binary(std::string const & filename, char const * data, size_t const bytes)
{
    //cout << "Writing " << filename.c_str() << "  " << bytes << " bytes" << endl;
    std::ofstream b_stream(filename.c_str(), std::fstream::out | std::fstream::binary);
    const char *d = data;
    size_t n = bytes;

    if (b_stream)
    {
        while (n)
        {
            b_stream.write(d, 3);
            d += 4;
            n -= 4;
        }
        return (b_stream.good());
    }

    //if (b_stream)
    //{
    //    b_stream.write(data, bytes);
    //    return (b_stream.good());
    //}

    return false;
}

bool write_24bit_file_binary(std::string const & filename, char const * data, size_t const bytes)
{
    std::ofstream b_stream(filename.c_str(), std::fstream::out | std::fstream::binary);
    const char *d = data;
    size_t n = bytes;

    if (b_stream)
    {
        b_stream.write(d, bytes);
        return (b_stream.good());
    }

    //if (b_stream)
    //{
    //    b_stream.write(data, bytes);
    //    return (b_stream.good());
    //}

    return false;
}

VideoThumbnailReader::VideoThumbnailReader()
: m_pReader(0)
{
    m_format  = new VideoFormatInfo;

    if (!mfStarted)
    {
        int hr;
        hr = MFStartup(MF_VERSION);
    }
}

VideoThumbnailReader::~VideoThumbnailReader()
{
    SafeRelease(&m_pReader);
    delete m_format;
}

int VideoThumbnailReader::SelectVideoStream()
{
    int hr = S_OK;

    IMFMediaType *pType = NULL;

    // Configure the source reader to give us progressive RGB32 frames.
    // The source reader will load the decoder if needed.

    hr = MFCreateMediaType(&pType);

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    }

    if (SUCCEEDED(hr))
    {
        hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
    }


    if (SUCCEEDED(hr))
    {

        hr = m_pReader->SetCurrentMediaType( (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, pType );
    }
    // Ensure the stream is selected.
    //if (SUCCEEDED(hr))
    {
        hr = m_pReader->SetStreamSelection( (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, TRUE );
    }

    if (SUCCEEDED(hr))
    {
        hr = GetVideoFormat(m_format);
    }

    SafeRelease(&pType);
    return hr;
}

RECTANGLE CorrectAspectRatio(const RECTANGLE& src, const MFRatio& srcPAR)
{
    // Start with a rectangle the same size as src, but offset to the origin (0,0).
    RECTANGLE rc(0, 0, src.right - src.left, src.bottom - src.top);

    if ((srcPAR.Numerator != 1) || (srcPAR.Denominator != 1))
    {
        // Correct for the source's PAR.

        if (srcPAR.Numerator > srcPAR.Denominator)
        {
            // The source has "wide" pixels, so stretch the width.
            rc.right = MulDiv(rc.right, srcPAR.Numerator, srcPAR.Denominator);
        }
        else if (srcPAR.Numerator < srcPAR.Denominator)
        {
            // The source has "tall" pixels, so stretch the height.
            rc.bottom = MulDiv(rc.bottom, srcPAR.Denominator, srcPAR.Numerator);
        }
        // else: PAR is 1:1, which is a no-op.
    }
    return rc;
}

int VideoThumbnailReader::GetVideoFormat(VideoFormatInfo *pFormat)
{
    int hr = S_OK;
    UINT32  width = 0, height = 0;
    LONG lStride = 0;
    MFRatio par = { 0 , 0 };

    VideoFormatInfo& format = *pFormat;

    GUID subtype = { 0 };

    IMFMediaType *pType = NULL;

    // Get the media type from the stream.
    hr = m_pReader->GetCurrentMediaType(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        &pType
    );

    if (FAILED(hr)) { goto done; }

    // Make sure it is a video format.
    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (subtype != MFVideoFormat_RGB32)
    {
        hr = E_UNEXPECTED;
        goto done;
    }

    // Get the width and height
    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);

    if (FAILED(hr)) { goto done; }

    // Get the stride to find out if the bitmap is top-down or bottom-up.
    lStride = (LONG)MFGetAttributeUINT32(pType, MF_MT_DEFAULT_STRIDE, 1);

    format.bTopDown = (lStride > 0);

    // Get the pixel aspect ratio. (This value might not be set.)
    hr = MFGetAttributeRatio(pType, MF_MT_PIXEL_ASPECT_RATIO, (UINT32*)&par.Numerator, (UINT32*)&par.Denominator);
    if (SUCCEEDED(hr) && (par.Denominator != par.Numerator))
    {
        RECTANGLE rcSrc(0, 0, width, height);

        format.rcPicture = CorrectAspectRatio(rcSrc, par);
    }
    else
    {
        // Either the PAR is not set (assume 1:1), or the PAR is set to 1:1.
        format.rcPicture.SetValues(0, 0, width, height);
    }

    format.imageWidthPels = width;
    format.imageHeightPels = height;

done:
    SafeRelease(&pType);

    return hr;
}

int VideoThumbnailReader::OpenFile(const wchar_t* wszFileName)
{
    wstring ws(wszFileName);
    string str(ws.begin(), ws.end());

    int hr = S_OK;

    IMFAttributes *pAttributes = NULL;

    SafeRelease(&m_pReader);

    // Configure the source reader to perform video processing.
    //
    // This includes:
    //   - YUV to RGB-32
    //   - Software deinterlace

    hr = MFCreateAttributes(&pAttributes, 1);

    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    }

    // Create the source reader from the URL.

    if (SUCCEEDED(hr))
    {
        hr = MFCreateSourceReaderFromURL(wszFileName, pAttributes, &m_pReader);
    }

    if (SUCCEEDED(hr))
    {
        // Attempt to find a video stream.
        hr = SelectVideoStream();
    }

    return hr;
}


void  VideoThumbnailReader::CreateBitmap(char *imageData, int width, int height, long long& hnsPos)
{
    int             hr           = S_OK;
    unsigned long   dwFlags      = 0;

    unsigned char  *pBitmapData  = NULL;    // Bitmap data
    unsigned long   cbBitmapData = 0;       // Size of data, in bytes
    long long       hnsTimeStamp = 0;
    bool            bCanSeek     = FALSE;       // Can the source seek?  
    int             cSkipped     = 0;           // Number of skipped frames

    IMFMediaBuffer *pBuffer      = 0;
    IMFSample      *pSample      = NULL;

    CanSeek(&bCanSeek);

    if (!bCanSeek)
    {
        cout << "Cannot seek" << endl;
        return;
    }

    if (bCanSeek && (hnsPos > 0))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        var.vt = VT_I8;
        var.hVal.QuadPart = hnsPos;

        hr = m_pReader->SetCurrentPosition(GUID_NULL, var);

        if (FAILED(hr))
        {
            cout << "failed 1" << endl;
            goto done;
        }

    }


    // Pulls video frames from the source reader.

    // NOTE: Seeking might be inaccurate, depending on the container
    //       format and how the file was indexed. Therefore, the first
    //       frame that we get might be earlier than the desired time.
    //       If so, we skip up to MAX_FRAMES_TO_SKIP frames.

    while (1)
    {
        IMFSample *pSampleTmp = NULL;

        hr = m_pReader->ReadSample( (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                    0,
                                    NULL,
                                    (DWORD*)&dwFlags,
                                    NULL,
                                    &pSampleTmp );

        if (FAILED(hr))
        {
            goto done;
        }

        if (dwFlags & MF_SOURCE_READERF_ENDOFSTREAM)
        {
            break;
        }

        if (dwFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
        {
            // Type change. Get the new format.
            hr = GetVideoFormat(m_format);
            if (FAILED(hr))
            {
                goto done;
            }
        }

        if (pSampleTmp == NULL)
        {
            continue;
        }

        // We got a sample. Hold onto it.

        SafeRelease(&pSample);

        pSample = pSampleTmp;
        pSample->AddRef();

        if (SUCCEEDED(pSample->GetSampleTime(&hnsTimeStamp)))
        {
            // Keep going until we get a frame that is within tolerance of the
            // desired seek position, or until we skip MAX_FRAMES_TO_SKIP frames.

            // During this process, we might reach the end of the file, so we
            // always cache the last sample that we got (pSample).

            if ((cSkipped < MAX_FRAMES_TO_SKIP) &&
                (hnsTimeStamp + SEEK_TOLERANCE < hnsPos))
            {
                SafeRelease(&pSampleTmp);

                ++cSkipped;
                continue;
            }
        }

        SafeRelease(&pSampleTmp);

        hnsPos = hnsTimeStamp;
        break;
    }
    //cout << "Timestamp = " << hnsPos << endl;

    if (pSample)
    {
        UINT32 pitch = 4 * m_format->imageWidthPels;

        // Get the bitmap data from the sample, and use it to create a
        // Direct2D bitmap object. Then use the Direct2D bitmap to 
        // initialize the sprite.

        hr = pSample->ConvertToContiguousBuffer(&pBuffer);

        if (FAILED(hr))
        {
            cout << "ConvertToContiguousBuffer failed" << endl;
            goto done;
        }

        hr = pBuffer->Lock(&pBitmapData, NULL, &cbBitmapData);

        //cout << "Size of data = " << cbBitmapData << endl;
        bitmap = new char[cbBitmapData];

        //write_32bit_file_binary("out.raw", (const char*)pBitmapData, cbBitmapData);

        int sHeight = m_format->imageHeightPels;
        int sWidth  = m_format->imageWidthPels;
        int dWidth = width;
        int dHeight = height;

        //cout << "A" << endl;

        double sAspect = (double)sWidth / (double)sHeight;
        double dAspect = (double)dWidth / (double)dHeight;
        int sx1, sx2, sy1, sy2;     // Source coordinates
        int dx1, dx2, dy1, dy2;         // Destination coordinates

        //cout << "B" << endl;
        if (sAspect > dAspect)        // Source is wider than destination
        {                                       // Touching at side
            sx1 = 0;
            sx2 = sWidth;

            sy2 = sHeight;
            sy1 = 0;

            dx1 = 0;
            dx2 = width;
            dy1 = 0;
            dy2 = sHeight * dWidth / sWidth;
            
            newWidth = dx2;
            newHeight = dy2;

            cout << sx1 << ", " << sy1 << ", " << sx2 << ", " << sy2 << endl;
            cout << dx1 << ", " << dy1 << ", " << dx2 << ", " << dy2 << endl;
        }
        else                                    // Destination is wider than source
        {                                       // Touching at top/bottom
            cout << "Aspect 2 " << sWidth << ", " << sHeight << endl;
            //cout << "C" << endl;
        }

        //int i = 3*width * (-y1 * height / (m_format->imageHeightPels*2));
        int i;

        for (int y=dy1; y<dy2; y++)
            for (int x = dx1; x<dx2; x++)
            {
                i = 3*(y * dWidth + x);

                int xx = x * (sWidth) / dx2;
                int yy = y * (sHeight) / dy2;
                int j = yy * sWidth + xx;
                //cout << x << ", " << y << "    " << xx << ", " << yy << endl;
                //int xx = dx1 + x * (dx2-dx1)  / width;
                //int yy = dy1 + y * (dy2-dy1) / height;
                //int j = yy * (dx2-dx1) + xx;
                j *= 4;

                if (Inside(xx, yy, 0,0, m_format->imageWidthPels, m_format->imageHeightPels))
                {
                    imageData[i++] = pBitmapData[j+2];
                    imageData[i++] = pBitmapData[j+1];
                    imageData[i++] = pBitmapData[j];
                    j += 3;
                }
            }

        //write_24bit_file_binary("out2.raw", (const char*)imageData, width*height*3);
    }
    else
    {
        hr = MF_E_END_OF_STREAM;
    }

done:

    if (pBitmapData)
    {
        pBuffer->Unlock();
    }
    SafeRelease(&pBuffer);
    SafeRelease(&pSample);
    //SafeRelease(&pBitmap);

    //return hr;
}


int VideoThumbnailReader::CanSeek(bool *pbCanSeek)
{
    int hr = S_OK;

    ULONG flags = 0;

    PROPVARIANT var;
    PropVariantInit(&var);

    if (m_pReader == NULL)
    {
        return MF_E_NOT_INITIALIZED;
    }

    *pbCanSeek = FALSE;

    hr = m_pReader->GetPresentationAttribute( (DWORD)MF_SOURCE_READER_MEDIASOURCE,
                                              MF_SOURCE_READER_MEDIASOURCE_CHARACTERISTICS,
                                              &var );

    if (SUCCEEDED(hr))
    {
        hr = PropVariantToUInt32(var, &flags);
    }

    if (SUCCEEDED(hr))
    {
        // If the source has slow seeking, we will treat it as
        // not supporting seeking. 

        if ((flags & MFMEDIASOURCE_CAN_SEEK) &&
            !(flags & MFMEDIASOURCE_HAS_SLOW_SEEK))
        {
            *pbCanSeek = TRUE;
        }
    }

    return hr;
}
