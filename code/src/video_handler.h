#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"
#include "video_thumbnail_reader.h"
#include "wx/filename.h"

class VideoHandler : public ImageFileHandler
{
public:
    VideoHandler()
    {}

    static void SetVideoPlayer(wxFileName _playerPath)
    {
        playerPath = _playerPath;
    }

    bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail);
    int  LoadImage(wxString fileName);

private:
    VideoThumbnailReader videoThumbnailReader;

    static wxFileName playerPath;
};
