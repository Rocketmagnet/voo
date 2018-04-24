#ifndef FILE_NAME_LIST_H_INCLUDED
#define FILE_NAME_LIST_H_INCLUDED

#include <vector>
#include <iostream>
#include "wx/string.h"
#include "wx/dir.h"
//#include "wx/image.h"






class FileNameList
{
public:
    FileNameList();

    FileNameList(wxString dir);
    void LoadFileList(wxString dir);
    void AddFilter(wxString ext);

    size_t   MaxFileNumber()   { return files.size() - 1; }
    wxString operator[](int i) { return files[i];         }

    std::vector<wxString>	files;
    std::vector<wxString>	filters;

    wxDir                   directory;
};

#endif
