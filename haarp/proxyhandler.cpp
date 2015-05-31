#include "default.h"
#include "proxyhandler.h"
#include "utils.h"
#include "params.h"

#include <errno.h>
#include <deque>
#include <iostream>
#include <fstream>
#include <cstdlib>

extern int LL; //LogLevel
extern int LOK; //LogOK
void ProxyHandler::Proxy( SocketHandler &ProxyServerT )
{
    int ret, CommunicationAnswer, retries;

    int requests = 0;
    alivecount = 0;

    ServerConnected = BrowserDropped = DropBrowser = false;

    //Wait for first connection
    while ( ProxyServerT.AcceptClient( ToBrowser ) == false ) sleep(10);
    //Infinite Processing Loop
    for(;;)
    {
        ++requests;

        ToBrowser.ClearVars();
        ToServer.ClearVars();

        Header = "";
        UnlockDone = AnswerDone = ReinitDone = HeaderSend = DropServer = false;
        TransferredHeader = TransferredBody = 0;

        //New connection needed?
        if ( DropBrowser || BrowserDropped )
        {
            ToBrowser.Close();
            ToServer.Close();

            ServerConnected = BrowserDropped = DropBrowser = false;
            alivecount = 0;

            //Wait for new connection
            //while ( ProxyServerT.AcceptClient( ToBrowser ) == false ) sleep(10);

			// now out of this process, free memory
			break;
        } 

        if ( ++alivecount > 1 )
        {
            //Keep-Alive timeout 10 seconds
            if ( ToBrowser.CheckForData(10) == false )
            {
                DropBrowser = true;
                continue;
            }
        }
        if ( ToBrowser.ReadHeader( Header ) == false )
        {
            if (LL>0) if (alivecount==1) LogFile::ErrorMessage("(%s) Could not read browser header\n", ToBrowser.GetIP().c_str());
            DropBrowser = true;
            continue;
        }
        //~ if (LL > 1) LogFile::AccessMessage("After ToBrowser.ReadHeader: \n%s\n",Header.c_str());
        if ( (ret = ToBrowser.AnalyseHeader( Header )) < 0 )
        {
            if (LL>0) LogFile::ErrorMessage("(%s) Invalid request from browser\n", ToBrowser.GetIP().c_str());
            ProxyMessage( ret, "" );
            DropBrowser = true;
            continue;
        }
        //~ if (LL > 1) LogFile::AccessMessage("After ToBrowser.AnalyseHeader: \n%s\n",Header.c_str());
        if ( ToBrowser.GetHost() == "" || ToBrowser.GetPort() == -1 )
        {
            LogFile::ErrorMessage("(%s) Invalid request from browser (no Host-header?)\n%s", ToBrowser.GetIP().c_str(),Header.c_str());
            ProxyMessage( -201, "" );
            DropBrowser = true;
            continue;
        }

        //Keep-Alive?
        if ( ToBrowser.IsItKeepAlive() == false || ToBrowser.GetRequestType() != "GET" || ( (alivecount > 99) && (ToBrowser.CheckForData(0) == false) ) )
        {
            DropBrowser = true;
        }

        //HTTP REQUEST
        if ( ToBrowser.GetRequestProtocol() == "http" ) //////////////////////  ENTRA AQUI!!
        {
            CommunicationAnswer = CommunicationHTTP();
            //~ if (LL > 1) LogFile::AccessMessage("Despues de ComunicationHTTP (return %i) header: %d, body: %d, range_min=%d, range_max=%d\n", CommunicationAnswer, TransferredHeader, TransferredBody, ToServer.range_min, ToServer.range_max);
        }
        //FTP REQUEST
        else if ( ToBrowser.GetRequestProtocol() == "ftp" )
        {
            if ( UseParentProxy )
            {
                CommunicationAnswer = CommunicationHTTP();
            }
            else
            {
                //TODO: Support ftp even without parentproxy :-)
                ProxyMessage( -110, "" );
                DropBrowser = true;
                continue;
            }
        }
#ifdef SSLTUNNEL
        //SSL CONNECT REQUEST
        else if ( ToBrowser.GetRequestProtocol() == "connect" )
        {
            //Drop Keep-Alive
            ToServer.Close();

            CommunicationAnswer = CommunicationSSL();

            //Close connection
            ToServer.Close();
            ServerConnected = false;

            if ( CommunicationAnswer != 0 ) ProxyMessage( CommunicationAnswer, "" );

            DropBrowser = true;
            continue;
        }
#endif
        else
        {
            LogFile::ErrorMessage("Program Error: Unsupported RequestProtocol: %s\n", ToBrowser.GetRequestProtocol().c_str());
            DropBrowser = true;
            continue;
        }
        ToServer.Update();
        if ( LOK )
        {
			//~ if (LL > 1) LogFile::AccessMessage("(1)*** Teminó La descarga_LUGAR? header: %d, body: %d, range_min=%d, range_max=%d\n",TransferredHeader, TransferredBody, ToServer.range_min, ToServer.range_max);
            //Clean request
            LogFile::AccessMessage("%s %s/%d "LLD" %s %s - NONE/- -\n", ToBrowser.GetIP().c_str(), ToServer.msghit.c_str(), ToServer.GetResponse(), TransferredHeader + TransferredBody, ToBrowser.GetRequestType().c_str(), ToBrowser.GetCompleteRequest().c_str());
        }
        //Retry GET connection if ReadHeader error (-80) or Connect error (-60, -61)
        //Also reconnect if server closed Keep-Alive connection (-60)
        retries = 0;
        while ( (CommunicationAnswer == -80 && ToBrowser.GetRequestType() == "GET") || CommunicationAnswer == -60 || CommunicationAnswer == -61 )
        {
            ToServer.Close();
            ServerConnected = false;

            //Sleep second before retry
            sleep(1);

            CommunicationAnswer = CommunicationHTTP();

            //No need to stop Keep-Alive if retry is clean
            if ( CommunicationAnswer == 0 ) DropServer = false;

            //Too many retries?
            if ( ++retries >= ToServer.IPCount() ) break;
        }

        //Make sure server connection is closed if needed
        if ( DropServer || DropBrowser || BrowserDropped )
        {
            ToServer.Close();
            ServerConnected = false;
        }

        if ( CommunicationAnswer != 0 )
        {
            //Request not clean
            ProxyMessage( CommunicationAnswer, "Error request answer" );
            DropBrowser = true;
        }

        //If some scanner timed out, bail out..
        if ( CommunicationAnswer == 3 ) break;

    }

    //Make sure browser connection is closed
    ToBrowser.Close();

    //Exit process
    //exit(1);
}

int ProxyHandler::CommunicationHTTP()
{
    string HeaderToServer = ToBrowser.PrepareHeaderForServer(UseParentProxy );
    
    ToServer.getLimitBytes(HeaderToServer);
    
    int64_t ContentLengthReference = ToBrowser.GetContentLength();
    
    //Check that POST has Content-Length
    if ( (ToBrowser.GetRequestType() == "POST") && (ContentLengthReference == -1) )
    {
        BrowserDropped = true;
        LogFile::ErrorMessage("(%s) Browser POST without Content-Length header\n", ToBrowser.GetIP().c_str());
        return -10;
    }

    //Make server connection
    if ( UseParentProxy )
    {
        if ( ServerConnected == false )
        {
            if ( ToServer.SetDomainAndPort( ParentHost, ParentPort ) == false ) //llamada a Cache()
            {
                LogFile::ErrorMessage("Could not resolve parent proxy (%s)\n", ParentHost.c_str());
                return -51;
            }
            if ( ToServer.ConnectToServer() == false )
            {
                LogFile::ErrorMessage("Could not connect to parent proxy (%s/%s:%d)\n", ToServer.GetIP().c_str(), ParentHost.c_str(), ParentPort);
                return -61;
            }
        }
    }
    else
    {
        //We need to close Keep-Alive connection if host to connect changes
        if ( ServerConnected && (ToBrowser.GetHost() != ConnectedHost || ToBrowser.GetPort() != ConnectedPort) )
        {
            ToServer.Close();
            ServerConnected = false;
        }

        if ( ServerConnected == false )
        {
            if ( ToServer.SetDomainAndPort( ToBrowser.GetHost(), ToBrowser.GetPort(),ToBrowser.GetRequest() ) == false ) //llamada a Cache()
            {
                if (LL>0) LogFile::ErrorMessage("Could not resolve hostname (%s)\n", ToBrowser.GetHost().c_str() );
                return -50;
            }
            if ( ToServer.ConnectToServer() == false )
            {
                if (LL>0) LogFile::ErrorMessage("Could not connect to server (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
                return -60;
            }

            ConnectedHost = ToBrowser.GetHost();
            ConnectedPort = ToBrowser.GetPort();
        }
    }
    //We are now connected
    ServerConnected = true;

    //Send header to server
    if ( ToServer.SendHeader( HeaderToServer, DropBrowser, ToBrowser.GetRequest() ) == false ) //posible llamar a Cache()
    {
        if (LL>0) LogFile::ErrorMessage("(%s) Could not send header to server (%s/%s:%d) - DropBrowser = %d -\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort(), (int)DropBrowser);
        DropServer = true;
        return -60;
    }

    //Check for client body
    if ( ContentLengthReference >= 0 )
    {
        //Transfer body if there is some
        if ( ContentLengthReference > 0 )
        {
            int repeat = int (ContentLengthReference / MAXRECV);
            string Body;

            for(int i=0; i <= repeat; i++)
            {
                Body = "";

                if ( i == repeat )
                {
                    int rest = ContentLengthReference - (MAXRECV * repeat);

                    if ( ToBrowser.RecvLength( Body, rest ) == false )
                    {
                        BrowserDropped = true;
                        if (LL>0) LogFile::ErrorMessage("(%s) Could not read browser body\n", ToBrowser.GetIP().c_str());
                        return -10;
                    }
                }
                else
                {
                    if ( ToBrowser.RecvLength( Body, MAXRECV ) == false )
                    {
                        BrowserDropped = true;
                        if (LL>0) LogFile::ErrorMessage("(%s) Could not read browser body\n", ToBrowser.GetIP().c_str());
                        return -10;
                    }
                }

                if ( ToServer.Send( Body ) == false )
                {
                    if (LL>0) LogFile::ErrorMessage("(%s) Could not send browser body to server (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
                    DropServer = true;
                    return -76;
                }
            }
        }

        //Check for extra CRLF (IE Bug or Content-Length: 0)
        if ( ToBrowser.CheckForData(0) )
        {
            string TempString;

            if ( ToBrowser.Recv( TempString, true, -1 ) < 0 )
            {
                BrowserDropped = true;
                if (LL>0) LogFile::ErrorMessage("(%s) Could not finish browser body transfer\n", ToBrowser.GetIP().c_str());
                return -10;
            }

            //It is OK if browser finished (empty) or extra CRLF received
            if ( TempString.find_first_not_of( "\r\n", 0 ) != string::npos )
            {
                BrowserDropped = true;
                if (LL>0) LogFile::ErrorMessage("(%s) Browser body was too long\n", ToBrowser.GetIP().c_str());
                return -10;
            }
        }

    }

    //Get response from server
    if ( ToServer.ReadHeader( Header ) == false ) //Se conoce Content-Length
    {
        if (LL>0) LogFile::ErrorMessage("(%s) Could not read server header (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
        DropServer = true;
        return -80;
    }
    TransferredHeader = Header.size();

    //Analyse server headers
    int ret = ToServer.AnalyseHeader( Header );
    if ( ret < 0 )
    {
        if (LL>0) LogFile::ErrorMessage("(%s) Invalid server header received (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
        DropServer = true;
        return ret;
    }

    //Server did not send Keep-Alive header, close after request (we can keep browser open)
    //El servidor no envía paque-cabecera, después de cerrar la solicitud (puedemos mantener el navegador abierto).
    if ( ToServer.IsItKeepAlive() == false ) DropServer = true;

    //Get Content-Length
    ContentLengthReference = ToServer.GetContentLength(); //////////////////// -- message MAXMIN CANCEL

    //Chunked workaround?
    bool ChunkedTransfer = ToServer.IsItChunked();

    if ( ContentLengthReference == -1 )
    {
        //No Keep-Alive for unknown length
        DropBrowser = true;
    }
    else if ( ChunkedTransfer )
    {
        // Chunked transfer not allowed with Content-Length
        LogFile::ErrorMessage("(%s) Invalid server header received, Chunked encoding with Content-Length (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
        DropServer = true;
        return -233;
    }

    if ( ChunkedTransfer ) DropBrowser = true;

    Header = ToServer.PrepareHeaderForBrowser();
    //No body expected? Not much to be done then
    //Cuerpo no se esperaba? No hay mucho que hacer entonces
    if ( (ToServer.GetResponse() == 304) || (ContentLengthReference == 0) || (ToBrowser.GetRequestType() == "HEAD") || (ToServer.GetResponse() == 204) )
    {
        //Send header to browser
        if ( ToBrowser.SendHeader( Header, DropBrowser ) == false )
        {
            BrowserDropped = true;
            if (LL>0) LogFile::ErrorMessage("(%s) Could not send header to browser\n", ToBrowser.GetIP().c_str());
            return -10;
        }

        //Check for extra CRLF
        if ( ToServer.CheckForData(0) )
        {
            string BodyTemp;
            ssize_t BodyLength = ToServer.Recv( BodyTemp, true, -1 );
            if ( (BodyLength > 0) && (BodyTemp.find_first_not_of( "\r\n", 0 ) != string::npos) )
            {
                if (LL>0) LogFile::ErrorMessage("(%s) Server tried to send body when not expected (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
                DropServer = true;
            }
        }

        //Return clean
        return 0;
    }

    //Read first part of body
    string BodyTemp;
    ssize_t BodyLength = ToServer.ReadBodyPart( BodyTemp, ChunkedTransfer );

    //Server disconnected?
    if ( BodyLength < 0 )
    {
        DropServer = true;
        if (LL>0) LogFile::ErrorMessage("(%s) Could not read initial server body (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
        return -74;
    }
	

    //Nothing received?
    if ( BodyLength == 0 )
    {
        //Lets be safe and close all connections
        //Vamos a ser seguro y cerrar todas las conexiones
        DropBrowser = true;

        //Send header to browser
        if ( ToBrowser.SendHeader( Header, DropBrowser ) == false )
        {
            BrowserDropped = true;
            if (LL>0) LogFile::ErrorMessage("(%s) Could not send header to browser\n", ToBrowser.GetIP().c_str());
            return -10;
        }

        //Return clean
        return 0;
    }

    //Send header to browser
    if ( ToBrowser.SendHeader( Header, DropBrowser ) == false )
    {
        BrowserDropped = true;
        if (LL>0) LogFile::ErrorMessage("(%s) Could not send header to browser\n", ToBrowser.GetIP().c_str());
        return -10;
    }
	if (ToServer.msghit == "HIT" && Params::GetConfigInt("ZPH_TOS_LOCAL")>0)
	{
		ToBrowser.setTOS(Params::GetConfigInt("ZPH_TOS_LOCAL")+1);
	}
    int64_t ContentLength = BodyLength; // to server
    TransferredBody = ContentLength;
	//BodyTemp - server
	//ContentLengthReference - total
	//ContentLength - browser
    //Server Body Transfer Loop
    for(;;)
    {
        //If we received more than Content-Length, discard the rest
        //Si recibimos más que Content-Length (total), desechar el resto
        if ( (ContentLengthReference > 0) && (ContentLength > ContentLengthReference) )
        {
            BodyTemp.erase( BodyTemp.size() - (ContentLength - ContentLengthReference) );

            ContentLength = ContentLengthReference;
			/*Change log*/
            if (LL>0) LogFile::ErrorMessage("(%s) Server sent more than Content-Length (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());

            //Drop server connection (we can keep browser open)
            DropServer = true;
        }
        //if (LL > 0) LogFile::AccessMessage("Pasando por for;; - BODY-BROWSER: '%s' \n", BodyTemp.c_str());
        //Send body to browser
        if ( ToBrowser.Send( BodyTemp ) == false )
        {
            BrowserDropped = true;
            /*Change log*/
            if (LL>0) if (alivecount==1) LogFile::ErrorMessage("(%s) - Could not send body to browser\n", ToBrowser.GetIP().c_str());
            return -10;
        }
        //File completely received?
        if ( ContentLength == ContentLengthReference ) break;

        //Read more of body
        if ( (BodyLength = ToServer.ReadBodyPart( BodyTemp, ChunkedTransfer )) < 0 )
        {
            DropServer = true;
            if (LL>0) LogFile::ErrorMessage("(%s) Could not read server body (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
            return -75;
        }
        //if (LL > 0) LogFile::AccessMessage("Pasando por for;; - BODY-SERVER: '%s' \n", BodyTemp.c_str());
        //Server finished, end loop
        if ( BodyLength == 0 )
        {
            //If we did not receive all data, close all connections
            //Si no hemos recibido todos los datos, cierre todas las conexiones.
            if ( ContentLength < ContentLengthReference ) DropBrowser = true;

            break;
        }
        ContentLength += BodyLength;
        TransferredBody = ContentLength;
		
        //Continue bodyloop..
    }
    ToServer.Update();
    //Return clean
    return 0;

}


//Not yet implemented..
int ProxyHandler::CommunicationFTP()
{
    return 0;
}


#ifdef SSLTUNNEL
int ProxyHandler::CommunicationSSL()
{
    string BodyTemp;
    ssize_t BodyLength;

    Header = ToBrowser.PrepareHeaderForServer( false, UseParentProxy );

    if ( UseParentProxy )
    {
        if ( ToServer.SetDomainAndPort( ParentHost, ParentPort, ToBrowser.GetRequest() ) == false ) //llamada a Cache()
        {
            LogFile::ErrorMessage("Could not resolve parent proxy (%s)\n", ParentHost.c_str() );
            return -51;
        }
        if ( ToServer.ConnectToServer() == false )
        {
            LogFile::ErrorMessage("Could not connect to parent proxy (%s/%s:%d)\n", ToServer.GetIP().c_str(), ParentHost.c_str(), ParentPort);
            return -61;
        }

        if ( ToServer.SendHeader( Header, true, ToBrowser.GetRequest() ) == false ) //llamada a Cache()
        {
            if (LL>0) LogFile::ErrorMessage("Could not send header to server (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
            return -60;
        }

        if ( ToServer.ReadHeader( Header ) == false )
        {
            if (LL>0) LogFile::ErrorMessage("Could not read server header (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
            DropServer = true;
            return -80;
        }

        string::size_type Position = Header.find_first_of("0123456789", Header.find(" "));

        if ( Position == string::npos )
        {
            if (LL>0) LogFile::ErrorMessage("Invalid HTTP response from parent proxy to SSL tunnel\n");
            return -300;
        }

        //If response not 200, we have some error from parent proxy
        if ( Header.substr(Position, 3) != "200" )
        {
            //Send header to browser
            if ( ToBrowser.SendHeader( Header, true ) == false )
            {
                BrowserDropped = true;
                if (LL>0) LogFile::ErrorMessage("(%s) Could not send header to browser\n", ToBrowser.GetIP().c_str());
                return -10;
            }

            int64_t ContentLengthReference = ToServer.GetContentLength(); //////////////////// -- message MAXMIN CANCEL

            //No body expected?
            if ( ContentLengthReference == 0 ) return 0;

            int64_t ContentLength = 0;

            //Server Body Transfer Loop
            for(;;)
            {
                //Read Body
                if ( (BodyLength = ToServer.ReadBodyPart( BodyTemp, false )) < 0 )
                {
                    if (LL>0) LogFile::ErrorMessage("(%s) Could not read server body (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ParentHost.c_str(), ParentPort);
                    DropServer = true;
                    return -75;
                }

                //If server finished, exit loop
                if ( BodyLength == 0 ) break;

                ContentLength += BodyLength;
                TransferredBody = ContentLength;

                //If we received more than Content-Length, discard the rest
                if ( (ContentLengthReference > 0) && (ContentLength > ContentLengthReference) )
                {
                    BodyTemp.erase( BodyTemp.size() - (ContentLength - ContentLengthReference) );

                    ContentLength = ContentLengthReference;

                    if (LL>0) LogFile::ErrorMessage("(%s) Server sent more than Content-Length (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetIP().c_str(), ParentHost.c_str(), ParentPort);
                }

                //Send body to browser
                if ( ToBrowser.Send( BodyTemp ) == false )
                {
                    BrowserDropped = true;
                    if (LL>0) if (alivecount==1) LogFile::ErrorMessage("(%s) Could not send body to browser\n", ToBrowser.GetIP().c_str());
                    return -10;
                }

                //File completely received?
                if ( ContentLength == ContentLengthReference ) break;
            }
            
            if (LL>0) LogFile::ErrorMessage("(%s) SSL tunneling failed through parentproxy (response: %s)\n", ToBrowser.GetIP().c_str(), Header.substr(Position, 3).c_str());
            return 0;
        }
    }
    else
    {
        if ( ToServer.SetDomainAndPort( ToBrowser.GetHost(), ToBrowser.GetPort() ) == false )
        {
            if (LL>0) LogFile::ErrorMessage("Could not resolve hostname: %s\n", ToBrowser.GetHost().c_str() );
            return -50;
        }
        if ( ToServer.ConnectToServer() == false )
        {
            if (LL>0) LogFile::ErrorMessage("Could not connect to server (%s/%s:%d)\n", ToServer.GetIP().c_str(), ToBrowser.GetHost().c_str(), ToBrowser.GetPort());
            return -60;
        }
    }

    Header = "HTTP/1.0 200 Connection established\r\n";

    if ( ToBrowser.SendHeader( Header, true ) == false )
    {
        BrowserDropped = true;
        if (LL>0) LogFile::ErrorMessage("(%s) Could not send SSL header to browser\n", ToBrowser.GetIP().c_str());
        return -10;
    }

    int ret;

    while ( (ret = ToBrowser.CheckForSSLData( ToBrowser.sock_fd, ToServer.sock_fd )) > 0 )
    {
        if ( ret == 2 )
        {
            BodyLength = ToServer.ReadBodyPart( BodyTemp, false );
            if ( BodyLength < 1 ) break;
            if ( ToBrowser.Send( BodyTemp ) == false ) break;
            continue;
        }
        else if ( ret == 1 )
        {
            BodyLength = ToBrowser.ReadBodyPart( BodyTemp, false );
            if ( BodyLength < 1 ) break;
            if ( ToServer.Send( BodyTemp ) == false ) break;
            continue;
        }
    }

    return 0;
}
#endif


bool ProxyHandler::ProxyMessage( int CommunicationAnswerT, string Answer )
{

    string filename = "";
    string message = "";

    switch ( CommunicationAnswerT )
    {
            if ( LOK )
            {
                LogFile::AccessMessage("%s %s %d %s %d+"LLD" OK\n", ToBrowser.GetIP().c_str(), ToBrowser.GetRequestType().c_str(), ToServer.GetResponse(), ToBrowser.GetCompleteRequest().c_str(), TransferredHeader, TransferredBody);
            }
            break;

        case -50:
            message = ToBrowser.GetHost();
            filename = "ERROR DNS";
            break;

        case -51:
            message = "Parentproxy did not resolve";
            filename = "ERROR DNS";
            break;

        case -60:
            message = "Connection failed";
            filename = "ERROR DOWN";
            break;

        case -61:
            message = "Parentproxy down";
            filename = "ERROR DOWN";
            break;

        case -74:
            message = "Zero sized reply";
            filename = "ERROR DOWN";
            break;

        case -75:
            message = "Could not read body";
            filename = "ERROR DOWN";
            break;

        case -76:
            message = "Could not send body";
            filename = "ERROR DOWN";
            break;

        case -80:
            message = "Could not read headers";
            filename = "ERROR DOWN";
            break;

        case -110:
            message = "FTP is currently supported only<br>if PARENTPROXY is used!";
            filename = "ERROR REQUEST";
            break;

        case -201:
            message = "Invalid request";
            filename = "ERROR REQUEST";
            break;

        case -202:
            message = "Invalid request method";
            filename = "ERROR REQUEST";
            break;

        case -210:
            message = "Hostname too long";
            filename = "ERROR REQUEST";
            break;

        case -211:
            message = "Port not allowed";
            filename = "ERROR REQUEST";
            break;

        case -212:
            message = "Invalid port";
            filename = "ERROR REQUEST";
            break;

        case -215:
            message = "Unsupported protocol";
            filename = "ERROR REQUEST";
            break;

        case -230:
            message = "Invalid HTTP response from server";
            filename = "ERROR REQUEST";
            break;

        case -231:
            message = "Server tried to send partial data<br>and RANGE is set to false";
            filename = "ERROR REQUEST";
            break;

        case -232:
            message = "Server sent forbidden Transfer-Encoding header";
            filename = "ERROR REQUEST";
            break;

        case -233:
            message = "Server sent Chunked response with Content-Length";
            filename = "ERROR REQUEST";
            break;

        case -250: //File larger than MAXDOWNLOADSIZE
            LogFile::AccessMessage("%s %s %d %s %d+"LLD" OVERMAXSIZE\n", ToBrowser.GetIP().c_str(), ToBrowser.GetRequestType().c_str(), ToServer.GetResponse(), ToBrowser.GetCompleteRequest().c_str(), TransferredHeader, TransferredBody);
            message = ToBrowser.GetCompleteRequest();
            filename = "ERROR MAXSIZE";
            break;

#ifdef SSLTUNNEL
        case -300:
            message = "SSL tunneling failed through parentproxy";
            filename = "ERROR REQUEST";
            break;
#endif

        default:

            //Log if we have error not defined above.. all should be there!
            if(LL>0) LogFile::ErrorMessage("Connection Error (%d): Url: %s%s\n", CommunicationAnswerT,ToBrowser.GetHost().c_str(),ToBrowser.GetRequest().c_str());
            char ErrorNumber[11];
            snprintf(ErrorNumber, 10, "%d", CommunicationAnswerT);
            message = ErrorNumber;
            filename = "ERROR BODY";
            break;

    }

    if ( BrowserDropped ) return false;

    //Send report to browser if possible
    if ( HeaderSend == false )
    {
        string UA = ToBrowser.GetUserAgent();
        string Code;

        //IE and Show friendly HTTP errors :(
        if ( UA.find("MSIE") != string::npos )
        {
            Code = "200";
        }
        else
        {
            Code = "403";
        }

        //Start header
        string errorheader = "HTTP/1.0 " + Code + " ";
	    errorheader += "\r\nMensagem: " + message + "\r\nErro: ";

        //Create header body
        if ( filename == "ERROR DNS" )
        {
            errorheader += "DNS error by Haarp";
        }
        else if ( filename == "ERROR DOWN" )
        {
            errorheader += "Server down by Haarp";
        }
        else if ( filename == "ERROR REQUEST" )
        {
            errorheader += "Request error by Haarp";
        }
        else
        {
            errorheader += "Forbidden by Haarp";
        }

        //End header
        errorheader += "\r\nContent-Type: text/html\r\nProxy-Connection: close\r\nConnection: close\r\n\r\n";

        if ( ToBrowser.Send( errorheader ) == false )
        {
            return false;
        }
    }

    //Send report page, but for HEAD request body is not allowed
    if ( (filename != "") && (ToBrowser.GetRequestType() != "HEAD") )
    { 
        string TemplateError = filename+": "+message;
        if ( ToBrowser.Send( TemplateError ) == false ) BrowserDropped = true;
    }

return false;
}


//Constructor
ProxyHandler::ProxyHandler()
{
    if ( Params::GetConfigString("PARENTPROXY") != "" )
    {
        UseParentProxy = true;
        ParentHost = Params::GetConfigString("PARENTPROXY");
        ParentPort = Params::GetConfigInt("PARENTPORT");
    }
    else
    {
        UseParentProxy = false;
    }

    Header.reserve(20000);

}


//Destructor
ProxyHandler::~ProxyHandler()
{
}

