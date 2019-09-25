#include "file_name_list.h"
#include "wx/dir.h"
#include <iostream>
#include <algorithm>
#include "status_bar.h"
#include "natural_sort.h"
using namespace std;


//#include <gl/glu.h>



FileNameList::FileNameList()
{
}

FileNameList::FileNameList(wxString dir)
{
    LoadFileList(dir);
}

bool FileSortNatural(const DirSortingItem &fn1, const DirSortingItem &fn2)
{
    //return (fn1.fileName.GetName().CmpNoCase(fn2.fileName.GetName()) < 0);
    return (wxCmpNatural(fn1.fileName.GetName(), fn2.fileName.GetName()) < 0);
}

void FileNameList::LoadFileList(wxString dir)
{
    //cout << "LoadFileList(" << dir << ")" << endl;
    files.clear();
    directory.Open(dir);

    //cout << directory.GetName() << directory.IsOpened() << endl;

    if (!directory.IsOpened())
        return;

    wxString filename;
    int i, n = filters.size();

    for (i = 0; i<n; i++)
    {
        bool cont = directory.GetFirst(&filename, filters[i], wxDIR_FILES);
        while (cont)
        {
            wxFileName fn = directory.GetName() + wxT("\\") + filename;
            files.emplace_back(fn, fn.GetModificationTime(), FileSortNatural);
            //files.push_back(directory.GetName() + wxT("\\") + filename);
            cont = directory.GetNext(&filename);
        }
    }

    sort(files.begin(), files.end(), FileSortNatural);
}


void FileNameList::AddFilter(wxString ext)
{
    filters.push_back(ext);
}

void FileNameList::AddFileToList(wxString name)
{
    wxFileName fn(name);
    files.emplace_back(fn, fn.GetModificationTime(), FileSortNatural);
}

void FileNameList::Resort()
{
    sort(files.begin(), files.end(), FileSortNatural);
}

bool FileNameList::DeleteFileNumber(int fileNumber)
{
    cout << "FileNameList::DeleteFile(" << fileNumber << ") = " << files[fileNumber].fileName.GetFullName() << endl;
    bool success = wxRemoveFile(files[fileNumber].fileName.GetFullPath());

    if (success)
    {
        std::vector<DirSortingItem>::iterator iter = files.begin();
        for (int i = 0; i < fileNumber; i++)
        {
            iter++;
        }
        files.erase(iter);
    }

    return success;
}

