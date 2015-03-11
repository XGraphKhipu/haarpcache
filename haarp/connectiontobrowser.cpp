#include "default.h"
#include "connectiontobrowser.h"
#include "params.h"
#include "utils.h"

#include <stdio.h>
#include <arpa/inet.h>

void ConnectionToBrowser::RewriteUrl( const string &url){
    int pos=0;
    if (!url.empty()){
	    pos = url.find("/");
	    if (pos>0) {
            Host = url.substr(0,pos);
            Request = url.substr(pos,url.size());
	    } else {
            Host = url;
            Request = "/";
	    }
    } 
}

//Prepare Header for Server
string ConnectionToBrowser::PrepareHeaderForServer( bool UseParentProxy )
{
    string PortString = "";

    if ( Port != 80 && Port != 21 )
    {
        char porttemp[21];
        snprintf(porttemp, 20, ":%d", Port);
        PortString = porttemp;
    }

    string header;
    header.reserve(20000);

    if ( UseParentProxy )
    {
        //HTTP
        if ( RequestProtocol == "http" )
        {
            header = RequestType;
            header += " http://";
            header += Host;
            header += PortString;
            header += Request;
            header += " HTTP/1.0\r\n";

            CompleteRequest = "http://";
            CompleteRequest += Host;
            CompleteRequest += PortString;
            CompleteRequest += Request;
        }
        //FTP
        else if ( RequestProtocol == "ftp" )
        {
            string AuthString = "";

            if ( FtpUser != "" )
            {
                SearchReplace( FtpUser, ":", "%3A" );
                SearchReplace( FtpUser, "@", "%40" );

                if ( FtpPass != "" )
                {
                    SearchReplace( FtpPass, ":", "%3A" );
                    SearchReplace( FtpPass, "@", "%40" );

                    AuthString = FtpUser;
                    AuthString += ":";
                    AuthString += FtpPass;
                    AuthString += "@";
                }
                else
                {
                    AuthString = FtpUser;
                    AuthString += "@";
                }
            }

            header = RequestType;
            header += " ftp://";
            header += AuthString;
            header += Host;
            header += PortString;
            header += Request;
            header += " HTTP/1.0\r\n";

            CompleteRequest = "ftp://";
            CompleteRequest += Host;
            CompleteRequest += PortString;
            CompleteRequest += Request;
        }
#ifdef SSLTUNNEL
        //CONNECT
        else if ( RequestProtocol == "connect" )
        {
            header = "CONNECT ";
            header += Request;
            header += " HTTP/1.0\r\n";

            CompleteRequest = "connect://";
            CompleteRequest += Request;
        }
#endif
    }
    else
    {
        header = RequestType;
        header += " ";
        header += Request;
        header += " HTTP/1.0\r\n";

        CompleteRequest = RequestProtocol;
        CompleteRequest += "://";
        CompleteRequest += Host;
        CompleteRequest += PortString;
        CompleteRequest += Request;
    }

    //Remove parameters from URL
    string::size_type QMark = CompleteRequest.find( "?" );

    if ( QMark != string::npos )
    {
        CompleteRequest = CompleteRequest.substr( 0, QMark + 1 );
    }

    //Strip long URL
    if (CompleteRequest.length() > 500)
    {
        CompleteRequest = CompleteRequest.substr( 0, 500 );
        CompleteRequest += "...";
    }

    string via = "";
    string it;
    it.reserve(200);

    //Skip first token
    for (vector<string>::iterator itvec = tokens.begin() + 1; itvec != tokens.end(); ++itvec)
    {

        //Uppercase for matching
        it = UpperCase(*itvec);

        if ( MatchBegin( it, "PROXY", 5 ) )
        {
            //Pass Proxy-Authorization to parent proxy if used
            if (! (UseParentProxy && MatchBegin( it, "PROXY-AUTHORIZATION", 19 )) )
            {
                continue;
            }
        }
        else if ( MatchBegin( it, "HOST", 4 ) )
        {
            *itvec = "Host: "+Host;
        }
        else if ( MatchBegin( it, "KEEP-ALIVE", 10 ) )
        {
            continue;
        }
        else if ( MatchBegin( it, "VIA", 3 ) )
        {
            string line = *itvec;
            string::size_type Position = line.find_first_not_of(" ", 4);
            if ( Position != string::npos )
            {
                via = ", " + line.substr(Position);
            }
            continue;
        }
        else if ( MatchBegin( it, "CONNECTION", 10 ) )
        {
            continue;
        }
        else if ( MatchBegin( it, "X-FORWARD", 9 ) )
        {
            continue;
        }
	else if ( MatchBegin( it, "ACCEPT-ENCODING", 15 ) && 
			(UpperCase(Host) == "WWW.YOUTUBE.COM") && 
			(UpperCase(Request).find("/WATCH?") != string::npos) 
		)
	{
		*itvec = "Accept-Encoding: ";
	}


        header += *itvec;
        header += "\r\n";
    }

    if ( Params::GetConfigBool("X_FORWARDED_FOR") )
    {
        header += "X-Forwarded-For: ";
        header += GetIP();
        header += "\r\n";

        header += "Via: 1.0 Haarp";
        header += via;
        header += "\r\n";
    }
    else
    {
        header += "Via: 1.0 PROXY\r\n";
    }

    return header;
}


int ConnectionToBrowser::AnalyseFirstHeaderLine( string &RequestT )
{
    //Uppercase for matching
    string RequestU = UpperCase(RequestT);

    //Looking for GET, POST, HEAD, CONNECT etc.
    for (unsigned int i=0;i < Methods.size(); i++)
    {
        if ( RequestU.find( Methods[i] + " ", 0 ) == 0 )
        {
            RequestType = Methods[i];
            return GetHostAndPortOfRequest( RequestT, Methods[i].size() + 1 );
        }
    }

    return -202;
}


int ConnectionToBrowser::AnalyseHeaderLine( string &RequestT )
{
    //Uppercase for matching
    string RequestU = UpperCase(RequestT);

    if ( MatchBegin( RequestU, "CONTENT-LENGTH: ", 15 ) )
    {
        if ( RequestU.find_first_not_of("0123456789", 16) != string::npos )
        {
            //Invalid Content-Length
            return 0;
        }

        string LengthToken = RequestT.substr( 16 );

        //Sanity check for invalid huge Content-Length
        if ( LengthToken.length() > 18 ) return 0;

        if ( sscanf(LengthToken.c_str(), LLD, &ContentLength) != 1 )
        {
            ContentLength = -1;
        }

        return 0;
    }

    if ( ProxyConnection == false )
    {
        if ( MatchBegin( RequestU, "CONNECTION: ", 12 ) )
        {
            if ( MatchSubstr( RequestU, "KEEP-ALIVE", 12 ) )
            {
                IsKeepAlive = true;
            }
            else
            {
                IsKeepAlive = false;
            }

            return 0;
        }

        if ( MatchBegin( RequestU, "PROXY-CONNECTION: ", 18 ) )
        {
            ProxyConnection = true;

            if ( MatchSubstr( RequestU, "KEEP-ALIVE", 18 ) )
            {
                IsKeepAlive = true;
            }
            else
            {
                IsKeepAlive = false;
            }

            return 0;
        }
    }

    if ( Params::GetConfigBool("FORWARDED_IP") == true )
    {
        if ( MatchBegin( RequestU, "X-FORWARDED-FOR: ", 17 ) )
        {
            IP = RequestT.substr(17);
            return 0;
        }
    }

    if ( MatchBegin( RequestU, "USER-AGENT: ", 12 ) )
    {
        UserAgent = RequestT.substr(12);

        if ( StreamUA.size() > 0 )
        {
            for (vector<string>::iterator UAi = StreamUA.begin(); UAi != StreamUA.end(); ++UAi)
            {
                if ( RequestU.find(*UAi, 12) != string::npos )
                {
                    IsStreamAgent = true;
                }
            }
        }

        return 0;
    }

    //Checks for TRANSPARENT
    if (Transparent && MatchBegin( RequestU, "HOST: ", 6 ) )
    {
        return GetHostAndPortOfHostLine( RequestT );
    }

    return 0;
}


//Get host and port
int ConnectionToBrowser::GetHostAndPortOfHostLine( string &HostLineT )
{
    string HostwithPort = HostLineT.substr( 6 );

    string::size_type PositionPort;

    if ( ( PositionPort = HostwithPort.find( ":", 1 )) != string::npos )
    {
        Host = HostwithPort.substr( 0, PositionPort );

        if ( Host.length() > 256 )
        {
            return -210;
        }

        string PortString = HostwithPort.substr( PositionPort );

        if ( PortString.length() > 6 )
        {
            return -212;
        }

        if ( sscanf(PortString.c_str(), ":%d", &Port) != 1 )
        {
            return -212;
        }

        if ( Port < 1 || Port > 65535 )
        {
            return -212;
        }

        return 0;
    }

    Host = HostwithPort;
    Port = 80;

    return 0;
}


int ConnectionToBrowser::GetHostAndPortOfRequest( string &RequestT, string::size_type StartPos )
{
    string::size_type Begin, LastPosition;

#ifdef SSLTUNNEL
    //Handle SSL
    if (RequestType == "CONNECT")
    {
        RequestProtocol = "connect";

        if ( (Begin = RequestT.find_first_not_of( " ", 8 )) != string::npos )
        {
            if ( (LastPosition = RequestT.find( " ", Begin )) != string::npos )
            {
                string HostwithPort = RequestT.substr( Begin, LastPosition - Begin );

                if ( (Begin = HostwithPort.find( ":", 1 )) != string::npos )
                {
                    Host = HostwithPort.substr( 0, Begin );

                    if (Host.length() > 256)
                    {
                        return -210;
                    }

                    string PortString = HostwithPort.substr( Begin );

                    if ( sscanf(PortString.c_str(), ":%d", &Port) == 1 )
                    {
                        Request = HostwithPort;

                        //If you want more control, edit or use Squid in front
                        if ( Port >= 80 && Port <= 65535 )
                        {
                            return 0;
                        }
                        else
                        {
                            return -211;
                        }
                    }
                }
            }
        }

        return -201;
    }
#endif

    //Check for other protocols..

    //Uppercase for matching
    string RequestU = UpperCase(RequestT);

    //Transparent proxying?
    if ( Transparent )
    {
        if ( RequestT.find( "/", StartPos ) == StartPos )
        {
            if ( (LastPosition = RequestT.find( " ", StartPos )) != string::npos )
            {
                if ( LastPosition > 4096 ) return -201;

                if ( RequestU.find( "HTTP/1.1", LastPosition + 1 ) != string::npos )
                {
                    //KeepAlive by default for HTTP/1.1 clients
                    IsKeepAlive = true;
                }

                RequestProtocol = "http";
                Request = RequestT.substr( StartPos, LastPosition - StartPos );

                return 0;
            }
        }

        return -201;
    }

    //HTTP
    if ( (Begin = RequestU.find( "HTTP://", StartPos )) == StartPos )
    {
        RequestProtocol = "http";
        Begin += 7;
    }
    //FTP
    else if ( (Begin = RequestU.find( "FTP://", StartPos )) == StartPos )
    {
        RequestProtocol = "ftp";
        Begin += 6;
    }
    //No supported protocol found
    else
    {
        return -201;
    }

    //Start parsing request..

    if ( (LastPosition = RequestT.find( " ", Begin )) == string::npos )
    {
        return -201;
    }
    if ( LastPosition == Begin || LastPosition > 4096 )
    {
        return -201;
    }

    if ( RequestU.find( "HTTP/1.1", LastPosition + 1 ) != string::npos )
    {
        //KeepAlive by default for HTTP/1.1 clients
        IsKeepAlive = true;
    }

    //Split domain and path
    string RequestDomain = RequestT.substr( Begin, LastPosition - Begin );

    if ( (LastPosition = RequestDomain.find( "/", 0 )) != string::npos )
    {
        Request = RequestDomain.substr( LastPosition );
        RequestDomain = RequestDomain.substr( 0, LastPosition );
    }
    else
    {
        Request = "/";
    }

    //Check for login info
    if ( (LastPosition = RequestDomain.rfind( "@", string::npos )) != string::npos )
    {
        if ( RequestProtocol == "ftp" )
        {
            string UserPass = RequestDomain.substr( 0, LastPosition );

            string::size_type Position;

            if ( (Position = UserPass.find( ":", 0 )) != string::npos )
            {
                FtpUser = UserPass.substr( 0, Position );
                FtpPass = UserPass.replace( 0, Position + 1, "" );
            }
            else
            {
                FtpUser = UserPass;
                FtpPass = "";
            }
        }

        //Strip login info.. also from HTTP because IE does it anyway
        RequestDomain.replace( 0, LastPosition + 1, "" );
        RequestT.replace( Begin, LastPosition + 1, "" );

        if ( !RequestDomain.length() )
        {
            return -201;
        }
    }

    //Check for Port and Host
    if ( (LastPosition = RequestDomain.rfind( ":", string::npos )) != string::npos )
    {
        string PortString = RequestDomain.substr( LastPosition );

        if ( PortString.length() > 6 )
        {
            return -212;
        }

        if ( sscanf(PortString.c_str(), ":%d", &Port) != 1 )
        {
            return -212;
        }

        if ( Port < 1 || Port > 65535 )
        {
            return -212;
        }

        Host = RequestDomain.substr( 0, LastPosition );
    }
    else
    {
        if ( RequestProtocol == "http" )
        {
            Port = 80;
        }
        else if ( RequestProtocol == "ftp" )
        {
            Port = 21;
        }
        else
        {
            return -215;
        }

        Host = RequestDomain;
    }

    //Sanity check - TODO: Config variable for allowed ports?
    if ( Host.length() > 256 )
    {
        return -210;
    }
    //if ( RequestProtocol == "http" && Port < 80 )
    //{
    //    return -211;
    //}

    return 0;
}


const string ConnectionToBrowser::GetHost()
{
    return Host;
}

const string ConnectionToBrowser::GetRequest()
{
    return Request;
}

const string ConnectionToBrowser::GetCompleteRequest()
{
    return CompleteRequest;
}

const string ConnectionToBrowser::GetRequestProtocol()
{
    return RequestProtocol;
}

const string ConnectionToBrowser::GetRequestType()
{
    return RequestType;
}

const string ConnectionToBrowser::GetUserAgent()
{
    return UserAgent;
}

int64_t ConnectionToBrowser::GetContentLength()
{
    return ContentLength;
}

int ConnectionToBrowser::GetPort()
{
    return Port;
}

string ConnectionToBrowser::GetIP()
{
   if (IP == "")
       IP = inet_ntoa( my_s_addr.sin_addr );

   //Else IP was set by X-Forwarded-For:
   return IP;
}

bool ConnectionToBrowser::IsItKeepAlive()
{
    return IsKeepAlive;
}


bool ConnectionToBrowser::IsItStreamAgent()
{
    return IsStreamAgent;
}

void ConnectionToBrowser::ClearVars()
{
    RequestProtocol = RequestType = Request = Host = IP = FtpUser = FtpPass = UserAgent = CompleteRequest = "";
    Port = ContentLength = -1;
    IsKeepAlive = ProxyConnection = IsStreamAgent = false;
}


//Constructor
ConnectionToBrowser::ConnectionToBrowser()
{

    string TempMethods[] = {METHODS};

    for(unsigned int i = 0; i < sizeof(TempMethods)/sizeof(string); i++)
    {
        Methods.push_back( TempMethods[i] );
    }

    if (Params::GetConfigString("STREAMUSERAGENT") != "")
    {
        string Tokens = UpperCase(Params::GetConfigString("STREAMUSERAGENT"));
        string::size_type Position;
        
        while ((Position = Tokens.find(" ")) != string::npos)
        {
            if (Position == 0)
            {
                Tokens.erase(0, 1);
                continue;
            }
                                    
            StreamUA.push_back(Tokens.substr(0, Position));
            Tokens.erase(0, Position + 1);
        }

        StreamUA.push_back(Tokens);
    }

    Transparent = Params::GetConfigBool("TRANSPARENT");
}


//Destructor
ConnectionToBrowser::~ConnectionToBrowser()
{
}
