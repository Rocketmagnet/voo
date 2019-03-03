#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "wx/glcanvas.h"
#include "drawable.h"
#include "freetype_fonts.h"

#define GL_PANEL_BLANK_SCREEN   true
#define GL_PANEL_RENDER_IMAGE   false

class FileNameList;

class BasicGLPanel : public wxGLCanvas
{

public:
    BasicGLPanel(wxFrame* parent, int* args);

    void SetFileNameList(FileNameList *fileNameList) { imageServer.SetFileNameList(fileNameList); }
    void Resized(wxSizeEvent& evt);

    int GetWidth();
    int GetHeight();

    void OnPaint(wxPaintEvent& evt);
    void Prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void Prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);

    void Clear();
    void ClearCache();

    //void DisplayImage(wxString filename);
    void DisplayImage(int imageNumber);

    // events
    void MouseMoved(wxMouseEvent& event);
    void MouseDown(wxMouseEvent& event);
    void MouseWheelMoved(wxMouseEvent& event);
    void MouseReleased(wxMouseEvent& event);
    void RightClick(wxMouseEvent& event);
    void MouseLeftWindow(wxMouseEvent& event);
    void KeyPressed(wxKeyEvent& event);
    void KeyReleased(wxKeyEvent& event);
    void Render(bool blankScreen);

    void ZoomIn();
    void ZoomOut();
    void MoveRel(float x, float y);
    
    void LoadFont(const wxFileName & fileName, FreetypeFont & font, int fontResolution = 0);

    int GetImageNumber() { return currentImageNumber; }
	wxString GetImageNumberInfo();

    wxGLContext        *m_context;

    GL_ImageServer      imageServer;
    GL_Image           *currentImage;
    int                 currentImageNumber;
    int                 imageNumberToLoad;
    float               scaling;
    bool                glewInitialised;
    FreetypeFont	    screenFont;
    int                 fontTimeRemaining;
    int                 zoomTimeRemaining;
    DECLARE_EVENT_TABLE()
};

#endif
