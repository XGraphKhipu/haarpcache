#include "httphandler.h"
#include "logfile.h"
#include "utils.h"
#include "default.h"

#include <stdio.h>

//Read header
//Leer Cabecera
bool HTTPHandler::ReadHeader( string &headerT )
{
    headerT = "";

    string tempheader;
    ssize_t read;
    int received = 0;
    string::size_type position;

    //Read initial header, ignore whitespace from beginning
    //Leer cabecera inicial, ignorar los espacios en blanco desde el principio
    for(;;)
    {
        read = SocketHandler::Recv( tempheader, false, -1 );

        if ( read < 1 )
        {
            return false;
        }

        received += read;

        if ( (position = tempheader.find_first_not_of("\r\n\t ")) != string::npos )
        {
            if ( position > 0 )
            {
                tempheader.erase( 0, position );

                read -= position;

                string ws;
                SocketHandler::RecvLength( ws, position );
            }

            //Jump to header processing
            break;
        }

        if ( received > MAXHTTPHEADERLENGTH )
        {
            LogFile::ErrorMessage("Too large header received (>%d)\n", MAXHTTPHEADERLENGTH);
            return false;
        }

        SocketHandler::RecvLength( tempheader, read );

        tempheader = "";
    }

    bool WrongHeader = false;
    int poscount = 0;

    while ( (position = tempheader.find("\r\n\r\n")) == string::npos )
    {
        //Maybe we should also look for \n\n (19.3 RFC 1945 - Tolerant Applications)
        if ( (position = tempheader.find("\n\n")) != string::npos )
        {
            WrongHeader = true;
            break;
        }

        //Header not yet found
        //Read and delete part of header containing no \r\n\r\n
        if ( SocketHandler::RecvLength( headerT, read ) == false )
        {
            return false;
        }

        poscount += read;
        received += read;

        //Too big header
        if ( received > MAXHTTPHEADERLENGTH )
        {
            LogFile::ErrorMessage("Too large header received (>%d)\n", MAXHTTPHEADERLENGTH);
            return false;
        }

        if ( (read = SocketHandler::Recv( tempheader, false, -1 )) < 1 )
        {
            //Did not receive last empty line?
            if ( read == 0 && ( headerT.find_last_of( "\r\n" ) == headerT.size() - 1 ) )
            {
                return true;
            }

            return false;
        }
    }

    //Read last part of header
    if ( SocketHandler::RecvLength( headerT, position - poscount + 2 ) == false )
    {
        return false;
    }

    if ( WrongHeader == false )
    {
        //Read last \r\n
        if ( SocketHandler::RecvLength( tempheader, 2 ) == false )
        {
            return false;
        }
    }

    return true;
}


//Split header to tokens
int HTTPHandler::AnalyseHeader( string &linesT )
{

    //Delete header tokens
    tokens.clear();

    string::size_type lastposition = 0;

    //Do "Tolerant Applications" - RFC 1945 - Hypertext Transfer Protocol -- HTTP/1.0
    while ( (lastposition = linesT.find( "\r", lastposition )) != string::npos )
    {
        linesT.replace( lastposition, 1, "" );
    }

    lastposition = 0;

    string::size_type length = linesT.length();
    string::size_type position, positiontmp;
    
    if ( (position = linesT.find( "\n", 0 )) == string::npos )
    {
        return -201;
    }

    int ret;
    string tempToken, headerbase;

    //Read first line with AnalyseFirstHeaderLine
    bool First = true;

    //Loop through headers
    while ( position != string::npos && lastposition != length )
    {
        tempToken = linesT.substr( lastposition, position - lastposition );

        if ( (lastposition = tempToken.find_last_not_of("\t ")) != string::npos )
        {
            tempToken = tempToken.substr( 0, lastposition + 1 );

            if ( First == true )
            {
                //Analyse request header
                if ( (ret = AnalyseFirstHeaderLine( tempToken )) < 0 )
                {
                    return ret;
                }
                First = false;
            }
            else
            {
                if ( (positiontmp = tempToken.find(":")) != string::npos )
                {
                    headerbase = tempToken.substr(0, positiontmp + 1);
                    headerbase += " ";

                    //Make sure we have "Header:<SPACE>value"
                    if ( (positiontmp = tempToken.find_first_not_of(" ", positiontmp + 1)) != string::npos )
                    {
                        tempToken = headerbase + tempToken.substr(positiontmp);

                        if ( (ret = AnalyseHeaderLine( tempToken )) < 0 )
                        {
                            return ret;
                        }
                    }

                }
            }

            //Add header to send queue
            tokens.push_back( tempToken );
        }

        lastposition = position + 1;
        position = linesT.find( "\n", lastposition );
    }

    return 0;
}


//Read part of Body
ssize_t HTTPHandler::ReadBodyPart( string &bodyT, bool Chunked )
{
    bodyT = "";
    ssize_t count;

    if ( Chunked == false )
    {
        if ( (count = SocketHandler::Recv( bodyT, true, -1 )) < 0)
        {
            return -1;
        }

        return count;
    }

    // Handle Chunked encoding, return one chunk

    string Temp = "";
    int received = 0;
    int read;
    unsigned int csize;
    string::size_type position, hpos;

    // Read header
    for(;;)
    {
        read = SocketHandler::Recv( Temp, false, -1 );

        if ( read < 1 ) return -1;

        if ( (position = Temp.find("\r\n")) != string::npos )
        {
            if ( position == 0 )
            {
                SocketHandler::RecvLength( Temp, 2 );
                Temp = "";
                continue;
            }

            string UTemp = UpperCase(Temp);
            hpos = UTemp.find_first_not_of("0123456789ABCDEF");

            // Read max 8 hex = 4gb
            if ( hpos == string::npos || hpos < 1 || hpos > 7 )
            {
                LogFile::ErrorMessage("Invalid Chunked-header received\n");
                return -1;
            }

            if ( sscanf( UTemp.substr(0, hpos).c_str(), "%x", &csize ) != 1 )
            {
                LogFile::ErrorMessage("Invalid Chunked-header received\n");
                return -1;
            }

            SocketHandler::RecvLength( Temp, position + 2 );

            break;
        }

        received += read;

        if ( received > 256 )
        {
            LogFile::ErrorMessage("Too large Chunked-header received (>256)\n");
            return -1;
        }
    }

    // All chunks received?
    if ( csize == 0 ) return 0;

    // Does someone send over 1MB chunks?
    if ( csize > 1048576 )
    {
        LogFile::ErrorMessage("Too large Chunked-body size received (>1MB)\n");
        return -1;
    }

    if ( RecvLength( bodyT, csize ) == false ) return -1;

    return csize;
}


//Send Header
bool HTTPHandler::SendHeader( string header, bool ConnectionClose )
{
    if ( ProxyConnection ) header += "Proxy-";

    if ( ConnectionClose )
    {
        header += "Connection: close\r\n\r\n";
    }
    else
    {
        header += "Connection: keep-alive\r\n\r\n";
    }

    if ( SocketHandler::Send( header ) == false )
    {
        return false;
    }

    return true;
}

//Send Header -debugger
bool HTTPHandler::SendHeader( string header, bool ConnectionClose, int *status )
{
    if ( ProxyConnection ) header += "Proxy-";

    if ( ConnectionClose )
    {
        header += "Connection: close\r\n\r\n";
    }
    else
    {
        header += "Connection: keep-alive\r\n\r\n";
    }
	*status = -100;
    if ( SocketHandler::Send( header, status ) == false )
    {
        return false;
    }

    return true;
}


//Constructor
HTTPHandler::HTTPHandler()
{
}


//Destructor
HTTPHandler::~HTTPHandler()
{
}
