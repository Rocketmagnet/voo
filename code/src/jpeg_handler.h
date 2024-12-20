#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"

extern "C"
{
#include "jpeg_turbo.h"
};

class wxImage;

class JpegHandler : public ImageFileHandler
{
public:
    JpegHandler()
    {}

    bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail);
    int  LoadImage(wxString fileName);

private:
    void ConvertGreyscaleToRGB(wxImage &image);
    int a;
    jpeg_load_state     jpegLoadState;
};
