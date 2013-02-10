#ifndef PROXYHANDLER_H
#define PROXYHANDLER_H

#include "default.h"
#include "connectiontobrowser.h"
#include "connectiontohttp2.h"
#include "logfile.h"
#include "x64compat.h"

#include <string>

using namespace std;

class ProxyHandler {

private:

bool HeaderSend;
bool BrowserDropped;
bool DropBrowser;
bool ScannerUsed;
bool UnlockDone;
bool AnswerDone;
bool ReinitDone;
bool ServerClosed;
bool ServerConnected;
bool DropServer;
int alivecount;
string ConnectedHost;
int ConnectedPort;

string Header;

ConnectionToBrowser ToBrowser;
ConnectionToHTTP2 ToServer;

bool UseParentProxy;
string ParentHost;
int ParentPort;

int MaxDownloadSize;
int KeepBackTime;
int TricklingTime;
unsigned int TricklingBytes;
int KeepBackBuffer;

int TransferredHeader;
int64_t TransferredBody;

bool ProxyMessage( int CommunicationAnswerT, string Answer );
int CommunicationHTTP();
int CommunicationFTP();

#ifdef SSLTUNNEL
int CommunicationSSL();
#endif

public:

void Proxy( SocketHandler &ProxyServerT );
 
ProxyHandler();
~ProxyHandler();

};

#endif
