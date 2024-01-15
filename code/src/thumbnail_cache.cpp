#include "thumbnail_cache.h"
#include <iostream>

using namespace std;

wxString MakeThumbnailFileName(wxFileName path)
{
    wxFileName thumbFilename = path;
    thumbFilename.SetName(thumbFilename.GetName() + "_" + thumbFilename.GetExt());
    thumbFilename.AppendDir("Thumbs");
    thumbFilename.SetExt("jpg");
    return thumbFilename.GetFullPath();
}

void ThumbnailCache::CacheImage(wxImage* image, wxString fullPath)
{
    wxFileName imagePath(fullPath);

    if (imagePath.GetPath() != currentPath.GetPath())
    {
        std::cout << imagePath.GetPath() << " is a new path" << std::endl;
        // This is the first thumbnail of a new path.
        numThumbnailsInThisPath = 1;
        currentPath = imagePath;
        savedImage = image->Copy();
    }
    else
    {
        std::cout << imagePath.GetPath() << " is a multi-video directory." << std::endl;
        // This is another image in the same path. Let's start saving them.
        
        wxString thumbPath;
        if (numThumbnailsInThisPath == 1)
        {
            thumbPath = MakeThumbnailFileName(currentPath);
            wxFileName(thumbPath).Mkdir();
            std::cout << "Saving1 = " << thumbPath << std::endl;
            savedImage.SaveFile(thumbPath, wxBITMAP_TYPE_JPEG);
        }

        thumbPath = MakeThumbnailFileName(imagePath);
        std::cout << "Saving2 = " << thumbPath << std::endl;
        image->SaveFile(thumbPath, wxBITMAP_TYPE_JPEG);
    }
}

