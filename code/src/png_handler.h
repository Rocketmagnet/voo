#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"



class PngHandler : public ImageFileHandler
{
public:
    PngHandler()
    {}

    bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail);
    int  LoadImage(wxString fileName);

private:
    int a;
};
