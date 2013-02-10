#ifndef LOGFILE_H
#define LOGFILE_H

#include <string>

using namespace std;

class LogFile {

private:

static int Error_fd;
static int Access_fd;

static bool UseSyslog;
static int SyslogLevel;

static int GetSyslogLevel();
static int GetSyslogFacility();

public:

static bool InitLogFiles( const char *AccessLogFileT, const char *ErrorLogFileT );
static void AccessMessage( const char *formatT, ... );
static void ErrorMessage( const char *formatT, ... );
   
};

#endif
