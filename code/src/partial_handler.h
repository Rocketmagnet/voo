#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"



class PartialHandler : public ImageFileHandler
{
public:
    PartialHandler()
    {}

    bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail);
    int  LoadImage(wxString fileName);

private:
    int a;
};
