#pragma once

#include "image_file_handler.h"
#include "image_file_handler_registry.h"
#include "thumbnail_canvas.h"
#include <iostream>
using namespace std;

ImageFileHandlerRegistry::ImageFileHandlerRegistry()
{
    //cout << "Created ImageFileHandlerRegistry instance " << this << endl;
}

// Create a singleton of the ImageFileHandlerRegistry
ImageFileHandlerRegistry& ImageFileHandlerRegistry::instance()
{
    static ImageFileHandlerRegistry ifhRegister;
    //cout << "returning " << &ifhRegister << endl;
    return ifhRegister;
}



bool ImageFileHandlerRegistry::RegisterImageFileHandler(ImageFileHandlerInfo const& imageFileHandlerInfo)
{
    //cout << this << ": " << imageFileHandlerInfo.formatExtension << endl;
    //cout << imageFileHandlerInfo.formatExtension << endl;
    imageFileHandlerInfoVector.push_back(imageFileHandlerInfo);
    //cout << imageFileHandlerInfoVector.back().formatExtension << endl;
    //cout << "Vector address: " << &imageFileHandlerInfoVector << endl;

    filtersList.Add("*." + imageFileHandlerInfo.formatExtension);
    return true;
}



bool ImageFileHandlerRegistry::RegisterImageFileHandler(ImageFileHandlerFunction imageFileHandlerFunction,
                                                        const wxString & name,
                                                        const wxString & extension,
                                                        const wxString & description,
                                                        ViewImageAbility viewImageAbility)
{
    ImageFileHandlerInfo imageFileHandlerInfo;

    imageFileHandlerInfo.formatDescription        = description;
    imageFileHandlerInfo.formatExtension          = extension.Upper();
    imageFileHandlerInfo.formatName               = name;
    imageFileHandlerInfo.imageFileHandlerFunction = imageFileHandlerFunction;
    imageFileHandlerInfo.viewImageAbility         = viewImageAbility;

    if (viewImageAbility == CAN_VIEW_IMAGE)
    {
        viewableExtensions.Append(extension);
        viewableExtensions.Append(" ");
    }

    //cout << imageFileHandlerInfo.formatExtension << endl;
    return RegisterImageFileHandler(imageFileHandlerInfo);
}



ImageFileHandler* ImageFileHandlerRegistry::GetImageFileHandlerFromExtension(const wxString & ext)
{
    //cout << "GetImageFileHandlerFromExtension" << endl;
    int i, n = imageFileHandlerInfoVector.size();

    for (i = 0; i < n; i++)
    {
        if (imageFileHandlerInfoVector[i].formatExtension == ext.Upper())
        {
            return (*imageFileHandlerInfoVector[i].imageFileHandlerFunction)();
        }
    }

    return 0;
}


void ImageFileHandlerRegistry::SearchForHandler(wxString fileName, Thumbnail& thumbnail)
{
    int i, n = imageFileHandlerInfoVector.size();

    for (i = 0; i < n; i++)
    {
        ImageFileHandler* imageFileHandler = imageFileHandlerInfoVector[i].imageFileHandlerFunction();
        bool success = imageFileHandler->LoadThumbnail(fileName, thumbnail);
        delete imageFileHandler;

        if (success)
            return;
    }

}