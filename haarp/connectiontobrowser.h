#ifndef CONNECTIONTOBROWSER_H
#define CONNECTIONTOBROWSER_H

#include "default.h"
#include "httphandler.h"

#include "x64compat.h"

#include <map>

using namespace std; 

class ConnectionToBrowser : public HTTPHandler  {

private:

string Request;
string Host;
int Port;
string IP;
string CompleteRequest;
string RequestType;
string RequestProtocol;
string FtpUser;
string FtpPass;
string UserAgent;
int64_t ContentLength;
bool IsKeepAlive;
bool IsStreamAgent;
vector<string> Methods;
vector<string> StreamUA;

bool Transparent;

int AnalyseFirstHeaderLine( string &RequestT );
int AnalyseHeaderLine( string &RequestT );
int GetHostAndPortOfRequest( string &RequestT, string::size_type StartPos );
int GetHostAndPortOfHostLine( string &HostLineT );

public:

void RewriteUrl( const string &url);
string PrepareHeaderForServer( bool UseParentProxy );
string GetIP();
const string GetHost();
const string GetRequest();
const string GetCompleteRequest();
const string GetRequestProtocol();
const string GetRequestType();
const string GetUserAgent();
bool IsItKeepAlive();
bool IsItStreamAgent();
int64_t GetContentLength();
int GetPort();
void ClearVars();

ConnectionToBrowser();
~ConnectionToBrowser();

};

#endif
