
#ifndef DIRECTORY_FUNCTIONS_H_INCLUDED
#define DIRECTORY_FUNCTIONS_H_INCLUDED

#include "wx/wx.h"
#include "wx/thread.h"

class ImageBrowser;
class wxTreeCtrl;
class wxTreeItemId;

class DirectorySearcher : public wxThread
{
public:
    DirectorySearcher(ImageBrowser &ib, wxTreeCtrl &tc, wxTreeItemId &id)
    : wxThread(wxTHREAD_DETACHED),
      imageBrowser(ib),
      treeCtrl(tc),
      treeItemId(id)
    {
        std::cout << "Thread constructor" << std::endl;
    }


protected:
    ExitCode Entry();

    //wxString         path;
    ImageBrowser    &imageBrowser;
    wxTreeCtrl      &treeCtrl;
    wxTreeItemId    &treeItemId;
};

void GreyEmptyDirectories(wxTreeCtrl &treeCtrl, wxTreeItemId treeItemId);

#endif
