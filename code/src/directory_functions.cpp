#include "directory_functions.h"
#include <wx/treectrl.h>
#include "wx/dirctrl.h"
#include "wx/dir.h"
#include "wx/filename.h"
#include "wx/arrstr.h"
#include "imagebrowser.h"
#include <iostream>

using namespace std;

#define FRACTIONAL_SLASH_UNICODE_CHARACTER  0x2044


// Returns true if the string contains forbidden characters.
bool HasForbiddenPathChars(const wxString &dir)
{
    //cout << "HasForbiddenPathChars(" << dir << ") ";
    static const wxString forbiddenChars = wxFileName::GetForbiddenChars();

    for (wxUniChar ch : dir)
    {
        if (forbiddenChars.Contains(ch))
        {
            return true;
        }

        if (ch == FRACTIONAL_SLASH_UNICODE_CHARACTER)   // Windows interprets this as a path separator!
        {
            return true;
        }
    }

    return false;
}

wxThread::ExitCode DirectorySearcher::Entry()
{
    wxTreeItemIdValue cookie;
    wxDirItemData *mainData = (wxDirItemData*)(treeCtrl.GetItemData(treeItemId));

    wxTreeItemId id = treeCtrl.GetFirstChild(treeItemId, cookie);

    //cout << "DirectorySearcher::Entry() " << mainData->m_path << endl;

    while (id.IsOk())
    {
        wxDirItemData *data = (wxDirItemData*)(treeCtrl.GetItemData(id));
        //cout << "Scanning: " << data->m_path << endl;
        id = treeCtrl.GetNextChild(treeItemId, cookie);
    }

    return 0;
}

void GreyEmptyDirectories(wxTreeCtrl &treeCtrl, wxTreeItemId treeItemId, wxArrayString &knownDirList)
{
    wxTreeItemIdValue cookie;
    wxDirItemData *mainData = (wxDirItemData*)(treeCtrl.GetItemData(treeItemId));
    wxTreeItemId id = treeCtrl.GetFirstChild(treeItemId, cookie);


    while (id.IsOk())
    {
        
        wxDirItemData *data = (wxDirItemData*)(treeCtrl.GetItemData(id));
        wxDir          dir  = data->m_path;
        wxFileName fileName(data->m_path);
        wxString      label = treeCtrl.GetItemText(id);


        if (fileName.IsDirReadable())
        {
            if ((!dir.HasFiles()) && (!dir.HasSubDirs()))
            {
                treeCtrl.SetItemTextColour(id, wxColor(128, 128, 128));
            }
            else if (HasForbiddenPathChars(label))
            {
                treeCtrl.SetItemTextColour(id, wxColor(255, 64, 64));
            }
            else
            {
                if (dir.HasFiles("*.mp4") ||
                    dir.HasFiles("*.wmv") ||
                    dir.HasFiles("*.avi") ||
                    dir.HasFiles("*.mpg") ||
                    dir.HasFiles("*.mpeg"))
                {
                    treeCtrl.SetItemTextColour(id, wxColor(64, 64, 255));
                }

                knownDirList.push_back(data->m_path);
            }
        }
        id = treeCtrl.GetNextChild(treeItemId, cookie);
    }
}
    