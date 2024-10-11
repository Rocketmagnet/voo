#pragma once
#include "wx/wx.h"
#include <wx/arrstr.h>

class ImageFileHandler;
class Thumbnail;

typedef ImageFileHandler* (*ImageFileHandlerFunction)();        // A function which returns a pointer to an ImageFileHandler

enum ViewImageAbility
{
    CANNOT_VIEW_IMAGE = 0,
    CAN_VIEW_IMAGE = 1
};


struct ImageFileHandlerInfo
{
    ImageFileHandlerFunction    imageFileHandlerFunction;
    wxString                    formatName;
    wxString                    formatExtension;
    wxString                    formatDescription;
    ViewImageAbility            viewImageAbility;
};

/// Singleton class which holds a registry of RenderTargetFile instances.
/// You cannot construct an instance of this class, but you could retrieve the singleton
/// by calling \code RenderTargetFileRegister::instance() \endcode static member function.
struct ImageFileHandlerRegistry
{
    static ImageFileHandlerRegistry& instance();

    //ImageFileHandlerRegistry();

    bool RegisterImageFileHandler(ImageFileHandlerInfo const& imageFileHandlerInfo);        //<! Register a SerialiserInfo into the registry.
    bool RegisterImageFileHandler(ImageFileHandlerFunction imageFileHandlerFunction,
                                  const wxString & name,
                                  const wxString & extension,
                                  const wxString & description,
                                ViewImageAbility   viewImageAbility);                            //<! Create a RenderTargetFile with the provided arguments and register it to the registry.



    ImageFileHandler* GetImageFileHandlerFromExtension(const wxString & ext);               //<! Retrieve the RenderTargetFile which saves files with the provided extension.
                                                                                            //<! \param name The name of the renderer, as given to RegisterRenderTargetFile( ).

    void SearchForHandler(wxString fileName, Thumbnail& thumbnail);

    wxArrayString& GetFiltersList() { return filtersList; }
    const wxString GetViewableExtensions() { return viewableExtensions; }

private:
    std::vector<ImageFileHandlerInfo> imageFileHandlerInfoVector;

private:                                                                                    // protecting the ctor, the copy ctor and the assignment operator make this class a singleton
    ImageFileHandlerRegistry();
    wxArrayString       filtersList;
    wxString            viewableExtensions;
};


