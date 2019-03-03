
#include "vector_renderer.h"
#include "wx/image.h"
#include "wx/tokenzr.h"
#include "wx/bitmap.h"
#include "wx/dc.h"
#include "wx/dcmemory.h"

#include <iostream>
using namespace std;

wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Tahoma");

VectorRendererInterpreter::VectorRendererInterpreter(wxString program)
{
    wxStringTokenizer tokenizer(program, " ,()");
    while (tokenizer.HasMoreTokens())
    {
        wxString token = tokenizer.GetNextToken();
        //cout << token << endl;
        arrayStr.Add(token);
    }

    i = 0;
}

int      VectorRendererInterpreter::GetInt()
{
    long l;
    arrayStr[i++].ToLong(&l);

    return l;
}

double   VectorRendererInterpreter::GetReal()
{
    double d;
    arrayStr[i++].ToDouble(&d);

    return d;
}

wxString VectorRendererInterpreter::GetString()
{
    return arrayStr[i++];
}

void VectorRenderer::Render(wxString &program, wxBitmap &bitmap)
{
    VectorRendererInterpreter interpreter(program);
    bool ending = false;
    char command;
    int x1, y1, x2, y2;
    int h, w;
    wxColour colourFG;
    wxColour colourBG;
    int r, g, b;
    wxString text;
    wxRect rect;

    double   xSize = bitmap.GetSize().x;
    double   ySize = bitmap.GetSize().y;
    wxMemoryDC dc(bitmap);

    while(!ending)
    {
        command = interpreter.GetString()[0];

        switch (command)
        {
        case'P':
            r = interpreter.GetInt();
            g = interpreter.GetInt();
            b = interpreter.GetInt();
            w = interpreter.GetInt();
            colourFG = wxColour(r, g, b);
            dc.SetPen(wxPen(colourFG, w));
            dc.SetTextForeground(wxColour(r, g, b));
            //cout << "Pen " << r << ", " << g << ", " << b << endl;
            break;

        case'B':
            r = interpreter.GetInt();
            g = interpreter.GetInt();
            b = interpreter.GetInt();
            colourBG = wxColour(r, g, b);
            dc.SetBrush(colourBG);
            //cout << "Brush " << r << ", " << g << ", " << b << endl;
            break;

        case 'L':
            x1 = int(interpreter.GetReal() * xSize);
            y1 = int(interpreter.GetReal() * ySize);
            x2 = int(interpreter.GetReal() * xSize);
            y2 = int(interpreter.GetReal() * ySize);

            dc.DrawLine(x1,y2, x2,y2);
            break;

        case 'R':
            x1 = int(interpreter.GetReal() * xSize);
            y1 = int(interpreter.GetReal() * ySize);
            w  = int(interpreter.GetReal() * xSize);
            h  = int(interpreter.GetReal() * ySize);

            dc.DrawRectangle(x1, y1, w, h);
            //cout << "Rectangle " << x1 << ", " << y1 << ", " << h << ", " << w << endl;
            break;

        case 'G':
            x1 = int(interpreter.GetReal() * xSize);
            y1 = int(interpreter.GetReal() * ySize);
            w = int(interpreter.GetReal() * xSize);
            h = int(interpreter.GetReal() * ySize);

            rect = wxRect(x1, y1, w, h);
            dc.GradientFillConcentric(rect, colourFG, colourBG);
            //cout << "Rectangle " << x1 << ", " << y1 << ", " << h << ", " << w << endl;
            break;

        case 'T':
            x1 = int(interpreter.GetReal() * xSize);
            y1 = int(interpreter.GetReal() * ySize);
            text = interpreter.GetString();
            dc.SetFont(font);
            dc.DrawText(text, wxPoint(x1, y1));
            //cout << "Text " << x1 << ", " << y1 << ", " << text << endl;
            break;

        case 'X':
            ending = true;
            break;
        }
    }

}


