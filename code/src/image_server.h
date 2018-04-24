#ifndef IMAGE_SERVER_H_INCLUDED
#define IMAGE_SERVER_H_INCLUDED

#include "wx/image.h"

class ImageServer
{
public:
    ImageServer();

    wxImage GetImage(wxString fullPath);

};



#endif
