#ifndef _drawable_
#define _drawable_

#include "wx/image.h"
#include "wx/thread.h"
#include <vector>

#include "vector3d.h"
#include <iostream>

extern "C"
{
#include "jpeg_turbo.h"
};


#define MAX_WIDTH          (4096 - 10)
#define MAX_HEIGHT         10000
#define BLOCK_SIZE_PIXELS (4096*32)

#define OVERLAP_NONE   0
#define OVERLAP_TOP    1
#define OVERLAP_LEFT   2
#define OVERLAP_RIGHT  4

class wxString;

class GL_Image;
class GL_ImageServer;
class BasicGLPanel;
class wxGLContext;
class FileNameList;

class RectangleVector
{
public:
    RectangleVector()
    : TL(0,0),
      BR(0,0)
    {}

    RectangleVector(Vector2D tl, Vector2D br)
    : TL(tl),
      BR(br)
    { }

    RectangleVector(int width, int height)
      : TL(0,0),
        BR(width, height)
    { }

    Vector2D GetFraction(Vector2D v) const
    {
        double x = (v.x - TL.x) / (BR.x - TL.x);
        double y = (v.y - TL.y) / (BR.y - TL.y);

        return Vector2D(x, y);
    }

    void Subtract(Vector2D v)
    {
        TL -= v;
        BR -= v;
    }

    double   XSize() const { return BR.x - TL.x; };
    double   YSize() const { return BR.y - TL.y; } ;
    Vector2D  Size() const { return Vector2D(BR.x - TL.x, BR.y - TL.y); };
    Vector2D TL, BR;
};

std::ostream& operator << (std::ostream& os, const RectangleVector& v);

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

class TextureUpload
{
public:
    TextureUpload()
    : wxImg(0),
      hasGeneratedTexture(false),
      uploadedTexture(false)
    {
    }

    ~TextureUpload();


    void Init(wxImage *img, Vector2D topLeft, Vector2D bottomRight, int clip);
    bool UploadNextBlock();
    void Render(Vector2D scrTL, Vector2D scrBR);

    wxImage          *wxImg;
    GLuint            ID;
    //wxSize            textureSize;                        // The size of the texture for this piece of the image

    RectangleVector   textureSize;                          // In pixels
    RectangleVector     originalImagePortion;               //   
    RectangleVector     expandedImagePortion;               //   
    //RectangleVector   renderableImagePortion;               // 

    RectangleVector    allocatedTexturePortion;             // In texture coordinates (0.0 .. 1.0)
    RectangleVector   renderableTexturePortion;             // 


    int               copyWidth;

    bool              hasGeneratedTexture;
    bool              uploadedTexture;
    bool              valid;
    int               currentY;
    int               clips;
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
    void CopyPositionFrom(const GL_Image &image);
    void CopyVisibilityFrom(const GL_Image &image);
    void CopyScaleAndPositionFrom(const GL_Image &image);
    double VisibilityAtScale(double sc) const;

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
    void   CalculateTextureSizes();

    wxString    GetInfoString() const;
    wxString    GetZoomInfo()   const;
    void        SetFileName(wxString fn);


//protected:
    int              width, height;                 // Actual image dimensions
    BasicGLPanel    *basicGLPanel;

    float       tex_coord_x, tex_coord_y;

    TextureUpload   textureUploads[4];

    bool        uploadedTexture;
    wxImage     wxImg;
    GLubyte    *imageData;
    int         nextBlockToUpload;
    int         blockSize;
    int         lastBlock;
    bool        loadedImage;
    wxString    fileName;

private:
	jpeg_load_state		load_state;
    float               x, y, scale;  // (x=y=0 means image is centered)
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
        
        for (unsigned int i = 0; i < imageSet.size(); i++)
            imageSet[i].glImage.basicGLPanel = panel;
    }

    void SetFileNameList(FileNameList *fnl);
    void Reset();

    void HandleCaching();
    int  NextImageToCache();

    void ClearCache();
    int       Cache(int imageNumber);
    GL_Image* GetImage(int imageNumber);

    void UpdateDebuggingText();
	size_t GetNumImages();

private:
    int GetCacheLocation(int imageNumber);
    int GetOldestCacheLocation();

    std::vector<ImageAndNumber>     imageSet;
    int                             currentImage;
    size_t                          t;                  // time. Each image has a sequence number, used to determine which image is oldest.
    FileNameList                   *fileNameList;
    BasicGLPanel                   *basicGLPanel;
    wxGLContext                    *glContext;

    wxString                        currentDirectory;
};

#endif
