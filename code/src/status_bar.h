#ifndef STATUS_BAR_H_INCLUDED
#define STATUS_BAR_H_INCLUDED
#include "wx/statusbr.h"
#include "deque_thread_safe.h"
#include "wx/wx.h"
#include "wx/thread.h"

class ImageBrowserFrame;

#define STATUS_BAR_DIRECTORY_SUMMARY    0
#define STATUS_BAR_FILE_SIZES           1
#define STATUS_BAR_FILE_FORMAT          2
#define STATUS_BAR_INFORMATION          3

//#define STATUS_TEXT(pos, fmt, ...) { } //wxString s; s.Printf(fmt, __VA_ARGS__); sBarGlobal->SetStatusText(s, pos); }

/* Status bar entries
* 
* 1: Directory contents: number of files, total size
* 2: This file: Size, date, dimensions
* 3: File name
* 4: Resizing status
* 
* 1,2,3 will be per- ImageBrowser
* 4 will be connected only to the global resizer
* 
*/

const int NUM_STATUS_MESSAGES_FOR_TABS = 3;
const int NUM_STATUS_MESSAGES          = NUM_STATUS_MESSAGES_FOR_TABS + 1;


struct StatusBarMessage
{
    StatusBarMessage(wxWindowID _tabID, int _place, wxString _text)
    : tabID(_tabID),
      place(_place),
      text(_text)
    {
    }

    wxWindowID  tabID;
    int         place;
    wxString    text;
};

struct StatusMessageSet
{
    StatusMessageSet(wxWindowID _tabID)
        : tabID(_tabID)
    {
    }

    wxWindowID tabID;
    wxString   messages[NUM_STATUS_MESSAGES_FOR_TABS];
};


class StatusBarThreadSafe : public wxThread
{
public:
    StatusBarThreadSafe(ImageBrowserFrame *imageBrowserFrame);

    void Message(wxWindowID tabID, size_t place, wxString text);
    void SwitchToTab(wxWindowID tabID);

protected:
    ExitCode Entry();

private:
    void StoreStatusMessage(StatusBarMessage& statusBarMessage);
    StatusMessageSet& FindstatusMessageSet(wxWindowID tabID);

    deque_thread_safe<StatusBarMessage>     statusBarMessages;      // Incomming messages from ImageBrowsers
    std::vector<StatusMessageSet>           statusMessageSets;      // Locally stored set of messages for each tab

    wxStatusBar *statusBar;
    wxWindowID   currentTabID;
};

#endif
