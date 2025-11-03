#ifndef _glpane_
#define _glpane_

#include "wx/wx.h"
#include "drawable.h"
#include "freetype_fonts.h"

#include "wx/glcanvas.h"

#define GL_PANEL_BLANK_SCREEN   true
#define GL_PANEL_RENDER_IMAGE   false

class FileNameList;
class ThumbnailCanvas;

//class JpegGpu;

class BasicGLPanel : public wxGLCanvas
{

public:
    BasicGLPanel(wxFrame* parent, int* args);
    ~BasicGLPanel();

    void SetFileNameList(FileNameList *fileNameList) { imageServer.SetFileNameList(fileNameList); }
    void Resized(wxSizeEvent& evt);
    void SetThumbnailCanvas(ThumbnailCanvas* _thumbnailCanvas)
    {
        imageServer.SetThumbnailCanvas(_thumbnailCanvas);
        thumbnailCanvas = _thumbnailCanvas;
    }
    int  GetWidth();
    int  GetHeight();

    void OnPaint(wxPaintEvent& evt);
    void Prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);
    void Prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y);

    void Clear();
    void ClearCache();

    //void DisplayImage(wxString filename);
    void DisplayImage(wxFileName fileName);

    // events
    void MouseMoved(wxMouseEvent& event);
    void MouseDown(wxMouseEvent& event);
    void MouseWheelMoved(wxMouseEvent& event);
    void MouseReleased(wxMouseEvent& event);
    void RightClick(wxMouseEvent& event);
    void MouseLeftWindow(wxMouseEvent& event);
    void MouseDClick(wxMouseEvent &event);
    void KeyPressed(wxKeyEvent& event);
    void KeyReleased(wxKeyEvent& event);
    void Render(bool blankScreen);

    void ZoomIn();
    void ZoomOut();
    void MoveRel(float x, float y);
    
    void InitFont(FreetypeFont & font, int fontResolution = 0);

    wxFileName GetImageFileName() { return currentImageFileName; }
	wxString GetImageNumberInfo();

    wxGLContext        *m_context;

    GL_ImageServer      imageServer;
    GL_Image           *currentImage;
    wxFileName          currentImageFileName;
    wxFileName          imageToLoad;
    float               scaling;
    bool                glewInitialised;
    FreetypeFont	    screenFont;
    int                 fontTimeRemaining;
    int                 zoomTimeRemaining;
    ThumbnailCanvas    *thumbnailCanvas;

    DECLARE_EVENT_TABLE()
};

#endif
