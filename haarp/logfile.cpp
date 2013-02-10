#include "default.h"
#include "logfile.h"
#include "params.h"
#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <fcntl.h>
#include <syslog.h>
#include <map>
#include <cstdlib>
#include <cstring>
#include <sstream>

#define LOGSTRINGLENGTH 5000

int LogFile::Access_fd = -1;
int LogFile::Error_fd = -1;
bool LogFile::UseSyslog = false;
int LogFile::SyslogLevel;
extern int LOK; //Logok

//Open access and error logfiles
bool LogFile::InitLogFiles( const char *AccessLogFileT, const char *ErrorLogFileT )
{
    if ( Params::GetConfigBool("USESYSLOG") )
    {
        //Already open?
        if ( UseSyslog ) return true;

        UseSyslog = true;
        SyslogLevel = GetSyslogLevel();

        openlog(Params::GetConfigString("SYSLOGNAME").c_str(), LOG_CONS | LOG_PID, GetSyslogFacility());

        return true;
    }

    if ( Error_fd > -1 ) close(Error_fd);
    if ( Access_fd > -1 ) close(Access_fd);

    if ( (Error_fd = open(ErrorLogFileT, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP)) < 0)
    {
        return false;
    }

    if ( (Access_fd = open(AccessLogFileT, O_WRONLY|O_APPEND|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP)) < 0)
    {
        return false;
    }

    return true;
}


//Log access messages
void LogFile::AccessMessage( const char *formatT , ... )
{
    if ( LOK ) {
        char str[LOGSTRINGLENGTH+1];

        va_list args;
        va_start(args, formatT);
        vsnprintf(str, LOGSTRINGLENGTH, formatT, args);
        va_end(args);
    
        if ( UseSyslog )
        {
            syslog(SyslogLevel, "%s", str);
        }
        else
        {
            std::ostringstream sin;
            sin << time(NULL) << ".000 ("<<getpid()<<") " << str;
	            
        	write(Access_fd, sin.str().c_str(), sin.str().size());
        }
    }
}

//Log error messages
void LogFile::ErrorMessage( const char *formatT , ... )
{
    if ( LOK ) {
        char str[LOGSTRINGLENGTH+1];

        va_list args;
        va_start(args, formatT);
        vsnprintf(str, LOGSTRINGLENGTH, formatT, args);
        va_end(args);

        if ( UseSyslog )
        {
            syslog(SyslogLevel, "%s", str);
        }
        else
        {
            char strt[LOGSTRINGLENGTH+1];
            char tmpdate[51];

            time_t now = time(NULL);
            struct tm TmDate = *localtime(&now);
            strftime(tmpdate, 50, TIMEFORMAT, &TmDate);

            strncpy(strt, tmpdate, sizeof(tmpdate));
            strncat(strt, str, LOGSTRINGLENGTH - sizeof(tmpdate) - 2);

            write(Error_fd, strt, strlen(strt));
        }
    }
}

int LogFile::GetSyslogLevel()
{
    string Level = UpperCase(Params::GetConfigString("SYSLOGLEVEL"));
    SearchReplace( Level, "LOG_", "" );

    if ( Level == "EMERG") return LOG_EMERG;
    if ( Level == "ALERT" ) return LOG_ALERT;
    if ( Level == "CRIT" ) return LOG_CRIT;
    if ( Level == "ERR" ) return LOG_ERR;
    if ( Level == "WARNING" ) return LOG_WARNING;
    if ( Level == "WARN" ) return LOG_WARNING;
    if ( Level == "NOTICE" ) return LOG_NOTICE;
    if ( Level == "INFO" ) return LOG_INFO;
    if ( Level == "DEBUG" ) return LOG_DEBUG;

    return LOG_INFO;
}

int LogFile::GetSyslogFacility()
{
    string Facility = UpperCase(Params::GetConfigString("SYSLOGFACILITY"));
    SearchReplace( Facility, "LOG_", "" );

    if ( Facility == "AUTH" ) return LOG_AUTH;
    if ( Facility == "CRON" ) return LOG_CRON;
    if ( Facility == "DAEMON" ) return LOG_DAEMON;
    if ( Facility == "KERN" ) return LOG_KERN;
    if ( Facility == "LOCAL0" ) return LOG_LOCAL0;
    if ( Facility == "LOCAL1" ) return LOG_LOCAL1;
    if ( Facility == "LOCAL2" ) return LOG_LOCAL2;
    if ( Facility == "LOCAL3" ) return LOG_LOCAL3;
    if ( Facility == "LOCAL4" ) return LOG_LOCAL4;
    if ( Facility == "LOCAL5" ) return LOG_LOCAL5;
    if ( Facility == "LOCAL6" ) return LOG_LOCAL6;
    if ( Facility == "LOCAL7" ) return LOG_LOCAL7;
    if ( Facility == "LPR" ) return LOG_LPR;
    if ( Facility == "MAIL" ) return LOG_MAIL;
    if ( Facility == "NEWS" ) return LOG_NEWS;
    if ( Facility == "USER" ) return LOG_USER;
    if ( Facility == "UUCP" ) return LOG_UUCP;

    return LOG_DAEMON;
}
