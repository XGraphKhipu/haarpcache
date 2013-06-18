/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 *
 */

#include "../../haarp/utils.h"
#include "../../haarp/params.h"

#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <string>
#include <vector>
#include <sys/types.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <iostream>
#include <math.h>
#include <cstdlib>
#include <utime.h>
#include <sys/statvfs.h>
//#include <blkid/blkid.h>
#include <fstab.h>
#include <cstdio>
#include <dirent.h>

//#include "database_mysql.cpp"
#define charmalloc(a) (char *)malloc(a*sizeof(char)) 
#define NUM_FILES 600
#define MAX_MB_DELETE 1000

using namespace std;

void SearchReplace2(string &source, string search, string replace) {
    string::size_type position = source.find(search);
    while (position != string::npos) {
        source.replace(position, search.size(), replace);
        position = source.find(search,position+replace.length());
    }
}

string sqlconv2(string sql) {
	SearchReplace2(sql,"'","\\'");
	SearchReplace2(sql,"$","\\$");
	SearchReplace2(sql,";","\\;");
	SearchReplace2(sql,"\"","\\\"");
	return sql;
}

int main(int carg, char **varg) {
	string cachedir,cachelimit,directory,completefilepath, subdir,host,user,pass,db;
        vector<string> list_dir;
        
	Params::SetParams(carg,varg);
	cachedir = Params::GetConfigString("CACHEDIR");
	cachelimit = Params::GetConfigString("CACHE_LIMIT");
	host = Params::GetConfigString("MYSQL_HOST");
	user = Params::GetConfigString("MYSQL_USER");
	pass = Params::GetConfigString("MYSQL_PASS");
	db = Params::GetConfigString("MYSQL_DB");

	stringexplode(cachedir,"|",&list_dir);
	
        for(int i=0;i < (int)list_dir.size();i++) {
                if( disk_use(list_dir.at(i)) < atoi(cachelimit.c_str()) ) {
			cout<<"The cache not are in the limit!."<<endl;
                	return 1;
                }
        }
	
	MYSQL *connect;
	char *file = charmalloc(400);
	char *query = charmalloc(400);
	connect = mysql_init(NULL);
        if(!mysql_real_connect(connect,host.c_str(),user.c_str(),pass.c_str(),db.c_str(),MYSQL_PORT,NULL,0)) {
                cout<<"MYSQL Error: "<<mysql_error(connect)<<endl;
                return 1;
        }

	int requested = 0;
	double megas = 0;
	MYSQL_RES *res;
	MYSQL_ROW r;
	int i = 0;
	int cont = 1;
	while( true ) {
		sprintf(query, "select domain,file,last_request,filesize,requested from haarp where deleted=0 and requested=%i order by last_request, filesize DESC limit %i", requested, NUM_FILES);
		if(mysql_query(connect, query)) {
                        cout<<"MYSQL Error: Query: '"<<query<<"'; "<<mysql_error(connect)<<endl;
                        break;
                }

		//mysql_query(connect,"select domain,file,last_request,filesize,requested from haarp where deleted=0 order by last_request, requested, filesize DESC limit 10");
	
		res = mysql_store_result(connect);
		while( (r = mysql_fetch_row(res)) != NULL)
		{
			strcpy(file,"\0");
			subdir = ConvertChar(r[1]);
			for(int j = 0;j < (int)list_dir.size();j++) {
				sprintf(file,"%s%s/%s/%s",(list_dir.at(j)).c_str(),r[0],subdir.c_str(),r[1]);
				if(file_exists(string(file)))		
					break;
			}
			if(!file)
				continue;
			//sprintf(query,"update haarp set deleted=1 where file='%s' and domain='%s'",sqlconv(r[1]).c_str(),r[0]);
			sprintf(query,"DELETE FROM haarp where file='%s' and domain='%s'",sqlconv2(r[1]).c_str(),r[0]);
			if(mysql_query(connect,query)) {
				cout<<"MYSQL Error: Query: '"<<query<<"'; "<<mysql_error(connect)<<endl;
				continue;
			}
			sprintf(query,"rm -f %s",file);
			double consumo = atoi(r[3])/(1024.0*1024);
			megas += consumo;
			printf("(%i) Deleting ... [%.2lfMB] %s [Hits: %s] Path: %s\n", i,  consumo, r[2], r[4], file);
			system(query);
			i++;
			if(megas >= MAX_MB_DELETE)
				break;
		}
		if(megas < MAX_MB_DELETE) {
			if( i >= NUM_FILES*cont ) {
				cont++;
				continue;	
			}	
			else
				requested++;	
		}
		else
			break;
	}
	if(megas > 1024)
		printf("Total Eliminado: %.2lf GB\n", megas/1024.0);
	else
		printf("Total Eliminado: %.2lf MB\n",megas);
	return 1;
}
