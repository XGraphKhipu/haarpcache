#include "default.h"
#include "params.h"
#include "logfile.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>

static void ChildExited( int SignalNo )
{
    //Handle with waitpid() in haarp.cpp
}

static void ChildChildExited( int SignalNo )
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0);
}

static void RereadAll( int SignalNo )
{
    extern bool rereadall;
    rereadall = true;
}

static void RestartChild( int SignalNo )
{
    extern bool childrestart;
    childrestart = true;
}

static void ExitProcess( int SignalNo )
{
    pid_t pgid = getpgid(0);

    //PSE: all processes have same pgid!
    if (getpid() == pgid)
    {
	//PSE: only parent, no scan-file to delete!!
	killpg(pgid,SIGINT);

	//Delete pidfile
	while (unlink(Params::GetConfigString("PIDFILE").c_str()) < 0 && (errno == EINTR || errno == EBUSY));
    }

    //End process
    exit(0);
}


//Install Signal Handlers for different fork levels
int InstallSignal( int level )
{
    struct sigaction Signal;
    memset(&Signal, 0, sizeof(Signal));
    Signal.sa_flags = 0;

    //Level 0 = Main Havp Process
    //Level 1 = ProxyHandler Process
    //Level 2 = Scanner Process
    //Signals are inherited from previous level at forking..

    if ( level == 0 ) //Main Havp Process
    {
        Signal.sa_handler = ExitProcess;
        if (sigaction(SIGINT, &Signal, NULL) != 0) return -1;
        if (sigaction(SIGTERM, &Signal, NULL) != 0) return -1;

        Signal.sa_handler = RereadAll;
        if (sigaction(SIGHUP, &Signal, NULL) != 0) return -1;
        //Compatibility for 0.77 and older init-script
        if (sigaction(SIGUSR2, &Signal, NULL) != 0) return -1;

        Signal.sa_handler = ChildExited;
        if (sigaction(SIGCHLD, &Signal, NULL) != 0) return -1;

        Signal.sa_handler = SIG_IGN;
        if (sigaction(SIGUSR1, &Signal, NULL) != 0) return -1;
        if (sigaction(SIGPIPE, &Signal, NULL) != 0) return -1;
    }
    else if ( level == 1 ) //ProxyHandler Process
    {
        Signal.sa_handler = RestartChild;
        if (sigaction(SIGUSR1, &Signal, NULL) != 0) return -1;

        Signal.sa_handler = ChildChildExited;
        if (sigaction(SIGCHLD, &Signal, NULL) != 0) return -1;

        Signal.sa_handler = SIG_IGN;
        if (sigaction(SIGHUP, &Signal, NULL) != 0) return -1;
        if (sigaction(SIGUSR2, &Signal, NULL) != 0) return -1;
    }


    return 0;
}


bool MakeDaemon()
{
    pid_t daemon = fork();

    if ( daemon < 0 )
    {
        return false;
    }
    else if (daemon != 0)
    {
        //Exit Parent
        exit(0);
    }
    //Child

    setsid();
    chdir("/tmp/");
    umask(077);

    //Close stdin/stdout/stderr
    close(0);
    close(1);
    close(2);

    return true;
}

bool ChangeUserAndGroup( string usr, string grp )
{
    if ( geteuid() != 0 ) return true;

    if ( usr == "" || grp == "" )
    {
        cout << "You must define User and Group" << endl;
        return false;
    }

    struct passwd *user;
    struct group *my_group;

    if ( (user = getpwnam( usr.c_str() )) == NULL )
    {
        cout << "User does not exist: " << usr << endl;
        cout << "You need to: useradd " << usr << endl;
        return false;
    }

    if ( (my_group = getgrnam( grp.c_str() )) == NULL )
    {
        cout << "Group does not exist: " << grp << endl;
        cout << "You need to: groupadd " << grp << endl;
        return false;
    }

#ifdef HAVE_INITGROUPS
    if ( initgroups( usr.c_str(), user->pw_gid ) )
    {
        cout << "Group initialization failed (initgroups)" << endl;
        return false;
    }
#else
#if HAVE_SETGROUPS
    if ( setgroups(1, &user->pw_gid) )
    {
        cout << "Group initialization failed (setgroups)" << endl;
        return false;
    }
#endif
#endif

    if ( setgid( my_group->gr_gid ) < 0 )
    {
        cout << "Could not change group to: " << grp << endl;
        return false;
    }

    if ( setuid( user->pw_uid ) < 0 )
    {
        cout << "Could not change user to: " << usr << endl;
        return false;
    }

    return true;
}

string GetUser()
{
    struct passwd *user = getpwuid( geteuid() );
    if ( user == NULL ) return "<error>";
    return (string)user->pw_name;
}

string GetGroup()
{
    struct group *my_group = getgrgid( getegid() );
    if ( my_group == NULL ) return "<error>";
    return (string)my_group->gr_name;
}

bool WritePidFile( pid_t havp_pid )
{
    ofstream pidf( Params::GetConfigString("PIDFILE").c_str(), ios_base::trunc );

    if ( !pidf ) return false;

    pidf << havp_pid << endl;

    pidf.close();

    return true;
}

