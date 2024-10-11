#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"
#include "video_thumbnail_reader.h"


class VideoHandler : public ImageFileHandler
{
public:
    VideoHandler()
    {}

    bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail);
    int  LoadImage(wxString fileName);

private:
    VideoThumbnailReader videoThumbnailReader;
};
