#ifndef HELPER_H
#define HELPER_H

#include <unistd.h>
#include <string>

bool MakeDaemon();
bool ChangeUserAndGroup( string usr, string grp );
string GetUser();
string GetGroup();
bool WritePidFile( pid_t pid );
int InstallSignal( int level );

#endif
