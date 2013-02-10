#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <map>

using namespace std;

class Params {

private:

static map <string,string> params;

static bool ReadConfig( string file );
static void ShowConfig( string cfgfile );
static bool TestConfig();
static void Usage();
static void SetDefaults();

public:

static bool SetParams( int argcT, char* argv[] );
static void SetConfig( string key, string val );
static bool GetConfigBool( string key );
static string GetConfigString( string key );
static int GetConfigInt( string key );

};

#endif
