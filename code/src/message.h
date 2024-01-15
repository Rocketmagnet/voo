#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <wx/wxprec.h>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/mdi.h>
#endif

#include <map>
#include <vector>
#include <iostream>
#include "vector3d.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <queue>


#define LiquidMessageCallback boost::function<int(LiquidMessage&, wxString&, int)>
#define CREATE_LIQUID_CALLBACK(y) bind(&y, this, _1, _2, _3)

#define MESSAGE_STOP        0       //<! Returned from a message handler. Means no more handlers should receive this message
#define MESSAGE_CONTINUE    1       //<! Returned from a message handler. Means more handlers may receive this message


class LiquidMessageName
{
public:
    LiquidMessageName() {};
    LiquidMessageName(const wxString &name);
    void SetLastCharacter(wxChar c)
    {
        if (messageName.Length())
            messageName[messageName.Length() - 1] = c;
    }

    wxString GetMessageName() { return messageName; }

    bool Matches(const LiquidMessageName &incommingMessageName);    /*<! If this messageName is "liquidpcb.input" and the incommingMessageName is "liquidpcb.input.mouse" then return true */
    bool Matches(const wxString &incommingMessageName);             /*<! If this messageName is "liquidpcb.input" and the incommingMessageName is "liquidpcb.input.mouse" then return true */

    wxString messageName;
};



class LiquidMessage
{
public:
    LiquidMessage()
        : magic(0xCAFE),
        messageName(_T(""))
    {
    }

    LiquidMessage(wxString messageType)
        : magic(0xCAFE),
        messageName(messageType)
    {
        //LogTime(_T("  message"));
    }

    ~LiquidMessage()
    {
        magic = 0x1234;
        //std::cout << "~LiquidMessage(" << messageName.GetMessageName().ToAscii() << ")" << std::endl;
    }

    /*
    LiquidMessage(const LiquidMessage &message)
    : messageName(message.messageName),
    messageDataTable(message.messageDataTable),
    inputStatus(message.inputStatus)
    {
    }
    */
    void SetFromURI(const wxString &uriString);
    void SetName(const wxString &uriString);
    void SetLastCharacter(wxChar c) { messageName.SetLastCharacter(c); }

    void SetItemData(wxString k, wxString    i) { messageDataTable[k] = i; }
    void SetItemData(wxString k, int         i);
    void SetItemData(wxString k, float       f);
    void SetItemData(wxString k, Vector2D    v);
    void SetItemDataPtr(wxString k, void     *ptr);

    wxString GetItemData(wxString itemKey)
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return _T(""); }

        std::map<wxString, wxString>::iterator it = messageDataTable.find(itemKey);
        if (it != messageDataTable.end())
            return messageDataTable[itemKey];
        else
            return _T("");
    }

    int GetItemDataInt(wxString itemKey)
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return 0; }

        long l;
        wxString s = GetItemData(itemKey);
        s.ToLong(&l);
        return (int)l;
    }

    float GetItemDataFloat(wxString itemKey)
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return 0.0f; }

        double d;
        wxString s = GetItemData(itemKey);
        s.ToDouble(&d);
        return (float)d;
    }

    Vector2D GetItemDataVector2D(wxString itemKey)
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return Vector2D(0, 0); }

        double x, y;
        Vector2D v;
        wxString s1 = GetItemData(itemKey);
        wxString s2 = s1.AfterFirst(' ');
        s1.Truncate(s1.Find(' '));
        s1.ToDouble(&x);
        s2.ToDouble(&y);
        //std::cout << "s1s2 = " << s1.ToAscii() << ", " << s2.ToAscii() << std::endl;
        //std::cout << "xy = " << x << ", " << y << std::endl;
        return Vector2D((float)x, (float)y);
    }

    void* GetItemDataPtr(wxString itemKey)
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return 0; }

        long l;
        wxString s = GetItemData(itemKey);
        s.ToLong(&l, 16);
        return (void*)l;
    }

    void PrintContents()
    {
        if (magic != 0xCAFE) { std::cout << "ERROR: Liquid message used after distruction" << std::endl;  return; }

        std::map<wxString, wxString>::iterator it = messageDataTable.begin();
        for (it = messageDataTable.begin(); it != messageDataTable.end(); it++)
        {
            std::cout << it->first << " : " << it->second << std::endl;
        }
    }

    int                             magic;
    LiquidMessageName               messageName;
    std::map<wxString, wxString>    messageDataTable;
    //InputStatus			           *inputStatus;
};


class LiquidMessageNameAndReceiver
{
public:
    LiquidMessageNameAndReceiver(wxString name, LiquidMessageCallback callbackFunction, int myID, int msgInt = 0)
        : messageName(name),
        callback(callbackFunction),
        id(myID),
        messageInt(msgInt)
    {}

    void operator = (const LiquidMessageNameAndReceiver &LMNaR)
    {
        messageName = LMNaR.messageName;
        callback = LMNaR.callback;
    }

    friend class LiquidMessageDispatcher;

private:
    LiquidMessageName       messageName;
    LiquidMessageCallback   callback;
    int                     id;
    int                     messageInt;
};


class LiquidMessageDispatcher
{
public:
    LiquidMessageDispatcher(LiquidMessageDispatcher* parentLiquidDispatcher)
        : parentDispatcher(parentLiquidDispatcher),
        lastRecieverID(1),
        name(_T("no name")),
        currentlyHandlingQueue(false)
    {};

    virtual void UpMessage(LiquidMessage &message);            //<! Messages going up the tree
    virtual void ReceiveMessage(LiquidMessage &message, bool immediate = false);        //<! Messages going down the tree
    virtual void ReceiveMessage(const wxString & url);          //<! Messages going down the tree
    virtual void ProcessMessage(LiquidMessage &message);

    friend class LiquidMessageConnection;

    wxString    name;

protected:
    void    HandleMessageQueue();

private:
    // These two can only be used by LiquidMessageConnection
    int  RegisterReceiver(wxString messageName, LiquidMessageCallback callbackFunction, int msgInt = 0);	// returns ID which must be used for forgetting
    void ForgetAbout(int idToForget);

    std::vector<LiquidMessageNameAndReceiver  >   liquidMessageNodes;         // set of registered receivers
                                                                              // Pointers point to liquidMessageNodes.
    LiquidMessageDispatcher                *parentDispatcher;
    int							            lastRecieverID;
    bool                                    currentlyHandlingQueue;
    std::queue<LiquidMessage*>                   messageQueue;
};



//!  The only way to connect a class's member function to a LiquidDispatcher
/*!
Problem: What if a class subscribes to a message, then gets deleted without unsubscribing?  You'll get a seg fault.
The solution is to only allow connections through the LiquidMessageConnection class.  The message handling class
owns as many LiquidMessageConnections as it has connections to make. When the parent class is deleted, the
LiquidMessageConnection destructors are called, and they unsubscribe from the dispatcher.
*/
class LiquidMessageConnection
{
public:

    //! The constructor
    /*! The connection cannot happen inside the constructor because the v-tables of the parent class have not been
    initialised during the init-list phase. So the constructor sets itself to safe values. */
    LiquidMessageConnection(LiquidMessageDispatcher *dispatcherPointer) : dispatcher(dispatcherPointer) {};
    //LiquidMessageConnection()  : callback(0), dispatcher(0), connectionID(0)   {};


    //! Connect to a dispatcher
    /*!
    \param messageName the messages you want to subscribe to. e.g. "liquidpcb.input.mouse"
    \param callbackFunction the member function which handles the messages.  Use CREATE_LIQUID_CALLBACK(ClassName::MemberFunctionName)
    \param dispatcherPointer obviously a pointer to the dispatcher.
    */
    void Connect(wxString messageName, LiquidMessageCallback callbackFunction, int msgInt = 0);
    void Disconnect();

    //! The destructor
    /*!
    The destructor automatically unsubscribes.
    */
    ~LiquidMessageConnection()
    {
        Disconnect();
    }

private:
    std::vector<int>                 connectionIDs;

    //LiquidMessageCallback       callback;             /*!< The function which will get called when the message arrives */
    LiquidMessageDispatcher     *dispatcher;          /*!< pointer to the parent dispatcher. */
                                                      //wxString                    messageFilter;        /*!< e.g. "liquidpcb.input.mouse" */
                                                      //int                         connectionID;         /*!< This is passed to the dispatcher when unsubscribing. See the destructor.*/
};


//extern LiquidMessageDispatcher mainLiquidDispatcher;

void TestMessages();

extern LiquidMessage LIQUIDMESSAGE_BLANK;

#endif
