#ifndef IMAGE_SERVER_H_INCLUDED
#define IMAGE_SERVER_H_INCLUDED

#include "wx/image.h"
#include "wx/filename.h"
#include <iostream>

class ThumbnailCache
{
public:
    ThumbnailCache()
    : currentPath(""),
      numThumbnailsInThisPath(0)
    {}

    static ThumbnailCache& Instance()
    {
        static ThumbnailCache thumbnailCache;
        return thumbnailCache;
    }

    void CacheImage(wxImage* image, wxString fullPath);
    void   GetImage(wxImage *image, wxString fullPath);

private:

    wxFileName      currentPath;
    int             numThumbnailsInThisPath;
    wxImage         savedImage;
    //SafeQueue
};



#endif
