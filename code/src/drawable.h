#ifndef _drawable_
#define _drawable_

#include "wx/image.h"
#include "wx/thread.h"
#include <vector>
#include <iostream>

class wxString;

class GL_Image;
class GL_ImageServer;
class BasicGLPanel;
class wxGLContext;
class FileNameList;

class ImageLoader : public wxThread
{
public:
    ImageLoader(GL_Image &image, wxString fn, BasicGLPanel *panel, wxGLContext *context)
    : wxThread(wxTHREAD_DETACHED),
      glImage(image),
      fileName(fn),
      basicGLPanel(panel),
      glContext(context)
    {
        std::cout << "Thread constructor" << std::endl;
    }

    //~ImageLoader();
    friend class GL_Image;
    friend class GL_ImageServer;

protected:
    ExitCode Entry();

    GL_Image      &glImage;
    wxString       fileName;
    BasicGLPanel  *basicGLPanel;
    wxGLContext   *glContext;
};


class GL_Image
{
public:
    GL_Image();
    ~GL_Image();

    void Invalidate();
    void Load(wxString path);
    void UploadNextBlock();
    void Render();
    //void rotate(int angle);
    //void setHotspotCentered();

    bool IsFullyVisible() const;

    void CopyScaleAndPositionFrom(const GL_Image &image)
    {
        if (image.IsFullyVisible())
        {
            std::cout << "Fully visible" << std::endl;
            scale = 0.01;
            x = 0;
            y = 0;
            ExpandToSides();
        }
        else
        {
            std::cout << "Not fully visible" << std::endl;
            //std::cout << this << "  Copying scale " << image.scale << "  (" << x << ", " << y << ")  from " << &image << std::endl;
            scale = image.scale;
            x = image.x;
            y = image.y;
        }
    }

    void SetScaleAndPosition(float xx, float yy, float sc)
    {
        //std::cout << this << "  SetScaleAndPosition(" << sc << "  (" << xx << ", " << yy << ")" << std::endl;
        scale = sc;
        x = xx;
        y = yy;
    }

    void Scale(float sc)
    {
        //std::cout << this << "  Scaling " << sc << "  (" << x << ", " << y << ")" << std::endl;
        scale *= sc;
        x     *= sc;
        y     *= sc;
    }

    void ZoomIn() { Scale(1.01f); }
    void ZoomOut() { Scale(1.0f/1.01f); }

    void MoveRel(float xx, float yy)
    {
        x += xx;
        y += yy;
        //std::cout << this << "  Move Rel(" << x << ", " << y << ")" << std::endl;
    }

    void   ClampToSides();
    void   ExpandToSides();
    void   CreateFakeImage();
    void   CopyImageLine(int y);
    double GetScaleDifference(const GL_Image& glImage) const;

    wxString    GetInfoString() const;
    wxString    GetZoomInfo()   const;
    void        SetFileName(wxString fn);

    //friend class ImageLoader;
    //friend class GL_ImageServer;

//protected:
    int              width, height, textureWidth, textureHeight;
    BasicGLPanel    *basicGLPanel;
    //int         screenWidth, screenHeight;

    float       tex_coord_x, tex_coord_y;
    //float       left, right, top, bottom;

    GLuint      ID;
    wxImage     wxImg;
    GLubyte    *imageData;
    int         nextBlockToUpload;
    int         blockSize;
    int         lastBlock;
    bool        loadedImage;
    bool        uploadedTexture;
    bool        hasGeneratedTexture;
    wxString    fileName;

private:
    float       x, y, scale;  // (x=y=0 means image is centered)
};


struct ImageAndNumber
{
    ImageAndNumber()
        : imageNumber(-1),
        creationTime(0)
    {
    }

    GL_Image   glImage;
    int        imageNumber;
    size_t     creationTime;
};

class GL_ImageServer
{
public:
    GL_ImageServer(size_t _cacheSize)
    : imageSet(_cacheSize),
      currentImage(-1),
      fileNameList(0),
      t(0)
    {
    }

    void SetPointers(BasicGLPanel *panel, wxGLContext *context)
    {
        basicGLPanel = panel;
        glContext    = context;
        
        for (int i = 0; i < imageSet.size(); i++)
            imageSet[i].glImage.basicGLPanel = panel;
    }

    void SetFileNameList(FileNameList *fnl);

    void HandleCaching();
    int  NextImageToCache();

    int       Cache(int imageNumber);
    GL_Image* GetImage(int imageNumber);



private:
    int GetCacheLocation(int imageNumber);
    int GetOldestCacheLocation();

    std::vector<ImageAndNumber>     imageSet;
    int                             currentImage;
    size_t                          t;                  // time. Each image has a sequence number, used to determine which image is oldest.
    FileNameList                   *fileNameList;
    BasicGLPanel                   *basicGLPanel;
    wxGLContext                    *glContext;

    GL_Image                        testImage;
};

#endif
