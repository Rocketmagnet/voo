#ifndef VECTOR_RENDERER_H_INCLUDED
#define VECTOR_RENDERER_H_INCLUDED

#include "wx/arrstr.h"

class wxString;
class wxBitmap;

class VectorRendererInterpreter
{
public: 
    VectorRendererInterpreter(wxString program);

    int      GetInt();
    double   GetReal();
    wxString GetString();

private:
    wxArrayString       arrayStr;
    int                 i;
};


class VectorRenderer
{
public:
    VectorRenderer()
    {

    }


    void Render(wxString &program, wxBitmap &bitmap);
};

#endif

