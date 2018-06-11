#include "directory_functions.h"
#include <wx/treectrl.h>
#include "wx/dirctrl.h"
#include "wx/dir.h"
#include "imagebrowser.h"
#include <iostream>

using namespace std;

wxThread::ExitCode DirectorySearcher::Entry()
{
    wxTreeItemIdValue cookie;
    wxDirItemData *mainData = (wxDirItemData*)(treeCtrl.GetItemData(treeItemId));

    wxTreeItemId id = treeCtrl.GetFirstChild(treeItemId, cookie);

    cout << "DirectorySearcher::Entry() " << mainData->m_path << endl;

    while (id.IsOk())
    {
        wxDirItemData *data = (wxDirItemData*)(treeCtrl.GetItemData(id));
        cout << "Scanning: " << data->m_path << endl;
        id = treeCtrl.GetNextChild(treeItemId, cookie);
    }

    return 0;
}

void GreyEmptyDirectories(wxTreeCtrl &treeCtrl, wxTreeItemId treeItemId)
{
    wxTreeItemIdValue cookie;
    wxDirItemData *mainData = (wxDirItemData*)(treeCtrl.GetItemData(treeItemId));

    wxTreeItemId id = treeCtrl.GetFirstChild(treeItemId, cookie);

    while (id.IsOk())
    {
        wxDirItemData *data = (wxDirItemData*)(treeCtrl.GetItemData(id));
        wxDir dir = data->m_path;

        if ((!dir.HasFiles()) && (!dir.HasSubDirs()))
        {
            treeCtrl.SetItemTextColour(id, wxColor(128, 128, 128));
        }
        id = treeCtrl.GetNextChild(treeItemId, cookie);
    }

}
