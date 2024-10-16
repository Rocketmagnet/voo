#ifndef FILE_NAME_LIST_H_INCLUDED
#define FILE_NAME_LIST_H_INCLUDED

#include <vector>
#include <iostream>
#include "wx/string.h"
#include "wx/arrstr.h"
#include "wx/dir.h"
#include "wx/filename.h"


class DirSortingItem;


typedef bool(*DirSortingItemCmpFunction)(const DirSortingItem&, const DirSortingItem&);

class DirSortingItem
{
public:
    DirSortingItem(const wxFileName& fn, const wxDateTime& dt, DirSortingItemCmpFunction cFunc)
        : fileName(fn),
          dateTime(dt),
          compareFunc(cFunc)
    {}

    bool operator <(const DirSortingItem &rhs) const
    {
        if (compareFunc)
        {
            return compareFunc(*this, rhs);
        }
        else
        {
            //return fileName.GetName().CmpNatural(rhs.fileName.GetName()) < 0;
            return wxDictionaryStringSortAscending(fileName.GetName(), (rhs.fileName.GetName())) < 0;

            // wxNaturalStringSortNoCaseAscending
        }
    }

    wxFileName fileName;
    wxDateTime dateTime;

    DirSortingItemCmpFunction compareFunc;
};


class FileNameList
{
public:
    FileNameList();
    FileNameList(wxString dir);

    void LoadFileList(wxString dir);
    void AddFilter(wxString ext);
    void AddFileToList(wxString name);
    void Resort();
    bool DeleteFileName(wxFileName fileName);
    //bool DeleteFileNumber(int fileNumber);
    int  GetFileNumber(wxString fileName);

    void FillArrayWithFileNamesFrom(wxString directory, wxArrayString &arrayString);

    bool Rename(wxString src, wxString dst);

    size_t   NumFiles()        const { return files.size();                     }
    size_t   MaxFileNumber()   const { return files.size() - 1;                 }
    wxString operator[](int i) const { return files[i].fileName.GetFullPath();  }
    size_t   Jump(size_t currentImage, int delta, wxString viewableExtensions);

//private:
    std::vector<DirSortingItem>	 files;
    std::vector<wxString>	     filters;

    wxDir                        directory;
};


#endif
