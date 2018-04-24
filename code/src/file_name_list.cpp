#include "file_name_list.h"
#include "wx/dir.h"
#include <iostream>
#include "status_bar.h"
using namespace std;


//#include <gl/glu.h>



FileNameList::FileNameList()
{
}

FileNameList::FileNameList(wxString dir)
{
    LoadFileList(dir);
}


void FileNameList::LoadFileList(wxString dir)
{
    cout << "LoadFileList" << endl;
    files.clear();
    directory.Open(dir);

    cout << directory.GetName() << endl;

    wxString filename;
    int i, n = filters.size();

    for (i = 0; i<n; i++)
    {
        bool cont = directory.GetFirst(&filename, filters[i], wxDIR_FILES);
        while (cont)
        {
            files.push_back(directory.GetName() + wxT("\\") + filename);
            cont = directory.GetNext(&filename);
        }
    }
}



void FileNameList::AddFilter(wxString ext)
{
    filters.push_back(ext);
}
