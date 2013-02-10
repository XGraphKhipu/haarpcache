/* haarp/default.h.  Generated from default.h.in by configure.  */
#ifndef DEFAULT_H
#define DEFAULT_H

#define VERSION "1.0"

//##############################################################
//Parameters in Configurationfile

#define CONFIGPARAMS \
 "USER","GROUP", \
 "SERVERNUMBER","PORT","BIND_ADDRESS","SOURCE_ADDRESS", "PIDFILE", \
 "DAEMON","TRANSPARENT","LOG_OKS","ACCESSLOG","ERRORLOG","LOGLEVEL", \
 "USESYSLOG","SYSLOGNAME","SYSLOGFACILITY","SYSLOGLEVEL", "DISPLAYINITIALMESSAGES", \
 "PARENTPROXY","PARENTPORT","MAXSERVERS","FORWARDED_IP","X_FORWARDED_FOR", \
 "CACHEDIR","PLUGINSDIR","MYSQL_HOST","MYSQL_USER","MYSQL_PASS","MYSQL_DB","CACHE_LIMIT"


//##############################################################
//Configuration not setable in havp.config

//Time format
#define TIMEFORMAT "%d/%m/%Y %H:%M:%S "

//CONNTIMEOUT in seconds
#define CONNTIMEOUT 60

//RECVTIMEOUT in seconds
#define RECVTIMEOUT 120

//SENDTIMEOUT in seconds
#define SENDTIMEOUT 120

//Maximum client connection waiting for accept
#define MAXCONNECTIONS 1024

//Maximum bytes received in one request
#define MAXRECV 14600

//Maximum logfile line length
#define STRINGLENGTH 1000

//Valid Methods
#define METHODS \
 "GET","POST","HEAD","CONNECT","PUT","TRACE","PURGE","OPTIONS","UNLOCK", \
 "SEARCH","PROPFIND","BPROPFIND","PROPPATCH","BPROPPATCH","MKCOL","COPY", \
 "BCOPY","MOVE","LOCK","BMOVE","DELETE","BDELETE","SUBSCRIBE","UNSUBSCRIBE", \
 "POLL","REPORT","ERROR","NONE","MKACTIVITY","CHECKOUT","MERGE"

//Maximum length of http headers
#define MAXHTTPHEADERLENGTH 65536

// DONT TOUCH - run configure
#define CONFIGFILE "/etc/haarp/haarp.conf"
#define ACCESSLOG "/var/log/haarp/access.log"
#define ERRORLOG "/var/log/haarp/error.log"
#define PIDFILE "/var/run/haarp/haarp.pid"
/* #undef SSLTUNNEL */
#define HAVE_SETGROUPS 1
#define HAVE_INITGROUPS 1

#endif
