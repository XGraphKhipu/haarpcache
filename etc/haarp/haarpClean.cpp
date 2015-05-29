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
#define NUM_FILES 500

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
	bool limit_surpassed=false;
	double expected_deletion=0;
	double disk_occupied_space;
	double disk_space;
	double use_percent;

	Params::SetParams(carg,varg);
	cachedir = Params::GetConfigString("CACHEDIR");
	cachelimit = Params::GetConfigString("CACHE_LIMIT");
	host = Params::GetConfigString("MYSQL_HOST");
	user = Params::GetConfigString("MYSQL_USER");
	pass = Params::GetConfigString("MYSQL_PASS");
	db = Params::GetConfigString("MYSQL_DB");

        char fecha_request[100];
	time_t temp;
        struct tm *timeptr;

	// Read config, check space usage and limits

	stringexplode(cachedir,"|",&list_dir);
        for(int i=0;i < (int)list_dir.size();i++) {

		disk_occupied_space=disk_occupation(list_dir.at(i));
		disk_space=disk_size(list_dir.at(i));
		use_percent=disk_use(list_dir.at(i));
		printf("Disk size : %.2lf MB, %.2lf GB\n",disk_space/(1048576),disk_space/(1073741824));
		cout<<"Disk occupation percentage on partition hosting cache directory "<<list_dir[i]<<" : "<<use_percent<<"%"<<endl;
                if( use_percent >= atof(cachelimit.c_str()) ) {
			expected_deletion=(use_percent - atof(cachelimit.c_str()))*disk_space/100;
                	limit_surpassed=true;
			printf("Disk space occupied over cache limit : %.2lf MB (%.2lf GB) %.2lf %% above limit in/etc/haarp/haarp.conf\n",expected_deletion/1024,expected_deletion/1073741824,use_percent - atof(cachelimit.c_str()));
			} // if( use_percent >= atoi(cachelimit.c_str()) )
        }
	if (!limit_surpassed) {
		cout<<"Ocupation of cache directories within configuration limits. Exiting..."<<endl;
		return 1;
	}

	// BD connection
	MYSQL *connect;
	char *file = charmalloc(NUM_FILES);
	char *query = charmalloc(NUM_FILES);
	connect = mysql_init(NULL);
        if(!mysql_real_connect(connect,host.c_str(),user.c_str(),pass.c_str(),db.c_str(),MYSQL_PORT,NULL,0)) {
                cout<<"MYSQL Error: "<<mysql_error(connect)<<endl;
                return 1;
        }

	// deleting entries
	for(int directorio=0; directorio < (int)list_dir.size(); directorio++) {

	// these values define the progression to select entries
	int factor=12;
	int days=60;
	int requested = 0;
	double deleted=0;
	MYSQL_RES *res;
	MYSQL_ROW r;
	int num_entries = 0;
	int nloop;

	do {
	        temp = time(NULL);
	        timeptr = localtime(&temp);
	        timeptr->tm_mday-=days;
	        temp=mktime(timeptr);
	        strftime(fecha_request,sizeof(fecha_request),"%Y-%m-%d %T", timeptr);
		nloop=0;
		cout<<"Selecting files last accesed before "<<fecha_request<<", requested less than "<<requested<<" times"<<endl;
		sprintf(query, "select domain,file,last_request,filesize,requested from haarp where deleted=0 and requested <= %i and last_request <= '%s' order by last_request,requested limit %i", requested, fecha_request, NUM_FILES);
		if(mysql_query(connect, query)) {
                        cout<<"MYSQL Error: Query: '"<<query<<"'; "<<mysql_error(connect)<<endl;
                        break;
                }
		//mysql_query(connect,"select domain,file,last_request,filesize,requested from haarp where deleted=0 order by last_request, requested, filesize DESC limit 10");
		res = mysql_store_result(connect);

		while ((r = mysql_fetch_row(res)) != NULL && use_percent >= atof(cachelimit.c_str()) && deleted <= expected_deletion) {
			nloop++;
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
			deleted +=atof(r[3]);
			if (nloop >=NUM_FILES) printf("Entry %i (loop %i), %.2lfMB (of %.2lfGB total), Last requested : %s [Hits: %s], Path: %s\n", num_entries, nloop, atof(r[3])/1048576, deleted/1073741824,r[2], r[4], file);
			system(query);
			num_entries++;
		} // ((r = mysql_fetch_row(res)) != NULL && disk_use(list_dir.at(directorio)) >= atoi(cachelimit.c_str()))

		if (deleted > expected_deletion) {
			cout<<"Finished, no need to delete more files."<<endl;
			break;
		}

		if (nloop == 0 && requested >= 1000) {
			cout<<"Finished, no more files to delete."<<endl;
			break;
		}

		if (r == NULL && (nloop == NUM_FILES || nloop == 0)) {
			// defining request
			requested=requested*2;
			if (requested == 0)
				requested=1;
			else if (requested == 2)
				requested=5;
				else if (requested > 20)
					requested=1000;
			// defining new date
			factor=factor/2;
			days=5*factor;
			if (requested > 10)
				days=1*(requested == 20);
		}

	use_percent=disk_use(list_dir.at(directorio));
	} while ( use_percent >= atof(cachelimit.c_str()) ); // do

	cout<<"Entries deleted : "<<num_entries<<endl;

	if (deleted > 1073741824)
		printf("Total Eliminado: %.2lf GB\n", deleted/1073741824);
	else
		printf("Total Eliminado: %.2lf MB\n", deleted/1048576);

	} // for(int i=0;i < (int)list_dir.size();i++)

	return 1;
}
