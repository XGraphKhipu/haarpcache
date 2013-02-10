#ifndef CONNECTIONTOHTTP_H
#define CONNECTIONTOHTTP_H

#include "httphandler.h"

#include "x64compat.h"

class ConnectionToHTTP : public HTTPHandler {

private:

int HTMLResponse;
int64_t ContentLength;
bool IsKeepAlive;
bool IsImage;
bool IsChunked;

int AnalyseFirstHeaderLine( string &RequestT );
int AnalyseHeaderLine( string &RequestT );

public: 

string PrepareHeaderForBrowser();
int GetResponse();
int64_t GetContentLength();
bool IsItKeepAlive();
bool IsItImage();
bool IsItChunked();
void ClearVars();

ConnectionToHTTP();
~ConnectionToHTTP();

};

#endif
