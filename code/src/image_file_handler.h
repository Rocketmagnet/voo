#pragma once
#include "wx/wx.h"

enum ImageFileHandlerReturn
{
	DO_NOTHING   = 0,
    LOAD_IMAGE   = 1,
    REFRESH_TREE = 2,
    DELETE_FILE  = 4
};

class Thumbnail;

class ImageFileHandler
{
public:

    ImageFileHandler()
    {}

    virtual ~ImageFileHandler()
    {}

    virtual bool LoadThumbnail(wxString fileName, Thumbnail &thumbnail) = 0;
    virtual int  LoadImage(    wxString fileName) = 0;

    int a;
};
