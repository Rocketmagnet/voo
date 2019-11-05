#pragma once

#include "image_file_handler_registry.h"
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
                                                        const wxString & description)
{
    //cout << endl << "Registering file handler " << name << ": " << extension << endl;
    ImageFileHandlerInfo imageFileHandlerInfo;

    imageFileHandlerInfo.formatDescription = description;
    imageFileHandlerInfo.formatExtension = extension.Upper();
    imageFileHandlerInfo.formatName = name;
    imageFileHandlerInfo.imageFileHandlerFunction = imageFileHandlerFunction;

    //cout << imageFileHandlerInfo.formatExtension << endl;
    return RegisterImageFileHandler(imageFileHandlerInfo);
}



ImageFileHandler* ImageFileHandlerRegistry::GetImageFileHandlerFromExtension(const wxString & ext)
{
    int i, n = imageFileHandlerInfoVector.size();
    //cout << "Searching for " << ext << " " << this << endl;
    //cout << "Vector address: " << &imageFileHandlerInfoVector << endl;
    //cout << n << endl;
    for (i = 0; i < n; i++)
    {
        //cout << i << ": name=" << imageFileHandlerInfoVector[i].formatExtension << endl;
        //cout      << "   ext=" << imageFileHandlerInfoVector[i].formatExtension << endl;
        //cout      << "   des=" << imageFileHandlerInfoVector[i].formatDescription << endl;

        if (imageFileHandlerInfoVector[i].formatExtension == ext.Upper())
        {
            //cout << "  Found" << endl;
            return (*imageFileHandlerInfoVector[i].imageFileHandlerFunction)();
        }
    }

    return 0;
}

