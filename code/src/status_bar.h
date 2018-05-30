#ifndef STATUS_BAR_H_INCLUDED
#define STATUS_BAR_H_INCLUDED
#include "wx/statusbr.h"
#include <iostream>
extern wxStatusBar *sBarGlobal;

#define STATUS_BAR_DIRECTORY_SUMMARY    0
#define STATUS_BAR_FILE_INFO            1
#define STATUS_BAR_FILE_FORMAT          2
#define STATUS_BAR_FILE_DIMENSIONS      3

#define STATUS_TEXT(pos, fmt, ...) { wxString s; s.Printf(fmt, __VA_ARGS__); sBarGlobal->SetStatusText(s, pos);  std::cout << "* " << s << std::endl; }


#endif
