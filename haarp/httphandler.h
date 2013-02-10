#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include "sockethandler.h"

#include <vector>

using namespace std;

class HTTPHandler : public SocketHandler {

protected:

bool ProxyConnection;
vector<string> tokens;

virtual int AnalyseFirstHeaderLine( string &RequestT ) = 0;
virtual int AnalyseHeaderLine( string &RequestT ) = 0;

public:

bool ReadHeader( string &headerT );
int AnalyseHeader( string &linesT );
ssize_t ReadBodyPart( string &bodyT, bool Chunked );
bool SendHeader( string header, bool ConnectionClose );

HTTPHandler();
virtual ~HTTPHandler();

};

#endif
