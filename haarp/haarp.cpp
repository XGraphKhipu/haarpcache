#include "default.h"
#include "logfile.h"
#include "helper.h"
#include "sockethandler.h"
#include "proxyhandler.h"
#include "params.h"

#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <iostream>
#include <cstdlib>

bool rereadall = false;
bool childrestart = false;
int LL = 0; //LogLevel
int LOK = 0; //LogOK

int main(int argc, char *argv[])
{
    if ( Params::SetParams(argc,argv) == false )
    {
        cout << "Exiting.." << endl;
        exit(1);
    }

    LL = Params::GetConfigInt("LOGLEVEL");
    LOK = Params::GetConfigBool("LOG_OKS");

    if ( Params::GetConfigBool("DISPLAYINITIALMESSAGES") )
    {
        cout << "Starting Haarp Version: " << VERSION << endl;
    }
	//~ cout<<"EXE: "<<argv[0]<<endl;
    //Install signal handlers
    if ( InstallSignal(0) < 0 )
    {
        cout << "Could not install signal handlers" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    SocketHandler ProxyServer;

    //Bind daemon port
    if ( ProxyServer.CreateServer( Params::GetConfigInt("PORT"), Params::GetConfigString("BIND_ADDRESS") ) == false )
    {
        cout << "Could not create server (already running?)" << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Change user/group ID
    if ( ChangeUserAndGroup(Params::GetConfigString("USER"), Params::GetConfigString("GROUP")) == false )
    {
        cout << "Exiting.." << endl;
        exit(1);
    }

    //Open logs
    if ( LogFile::InitLogFiles(Params::GetConfigString("ACCESSLOG").c_str(), Params::GetConfigString("ERRORLOG").c_str()) == false )
    {
        cout << "Could not open logfiles!" << endl;
        cout << "Invalid permissions? Maybe you need: chown " << GetUser() << " " << Params::GetConfigString("ACCESSLOG").substr(0, Params::GetConfigString("ACCESSLOG").rfind("/")) << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }

    LogFile::ErrorMessage("=== Starting Haarp\n");

    LogFile::ErrorMessage("Running as user: %s, group: %s\n", GetUser().c_str(), GetGroup().c_str());

    if ( Params::GetConfigString("PARENTPROXY") != "" )
    {
        LogFile::ErrorMessage("Use parent proxy: %s:%d\n", Params::GetConfigString("PARENTPROXY").c_str(), Params::GetConfigInt("PARENTPORT"));
    }

	cout << "Listen on port "<<Params::GetConfigInt("PORT")<<endl;

    if ( Params::GetConfigBool("TRANSPARENT") )
    {
        LogFile::ErrorMessage("Use transparent proxy mode\n");
        cout << "Use transparent proxy mode"<<endl;
    }

    if ( Params::GetConfigBool("DAEMON") )
    {
        if ( MakeDaemon() == false )
        {
            cout << "Could not fork daemon" << endl;
            cout << "Exiting.." << endl;
            exit(1);
        }
    }

    pid_t pid = getpid();

    if ( WritePidFile( pid ) == false )
    {
        LogFile::ErrorMessage("Can not write to PIDFILE!\n");
    }

    LogFile::ErrorMessage("Process ID: %d\n", pid);

    int maxservers = Params::GetConfigInt("MAXSERVERS");
    int servernumber = Params::GetConfigInt("SERVERNUMBER");

    int Instances = 0;
    int startchild = 0;
    int status;

    bool restartchilds = false;

    ProxyHandler Proxy;
    //Infinite Server Loop
    for(;;)
    {
        if ( rereadall ) //Signal Refresh
        {
            rereadall = false;

            LogFile::ErrorMessage("Signal HUP received, reloading\n");
            //Reopen logs
            LogFile::InitLogFiles(Params::GetConfigString("ACCESSLOG").c_str(), Params::GetConfigString("ERRORLOG").c_str());
            if ( Params::GetConfigBool("USESYSLOG") == false ) restartchilds = true;
        }

        //Send restart signal to childs if needed
        if ( restartchilds )
        {
            restartchilds = false;

            killpg(getpgid(0), SIGUSR1);
        }

        //Clean proxyhandler zombies
        while (waitpid(-1, &status, WNOHANG) > 0)
        {
            Instances--;
        }

        while ((startchild > 0) || (Instances < servernumber))
        {
            if ( (pid = fork()) < 0 ) //Fork Error
            {
                //Too many processes or out of memory?
                LogFile::ErrorMessage("Could not fork proxychild: %s\n", strerror(errno));

                //Lets hope the the causing error goes away soon
                sleep(10);
            }
            else if ( pid == 0 ) //Child
            {
                //Install ProxyHandler Signals
                if ( InstallSignal(1) < 0 )
                {
                    LogFile::ErrorMessage("Error installing ProxyHandler signals\n");
                    sleep(10);
                    exit(1);
                }

                //Start processing requests
                Proxy.Proxy( ProxyServer );
				Instances--;
                exit(1);
            }
            else //Parent
            {
                if ( startchild > 0 ) startchild--;
                Instances++;
            }
        }

        //Do we need more proxy-processes?
        if ( Instances >= servernumber )
        {
            bool hangup = ProxyServer.CheckForData(0);
            sleep(1);

            if ( (hangup == true) && (Instances < maxservers) )
            {
                //Did a old process take care or is there still data? Create two if needed
                if ( ProxyServer.CheckForData(0) )
                {
                    if (LL>0) LogFile::ErrorMessage("All childs busy, spawning new (now: %d) - SERVERNUMBER might be too low\n", Instances+2);
                    startchild += 2;
                }
            }
        }
    }

    return 0;
}

