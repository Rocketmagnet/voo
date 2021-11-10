#include <gl/glew.h>
#include "wx/wx.h"
#include "wx/sizer.h"
#include "wx/glcanvas.h"

#include "drawable.h"
#include "gl_panel.h"
#include <GL/glu.h>
#include <GL/gl.h>
#include <iostream>
using namespace std;


extern void NoteTime(wxString s);


BEGIN_EVENT_TABLE(BasicGLPanel, wxGLCanvas)
    //EVT_MOTION(      BasicGLPanel::MouseMoved)
    //EVT_LEFT_DOWN(   BasicGLPanel::MouseDown)
    //EVT_LEFT_UP(     BasicGLPanel::MouseReleased)
    //EVT_LEFT_DCLICK( BasicGLPanel::MouseDClick)
    //EVT_RIGHT_DOWN(  BasicGLPanel::RightClick)
    //EVT_LEAVE_WINDOW(BasicGLPanel::MouseLeftWindow)
    EVT_SIZE(        BasicGLPanel::Resized)
    //EVT_MOUSEWHEEL(  BasicGLPanel::MouseWheelMoved)
    EVT_PAINT(       BasicGLPanel::OnPaint)
END_EVENT_TABLE()


// some useful events to use
void BasicGLPanel::MouseMoved(wxMouseEvent& event)      { cout << "BasicGLPanel::MouseMoved()\n"; event.Skip();}
void BasicGLPanel::MouseDown(wxMouseEvent& event)       { cout << "BasicGLPanel::MouseDown()\n"; event.Skip();}
void BasicGLPanel::MouseWheelMoved(wxMouseEvent& event) { cout << "BasicGLPanel::MouseWheelMoved()\n"; event.Skip();}
void BasicGLPanel::MouseReleased(wxMouseEvent& event)   { cout << "BasicGLPanel::MouseReleased()\n"; event.Skip();}
void BasicGLPanel::RightClick(wxMouseEvent& event)      { cout << "BasicGLPanel::RightClick()\n"; event.Skip();}
void BasicGLPanel::MouseLeftWindow(wxMouseEvent& event) { cout << "BasicGLPanel::MouseLeftWindow()\n"; event.Skip();}
void BasicGLPanel::MouseDClick(wxMouseEvent& event)     { cout << "BasicGLPanel::MouseDClick()\n"; event.Skip(); }

BasicGLPanel::BasicGLPanel(wxFrame* parent, int* args)
: wxGLCanvas(parent, -1, 0, wxDefaultPosition, wxDefaultSize, 0, wxT("GLCanvas")),
  scaling(1.0f),
  glewInitialised(false),
  currentImage(0),
  imageServer(3),
  imageNumberToLoad(-1),
  screenFont()
{
    m_context = new wxGLContext(this);
    imageServer.SetPointers(this, m_context);

    //subImage.Create(256, 256, true);

}

void BasicGLPanel::InitFont(FreetypeFont & font, int fontResolution)
{
    if (!fontResolution)
        font.Initialise();
    else
        font.Initialise(fontResolution);
}

void BasicGLPanel::Resized(wxSizeEvent& evt)
{
    //wxGLCanvas::OnSize(evt);
    
    Refresh();
}

void BasicGLPanel::ZoomIn()
{
    //scaling *= 1.01f;
    //image.scale(scaling, scaling);
    zoomTimeRemaining = 60;
    if (currentImage)
        currentImage->ZoomIn();
}

void BasicGLPanel::ZoomOut()
{
    //scaling *= 1.0f / 1.01f; 
    //image.scale(scaling, scaling);
    zoomTimeRemaining = 60;
    if (currentImage)
        currentImage->ZoomOut();
}


void BasicGLPanel::Prepare3DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{
    /*
    *  Inits the OpenGL viewport for drawing in 3D.
    */

    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClearDepth(1.0f);	// Depth Buffer Setup
    glEnable(GL_DEPTH_TEST); // Enables Depth Testing
    glDepthFunc(GL_ALWAYS); // The Type Of Depth Testing To Do
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    glEnable(GL_COLOR_MATERIAL);

    glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x, bottomrigth_y - topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float ratio_w_h = (float)(bottomrigth_x - topleft_x) / (float)(bottomrigth_y - topleft_y);
    gluPerspective(45 /*view angle*/, ratio_w_h, 0.1 /*clip close*/, 200 /*clip far*/);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void BasicGLPanel::Prepare2DViewport(int topleft_x, int topleft_y, int bottomrigth_x, int bottomrigth_y)
{

    /*
    *  Inits the OpenGL viewport for drawing in 2D
    */
    //cout << "Prepare2DViewport" << endl;


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glEnable(GL_TEXTURE_2D);   // textures
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glViewport(topleft_x, topleft_y, bottomrigth_x - topleft_x, bottomrigth_y - topleft_y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //glScalef(scaling, scaling, scaling);
    //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);

    gluOrtho2D(topleft_x, bottomrigth_x, bottomrigth_y, topleft_y);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //glRotatef(30.0f, 0.0f, 0.0f, 1.0f);
    //glScalef(2.0f, 2.0f, 2.0f);
}

int BasicGLPanel::GetWidth()
{
    return GetSize().x;
}

int BasicGLPanel::GetHeight()
{
    return GetSize().y;
}

int firstImage = 1;
/*
void BasicGLPanel::DisplayImage(wxString filename)
{
    int xCentre = GetWidth() / 2;
    int yCentre = GetHeight() / 2;
    image.MoveAbs(xCentre, yCentre);
    image.SetScreenSize(GetWidth(), GetHeight());
    image.scale(scaling);
    image.Load(filename);
}
*/

void BasicGLPanel::DisplayImage(int imageNumber)
{
    //cout << "BasicGLPanel::DisplayImage(" << path << ")" << endl;
    //imagePathToLoad = path;
    imageNumberToLoad = imageNumber;
    fontTimeRemaining = 100;
    zoomTimeRemaining = 100;
}


void BasicGLPanel::OnPaint(wxPaintEvent& evt)
{

    //cout << endl;
    //cout << "BasicGLPanel::OnPaint " << endl;
    //NoteTime(wxT("BasicGLPanel::OnPaint"));

    /*
    wxGLCanvas::SetCurrent(*m_context);

    if (imagePathToLoad.Len() > 0)
    {
        currentImage = imageServer.GetImage(imagePathToLoad);
        imagePathToLoad = wxT("");

        //cout << " currentImage = " << currentImage << endl;
        if (currentImage)
        {
            int xCentre = GetWidth() / 2;
            int yCentre = GetHeight() / 2;
            currentImage->MoveAbs(xCentre, yCentre);
            currentImage->SetScreenSize(GetWidth(), GetHeight());
            currentImage->scale(scaling);
            //currentImage->Show();
        }
    }*/

	wxPaintDC dc(this);

    if (!glewInitialised)
    {
        wxGLCanvas::SetCurrent(*m_context);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            cout << "Error: " << glewGetErrorString(err) << endl;
            exit(1);
        }
        InitFont(screenFont, 24);
        glewInitialised = true;
    }
}

void BasicGLPanel::ClearCache()
{
    imageServer.ClearCache();
}

void BasicGLPanel::Clear()
{
    wxGLCanvas::SetCurrent(*m_context);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black Background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glFlush();
    SwapBuffers();
}

void BasicGLPanel::Render(bool blankScreen)
{
    //cout << endl;
    //NoteTime(wxT("BasicGLPanel::Render"));
    //if (currentImage)
    //    cout << "Current = " << currentImage->width << endl;

    if (!IsShown())         return;
    if (!glewInitialised)   return;

    //cout << "BasicGLPanel::Render " << endl;

    //wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

    imageServer.HandleCaching();

    wxGLCanvas::SetCurrent(*m_context);
    //NoteTime(wxT("Set Current"));

    if (imageNumberToLoad >= 0)
    {
        GL_Image *newImage = imageServer.GetImage(imageNumberToLoad);
        //NoteTime(wxT("Image Loaded"));

        if (newImage->loadedImage)
        {
            if (currentImage)
            {
                bool isFullyVisible = currentImage->IsFullyVisible();

                newImage->CopyScaleAndPositionFrom(*currentImage);
                //cout << "Scale Diff = " << currentImage->GetScaleDifference(*newImage) << endl;
                if (newImage->width > 400)
                {
                    if (isFullyVisible)
                        newImage->SetScaleAndPosition(0, 0, 0.01);
                    newImage->ExpandToSides();
                }
            }
            else
            {
                newImage->Scale(0.01);
                newImage->ExpandToSides();
            }

            currentImage = newImage;

            currentImageNumber = imageNumberToLoad;
            imageNumberToLoad = -1;
        }

        if (currentImage)
        {
            //int xCentre = GetWidth() / 2;
            //int yCentre = GetHeight() / 2;
            //currentImage->SetScreenSize(GetWidth(), GetHeight());
        }
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //cout << "(" << GetWidth() << ", " << GetHeight() << endl;
    // render loaded image
    Prepare2DViewport(0, 0, GetWidth(), GetHeight());
    //glTranslatef(sprite->image->G)


    if (currentImage && !blankScreen)
    {
        //currentImage->setHotspotCentered();
        {
            currentImage->basicGLPanel = this;
            currentImage->ClampToSides();
            currentImage->Render();
        }

        if (fontTimeRemaining)
        {
            fontTimeRemaining--;
            float transparency = 1.0f;

            if (fontTimeRemaining < 20)
                transparency = fontTimeRemaining * 0.05f;

            glLoadIdentity();
			wxString s = currentImage->GetInfoString();
			s.Append("\n");
			s.Append(GetImageNumberInfo());

            screenFont.SetColour(0.0f, 0.0f, 0.0f, transparency*0.5f);
            screenFont.Print(2.0, 2.0, 24.0, s);
            screenFont.SetColour(1, 1, 1, transparency);
            screenFont.Print(0.0, 0.0, 24.0, s);
            screenFont.SetColour(1, 1, 1, 1);
            screenFont.Print(0.0, 120.0, 24.0, " ");

			//s.Printf("%d/%d", currentImageNumber, 0);
			//screenFont.SetColour(0.0f, 0.0f, 0.0f, 0.5f);
			//screenFont.Print(2.0, 62.0, 24.0, s);
			//screenFont.SetColour(1, 1, 1, 1);
			//screenFont.Print(0.0, 60.0, 24.0, s);
		}

        if (zoomTimeRemaining)          // For a while after we adust the zoom, show the zoom percentage on the screen
        {
            zoomTimeRemaining--;

            float transparency = 1.0f;

            if (zoomTimeRemaining < 20)
                transparency = zoomTimeRemaining * 0.05f;

            glLoadIdentity();
            screenFont.SetColour(0.0f, 0.0f, 0.0f, transparency*0.5f);
            screenFont.Print(2.0, 152.0, 24.0, currentImage->GetZoomInfo());
            screenFont.SetColour(1, 1, 1, transparency);
            screenFont.Print(0.0, 150.0, 24.0, currentImage->GetZoomInfo());
            screenFont.SetColour(1, 1, 1, 1);
            screenFont.Print(0.0, 150.0, 24.0, " ");
        }
    }


    glFlush();
    SwapBuffers();    

    //cout << "BasicGLPanel::Render Done" << endl;
}

wxString BasicGLPanel::GetImageNumberInfo()
{
	int numImages = imageServer.GetNumImages();
	wxString s;

	s.Printf("%d of %d", currentImageNumber+1, numImages);
	return s;
}

void BasicGLPanel::MoveRel(float x, float y)
{
    if (currentImage)
    {
        currentImage->MoveRel(x, y);
    }
}

