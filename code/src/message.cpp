#include "message.h"
#include <iostream>
#include <wx/url.h>
#include <wx/filename.h>

using namespace std;

LiquidMessage LIQUIDMESSAGE_BLANK(wxT(""));

LiquidMessageName::LiquidMessageName(const wxString &name)
  : messageName(name)
{
    messageName.MakeLower();                    // All message names are lower case

    /*
    int i,n=name.Length(), hash=0, partLength;
    int numParts = messageName.Freq('.')+1;         // how many parts are there to this message name?  (number of '.' + 1)
    hashes.reserve(numParts);                   // allocate that many hashes
    partLocations.reserve(numParts*2);          
    
    partLocations.push_back(0);                 // the first part starts at 0 obvoiusly

    for (i=0; i<n; i++)                         // Now, calculate the hashes, and remember where each part is in the name
    {
        wxChar character = messageName[i];

        if (character == '.')                   // found the end of a part
        {
            partLocations.push_back(i-1);
            partLocations.push_back(partLength);
            hashes.push_back(hash);
            hash=0;
        }
        else
        {
            hash *= 7;
            hash += (int)character;
            partLength++;
        }
    }

    partLocations.push_back(partLength);
    hashes.push_back(hash);
    */
}

void ReplaceEqualsWithSlash(wxString & s)
{
    int i,n=s.Length();

    for (i=0; i<n; i++)
        if (s[i] == wxChar('='))
            s[i] = wxChar('/');
}

void LiquidMessage::SetFromURI(const wxString &uriString)
{
    wxURI         uri(uriString);
    wxString      messageNameString = uri.GetServer();
    wxFileName    dataItemsPath     = uri.GetPath();
    wxArrayString dataItemsArray    = dataItemsPath.GetDirs();
    
    messageDataTable.clear();
    messageName = LiquidMessageName(messageNameString);

    int i, n=dataItemsArray.GetCount();
    n &= 0xFFFE;                                // must be even.

    //cout << uriString.ToAscii() << endl;
    //cout << "messageName = " << messageNameString.ToAscii() << endl;

    for (i=0; i<n; i+=2)
    {
        wxString itemKey  = dataItemsArray[i];
        wxString itemData = dataItemsArray[i+1];
        ReplaceEqualsWithSlash(itemData);
        //cout << itemKey.ToAscii() << "=" << itemData.ToAscii() << endl;
        SetItemData(itemKey, itemData);
    }
}

void LiquidMessage::SetItemData(wxString k, int      i)
{
	wxString s;
	s.Printf(_T("%d"), i);
	messageDataTable[k] = s;
}

void LiquidMessage::SetItemData(wxString k, float    f)
{
	wxString s;
	s.Printf(_T("%f"), f);
	messageDataTable[k] = s;
}

void LiquidMessage::SetItemData(wxString k, Vector2D v)
{
	wxString s;
	s.Printf(_T("%f %f"), v.x, v.y);
	messageDataTable[k] = s;
}

void LiquidMessage::SetItemDataPtr(wxString k, void *ptr)
{
	wxString s;
	s.Printf(_T("%p"), ptr);
	messageDataTable[k] = s;
}


/*
bool operator == (const LiquidMessageName &name1, const LiquidMessageName &name2)
{
    unsigned int i,n = name1.hashes.size();

    if (n != name2.hashes.size())
        return false;

    for (i=0; i<n; i++)
        if (name1.hashes[i] != name2.hashes[i])
            return false;

    return true;
}

bool operator <  (const LiquidMessageName &name1, const LiquidMessageName &name2)    { return (name1.messageName <  name2.messageName); }
bool operator >  (const LiquidMessageName &name1, const LiquidMessageName &name2)    { return (name1.messageName >  name2.messageName); }
*/

// If this messageName is "liquidpcb.input" and the incommingMessageName is "liquidpcb.input.mouse" then return true
// This function has been optimised using the hashes generated in the constructor.  Sorry.

bool LiquidMessageName::Matches(const LiquidMessageName &incommingMessageName)
{/*
    unsigned int i, n=hashes.size();

    if (n > incommingMessageName.hashes.size())             // if incommingMessageName is shorter than me
        return false;                                       // no match


    for (i=0; i<n; i++)                                     // my list of hashes must match incommingMessageName's list of hashes
        if (hashes[i] != incommingMessageName.hashes[i])    // FIXME: There's a small chance that the hashes are the same if the parts are different
            return false;


    return true;                                            // All the hashes match
    */

    return Matches(incommingMessageName.messageName);
}


bool LiquidMessageName::Matches(const wxString &incommingMessageName)
{
    if (messageName.Length() > incommingMessageName.Length())
        return false;

    // Start at the end of the filter, and work backwards.
    // This should generate a "false" sooner than starting from the front, like StartsWith( ),
    // because loads of messages have similar beginnings (e.g. "liquidpcb.") whereas
    // messages will rarely have similar endings.
    //
    // In future I might implement this as a trie or something.

    int i = messageName.Length();

    for (; i>=0; i--)                                       
        if (messageName[i] != incommingMessageName[i])      // 
            return false;

    return true;
}


void LiquidMessageDispatcher::HandleMessageQueue()
{
    currentlyHandlingQueue = true;

    while (messageQueue.size())
    {
        ProcessMessage(*(messageQueue.front()));
        messageQueue.pop();
    }

    currentlyHandlingQueue = false;
}

void LiquidMessageDispatcher::ProcessMessage(LiquidMessage &message)
{
    wxString        messageName = message.messageName.GetMessageName();
    int i, n = liquidMessageNodes.size();
    for (i=0; i<n; i++)
    {
        wxString registeredMessageName = liquidMessageNodes[i].messageName.GetMessageName();
        wxString tail;

        if (message.messageName.GetMessageName().StartsWith(registeredMessageName, &tail) )
        {
            //cout << "  found subscriber" << endl;
            int messageInt = liquidMessageNodes[i].messageInt;
            liquidMessageNodes[i].callback(message, tail, messageInt);
        }
    }

}

void LiquidMessageDispatcher::UpMessage(LiquidMessage &message)
{
    //cout << name.ToAscii() << " SendMessageUp (" << message.messageName.GetMessageName().ToAscii() << ")" << endl;

    if (parentDispatcher)
        parentDispatcher->UpMessage(message);
    else
        ReceiveMessage(message);
}

void LiquidMessageDispatcher::ReceiveMessage(LiquidMessage &message, bool immediate)
{
    //cout << name.ToAscii() << " ReceiveMessage (" << message.messageName.GetMessageName().ToAscii() << ")" << endl;

    if (immediate)
        ProcessMessage(message);
    else
    {
        messageQueue.push(&message);
        if (!currentlyHandlingQueue)
            HandleMessageQueue();
    }
}

void LiquidMessageDispatcher::ReceiveMessage(const wxString & url)
{
    LiquidMessage message(_T(""));
    //cout << "LiquidMessageDispatcher::ReceiveMessage(" << message.messageName.GetMessageName().ToAscii() << ")" << endl;

    message.SetFromURI(url);
    ReceiveMessage(message, true);      // Do it immediately.

//    cout << "done LiquidMessageDispatcher::ReceiveMessage(" << message.messageName.GetMessageName().ToAscii() << ")" << endl;
}


/*
void LiquidMessageDispatcher::FindInterestedReceivers(LiquidMessage &message)
{
    interestedNodes.clear();

    int i,n=liquidMessageNodes.size();

    for (i=0; i<n; i++)
        if (message.messageName.GetMessageName().StartsWith(liquidMessageNodes[i].messageName.GetMessageName()))
            interestedNodes.push_back(&liquidMessageNodes[i]);
}

*/
int LiquidMessageDispatcher::RegisterReceiver(wxString messageName, LiquidMessageCallback callbackFunction, int msgInt)
{
    //cout << name.ToAscii() << " LiquidMessageDispatcher::RegisterReceiver " << messageName.ToAscii() << " " << liquidMessageNodes.size() << endl;
	lastRecieverID++;

    liquidMessageNodes.push_back(LiquidMessageNameAndReceiver(messageName, callbackFunction, lastRecieverID, msgInt));

	return lastRecieverID;
}

void LiquidMessageDispatcher::ForgetAbout(int idToForget)
{
    int i, lastElement = liquidMessageNodes.size()-1;

    for (i=lastElement; i>=0; i--)                                              // Search for all subscriptions
        if (liquidMessageNodes[i].id == idToForget)
        {
            liquidMessageNodes[i] = liquidMessageNodes.back();
            liquidMessageNodes.pop_back();
        }
}


void LiquidMessageConnection::Disconnect()
{
    int i,n=connectionIDs.size();

    if (dispatcher)                                     // disconnect from any current connection
        for (i=0; i<n; i++)
            dispatcher->ForgetAbout(connectionIDs[i]);
}


void LiquidMessageConnection::Connect(wxString messageName, LiquidMessageCallback callbackFunction, int msgInt)
{
    if (dispatcher)
        connectionIDs.push_back(dispatcher->RegisterReceiver(messageName, callbackFunction, msgInt));
}

