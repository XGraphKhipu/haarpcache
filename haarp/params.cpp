#include "default.h"
#include "params.h"
#include "utils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <cstdlib>

#ifndef INADDR_NONE
#define INADDR_NONE ((unsigned long) -1)
#endif

map <string,string> Params::params;

void Params::SetDefaults()
{
    SetConfig("CACHEDIR", "/haarp");
    SetConfig("PLUGINSDIR", "/etc/haarp/plugins");
    SetConfig("MYSQL_HOST", "localhost");
    SetConfig("MYSQL_USER", "root");
    SetConfig("MYSQL_PASS", "");
    SetConfig("MYSQL_DB", "haarp");
    SetConfig("CACHE_LIMIT", "98");
    SetConfig("DISPLAYINITIALMESSAGES", "true");
    SetConfig("USER",		"root");
    SetConfig("GROUP",		"root");
    SetConfig("DAEMON",		"true");
    SetConfig("SERVERNUMBER",	"8");
    SetConfig("MAXSERVERS",	"150");
    SetConfig("PORT",		"8080");
    SetConfig("BIND_ADDRESS",	"");
    SetConfig("SOURCE_ADDRESS",	"");
    SetConfig("PARENTPROXY",	"");
    SetConfig("PARENTPORT",	"0");
    SetConfig("ACCESSLOG",	ACCESSLOG);
    SetConfig("ERRORLOG",	ERRORLOG);
    SetConfig("LOG_OKS",	"true");
    SetConfig("LOGLEVEL",	"0");
    SetConfig("USESYSLOG",	"false");
    SetConfig("SYSLOGNAME",	"haarp");
    SetConfig("SYSLOGFACILITY",	"daemon");
    SetConfig("SYSLOGLEVEL",	"info");
    SetConfig("PIDFILE",	PIDFILE);
    SetConfig("TRANSPARENT",	"false");
    SetConfig("FORWARDED_IP",	"false");
    SetConfig("X_FORWARDED_FOR","false");
}

bool Params::ReadConfig( string file )
{
    ifstream input( file.c_str() );

    if ( !input )
    {
        cerr << "Could not open config file: " << file << endl;
        return false;
    }

    string::size_type Position;
    string line, key, val;

    while ( input )
    {
        getline( input, line );

        //Strip whitespace from beginning and end
        if ( (Position = line.find_first_not_of(" \t")) != string::npos )
        {
            line = line.substr(Position, (line.find_last_not_of(" \t", string::npos) - Position) + 1);
        }

        //Read next if nothing found
        if ( (Position == string::npos) || (line.size() == 0) ) continue;

        //Read next if commented
        if ( line.substr(0, 1) == "#" ) continue;

        //Find key and value
        if ( (Position = line.find_first_of(" \t")) != string::npos )
        {
            key = line.substr(0, Position);

            if ( (Position = line.find_first_not_of(" \t", Position + 1)) == string::npos )
            {
                cout << "Invalid Config Line: " << line << endl;
                return false;
            }

            val = line.substr( Position );

            Params::SetConfig( key, val );
        }
        else
        {
            cout << "Invalid Config Line: " << line << endl;
            return false;
        }
    }

    input.close();

    return true;
}

void Params::SetConfig( string param, string value )
{
    //string TempParams[] = {CONFIGPARAMS};
    //bool ParamFound = false;

    param = UpperCase(param);
    /*
    for ( unsigned int i = 0; i < sizeof(TempParams)/sizeof(string); i++ )
    {
        if ( param == TempParams[i] )
        {
            ParamFound = true;
        }
    }
    
    if ( ParamFound )
    {
    */
        if ( UpperCase(value) == "TRUE" || UpperCase(value) == "FALSE" )
        {
            value = UpperCase(value);
        }

        params[param] = value;
    /*
    }
    else
    {
        cout << "Unknown Config Parameter: " << param << endl;
        cout << "Exiting.." << endl;
        exit(1);
    }
    */
}

int Params::GetConfigInt( string param )
{
    return atoi( params[param].c_str() );
}

bool Params::GetConfigBool( string param )
{
    if ( params[param] == "TRUE" )
    {
        return true;
    }
    else
    {
        return false;
    }
}

string Params::GetConfigString( string param )
{
    return params[param];
}

void Params::ShowConfig( string cfgfile )
{
    cout << endl << "# Using Haarp config: " << cfgfile << endl << endl;
    typedef map<string,string>::const_iterator CI;
    for(CI p = params.begin(); p != params.end(); ++p)
    {
        cout << p->first << "=" << p->second << '\n';
    }
    cout << endl;
}

void Params::Usage()
{
    cout << endl << "Usage: haarp [Options]" << endl << endl;
    cout << "Haarp Version " << VERSION << endl << endl;
    cout << "Possible options are:" << endl;
    cout << "--help | -h                         This pamphlet" << endl;
    cout << "--conf-file=FileName | -c Filename  Use this Config-File" << endl << endl;
}

bool Params::SetParams( int argvT, char* argcT[] )
{
    string option, value;
    string::size_type i1, i2;

    string cfgfile = CONFIGFILE;
    bool showconf = false;

    SetDefaults();

    while ( --argvT )
    {
        value = *++argcT;
        i1 = value.find_first_not_of("-");

        //No GNU options
        if ( i1 == 1 )
        {
            option = value.substr(i1, 1);

            if ( option == "c" )
            {
                --argvT;

                if ( argvT == 0 )
                {
                    Usage();
                    return false;
                }
                value = *++argcT;
            }
            else if ( option == "s" ) //
            {
                showconf = true;
            }
            else
            {
                Usage();
                return false;
            }
        }
        //GNU options
        else if ( i1 == 2 )
        {
            if ( (i2 = value.find("=")) != string::npos )
            {
                option = value.substr(i1, i2 - i1);

                if ( value.size() > i2 + 1 )
                {
                    value = value.substr(i2 + 1);
                }
                else
                {
                    Usage();
                    return false;
                }
            }
            else
            {
                option = value.substr(i1);
                value = "";
            }
        }
        else
        {
            Usage();
            return false;
        }

        if ( option == "help" )
        {
            Usage();
            return false;
        }
        else if ( option == "conf-file" || option == "c" )
        {
            if (value == "")
            {
                Usage();
                return false;
            }

            cfgfile = value;
        }
        else
        {
            Usage();
            return false;
        }
    }

    if ( ReadConfig( cfgfile ) == false )
    {
        return false;
    }

    return TestConfig();
}

bool Params::TestConfig()
{
    //Test that some options are sane
    if ( Params::GetConfigInt("SERVERNUMBER") < 1 )
    {
        cout << "Invalid Config: SERVERNUMBER needs to be greater than 0" << endl;
        return false;
    }
    if ( Params::GetConfigString("ACCESSLOG").substr(0,1) != "/" || Params::GetConfigString("ERRORLOG").substr(0,1) != "/" )
    {
        cout << "Invalid Config: Log paths need to be absolute" << endl;
        return false;
    }
    if ( Params::GetConfigString("BIND_ADDRESS") == "NULL" ) Params::SetConfig("BIND_ADDRESS","");
    if ( Params::GetConfigString("BIND_ADDRESS") != "" )
    {
        if ( inet_addr( Params::GetConfigString("BIND_ADDRESS").c_str() ) == INADDR_NONE )
        {
            cout << "Invalid Config: Invalid BIND_ADDRESS" << endl;
            return false;
        }
    }
    if ( Params::GetConfigString("SOURCE_ADDRESS") == "NULL" ) Params::SetConfig("SOURCE_ADDRESS","");
    if ( Params::GetConfigString("SOURCE_ADDRESS") != "" )
    {
        if ( inet_addr( Params::GetConfigString("SOURCE_ADDRESS").c_str() ) == INADDR_NONE )
        {
            cout << "Invalid Config: Invalid SOURCE_ADDRESS" << endl;
            return false;
        }
    }
    if ( Params::GetConfigString("PARENTPROXY") != "" && Params::GetConfigInt("PARENTPORT") < 1 )
    {
        cout << "Invalid Config: Invalid PARENTPROXY/PARENTPORT" << endl;
        return false;
    }

    return true;
}
