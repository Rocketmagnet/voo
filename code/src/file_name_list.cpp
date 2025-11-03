#include "file_name_list.h"
#include "wx/dir.h"
#include <iostream>
#include <algorithm>
#include "status_bar.h"
#include "natural_sort.h"
#include "mathhelpers.h"
using namespace std;


FileNameList::FileNameList()
{
    //cout << "FileNameList::FileNameList()" << endl;
}

FileNameList::~FileNameList()
{
    //cout << "FileNameList::~FileNameList()" << endl;
}

FileNameList::FileNameList(wxString dir)
{
    LoadFileList(dir);
}

void FileNameList::Print()
{
    ///cout << "FileNameList::Print()" << endl;
    for (auto &fileName : files)
    {
        //cout << " : " << fileName.fileName.GetFullPath() << endl;
    }
    //cout << "Done" << endl;
}

bool FileNameList::Contains(wxFileName fileName)
{
    for (auto& fileNameInList : files)
    {
        if (fileNameInList.fileName == fileName)
        {
            return true;
        }
    }

    return false;
}

bool FileSortNatural(const DirSortingItem &fn1, const DirSortingItem &fn2)
{
    return (wxCmpNatural(fn1.fileName.GetName(), fn2.fileName.GetName()) < 0);
}

bool FileSortBasic(const DirSortingItem& fn1, const DirSortingItem& fn2)
{
    return fn1.fileName.GetName().CmpNoCase( fn2.fileName.GetName() ) < 0;
}

void FileNameList::LoadFileList(wxString dir)
{
    files.clear();
    directory.Open(dir);

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

            if (fn.IsOk())
                files.emplace_back(fn, fn.GetModificationTime(), FileSortNatural);
            else
                files.emplace_back(fn, wxDateTime::Now(), FileSortNatural);
            cont = directory.GetNext(&filename);
        }
    }

    directory.Close();

    sort(files.begin(), files.end(), FileSortNatural);
}

void FileNameList::FillArrayWithFileNamesFrom(wxString dir, wxArrayString &arrayString)
{
  
    directory.Open(dir);
    //cout << "FillArrayWithFileNamesFrom(" << dir << ")" << endl;
    if (!directory.IsOpened())
    {
        cout << "FAIL!" << endl;
        return;
    }

    for (auto filter : filters)
    {
        wxDir::GetAllFiles(dir, &arrayString, filter, wxDIR_DEFAULT);
    }

    //for (auto& s : arrayString)
    //    cout << s << endl;
    return;

    /*
    directory.Open(dir);

    cout << "FillArrayWithFileNamesFrom(" << dir << ")" << endl;

    if (!directory.IsOpened())
    {
        cout << "FAIL!" << endl;
        return;
    }

    wxString filename;
    int i, n = filters.size();
    cout << n << " filters" << endl;


    for (i = 0; i<n; i++)
    {
        bool cont = directory.GetFirst(&filename, filters[i], wxDIR_DEFAULT);
        while (cont)
        {
            wxFileName fn = directory.GetName() + wxT("\\") + filename;
            arrayString.push_back(fn.GetFullPath());
            cout << fn.GetFullPath() << endl;
            cont = directory.GetNext(&filename);
        }
    }
    */
}

void FileNameList::AddFilter(wxString ext)
{
    filters.push_back(ext);
}

void FileNameList::AddFileToList(wxString name)
{
    wxFileName fn(name);

    if (fn.IsOk())
        files.emplace_back(fn, fn.GetModificationTime(), FileSortNatural);
    else
        files.emplace_back(fn, wxDateTime::Now(), FileSortNatural);

    //files.emplace_back(fn, fn.GetModificationTime(), FileSortNatural);
}

void FileNameList::Resort()
{
    sort(files.begin(), files.end(), FileSortNatural);
}

bool FileNameList::DeleteFileName(wxFileName fileName)
{
    bool success = wxRemoveFile(fileName.GetFullPath());
    //cout << "FileNameList::DeleteFileName(" << fileName.GetFullName() << ") " << success << endl;
    for (int i = 0; i < files.size(); i++)
    {
        //cout << i << endl;
        if (files[i].fileName == fileName)
        {
            //cout << "Deleting " << i << endl;
            files.erase(files.begin() + i);
            break;
        }
    }
    return success;
}

//bool FileNameList::DeleteFileNumber(int fileNumber)
//{
//    //cout << "FileNameList::DeleteFile(" << fileNumber << ") = " << files[fileNumber].fileName.GetFullName() << endl;
//    bool success = wxRemoveFile(files[fileNumber].fileName.GetFullPath());
//    /*
//    if (success)
//    {
//        std::vector<DirSortingItem>::iterator iter = files.begin();
//        for (int i = 0; i < fileNumber; i++)
//        {
//            iter++;
//        }
//        files.erase(iter);
//    }
//    */
//    return success;
//}

int FileNameList::GetFileNumber(wxString fileName)
{
    for (int i = 0; i < files.size(); i++)
    {
        if (fileName == files[i].fileName.GetFullName())
            return i;
    }

    return -1;
}

size_t FileNameList::Jump(size_t currentImage, int delta, wxString viewableExtensions)
{
    return -1;
    /*
    //cout << "FileNameList::Jump(" << currentImage << ", " << delta << ", " << viewableExtensions << ")" << endl;
    size_t nextImage = currentImage + delta;

    nextImage = clamp(nextImage, (size_t)0, MaxFileNumber());
    //cout << "  nextImage = " << nextImage << endl;

    if (viewableExtensions.Contains(files[nextImage].fileName.GetExt()))
    {
        //cout << "  OK " << endl;
        return nextImage;
    }

    if (delta > 0)                                                                      // Jumping forward?
    {
        //cout << "  Skipping up" << endl;
        while (!viewableExtensions.Contains(files[nextImage].fileName.GetExt()))        // Slide past any extensions we can't view
        {
            nextImage++;                                                                
            //cout << "  " << nextImage << endl;
            if (nextImage == (int)MaxFileNumber())                                      // Hit the end of the list?
            {
                //cout << "  hit end: " << currentImage << endl;
                return currentImage;                                                    // .. then just give up looking.
            }
        }
        return nextImage;
    }

    if (delta < 0)                                                                      // Jumping backward?
    {
        cout << "  Skipping down" << endl;
        while (!viewableExtensions.Contains(files[nextImage].fileName.GetExt()))        // Slide past any extensions we can't view
        {
            nextImage--;
            cout << "  " << nextImage << endl;
            if (nextImage == 0)                                                         // Hit the start of the list?
            {
                cout << "  hit start: " << currentImage << endl;
                return currentImage;                                                    // .. then just give up looking.
            }
        }
        return nextImage;
    }

    cout << "  oops: " << currentImage << endl;
    return currentImage;
    */
}

bool FileNameList::Rename(wxString src, wxString dst)
{
    for (auto& fileName : files)
    {
        if (fileName.fileName == src)
        {
            wxRenameFile(src, dst);
            fileName.fileName = src;
            return true;
        }
    }
    return false;
}