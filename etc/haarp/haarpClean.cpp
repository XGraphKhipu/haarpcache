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
#include <list>

//#include "database_mysql.cpp"
#define charmalloc(a) (char *)malloc(a*sizeof(char)) 
#define NUM_FILES 5000

#define pb push_back

using namespace std;

void SearchReplace2(string &source, string search, string replace) {
    string::size_type position = source.find(search);
    while (position != string::npos) {
        source.replace(position, search.size(), replace);
        position = source.find(search,position+replace.length());
    }
}
class ldir{
	public:
		string dir;
		double mb_to_delete;
};
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
    list<ldir> list_dir_clean;
    list<ldir>::iterator index_dir;
    
	bool limit_surpassed=false;
	double expected_deletion=0;
	//double disk_occupied_space;
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
	ldir ldirec;
	// Read config, check space usage and limits

	stringexplodetrim(cachedir,"|",&list_dir);
        for(int i=0;i < (int)list_dir.size();i++) {
			//disk_occupied_space = disk_occupation(list_dir.at(i));
			cachedir = list_dir.at(i); 
			disk_space = disk_size(cachedir);
			use_percent = disk_use(cachedir);

			printf("Disk size : %.2lf MB, %.2lf GB\n",disk_space/(1048576),disk_space/(1073741824));
			cout<<"Disk occupation percentage on partition hosting cache directory "<<cachedir<<" : "<<use_percent<<"%"<<endl;
			if( use_percent >= atof(cachelimit.c_str()) ) {
				expected_deletion=(use_percent - atof(cachelimit.c_str()))*disk_space/100;
				
				ldirec.mb_to_delete = expected_deletion/1048576;
				ldirec.dir = cachedir;
				list_dir_clean.pb(ldirec);
				
				limit_surpassed=true;
				printf("In the disk: %s, space occupied over cache limit : %.2lf MB (%.2lf GB) %.2lf %% above limit in /etc/haarp/haarp.conf (%s %%)\n", cachedir.c_str(), expected_deletion/1048576, expected_deletion/1073741824, use_percent - atof(cachelimit.c_str()), cachelimit.c_str());
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
	//~ for(int directorio=0; directorio < (int)list_dir.size(); directorio++) {

	// these values define the progression to select entries
	float factor=1.5;
	int days=60;
	int day0=60;
	int requested = 0;
	int last_requested = 0;
	int limit_inf = 0;
	double deleted=0;
	MYSQL_RES *res;
	MYSQL_ROW r;
	int num_entries = 0;
	int nloop;
	string jump = "";
	while(true)
	{
		temp = time(NULL) - days*24*3600;
		timeptr = localtime(&temp);
		temp=mktime(timeptr);
		strftime(fecha_request,sizeof(fecha_request),"%Y-%m-%d %T", timeptr);
		nloop = 0;
		cout<<"Selecting files last accesed before "<<fecha_request<<", requested less than "<<requested<<" times"<<endl;
		sprintf(query, "select domain,file,last_request,filesize,requested from haarp where deleted=0 and requested <= %i and last_request <= '%s' order by requested, last_request limit %i,%i", requested, fecha_request, limit_inf, NUM_FILES);
		//~ cout<<"my query:"<<query<<endl;
		if(mysql_query(connect, query)) {
				cout<<"MYSQL Error: Query: '"<<query<<"'; "<<mysql_error(connect)<<endl;
				break;
		}
		//mysql_query(connect,"select domain,file,last_request,filesize,requested from haarp where deleted=0 order by last_request, requested, filesize DESC limit 10");
		res = mysql_store_result(connect);
		jump = "";
		while ( (r = mysql_fetch_row(res)) != NULL && !list_dir_clean.empty() ) {
			nloop++;
			strcpy(file,"\0");
			subdir = ConvertChar(r[1]);
			for(index_dir = list_dir_clean.begin();index_dir != list_dir_clean.end();index_dir++) {
				sprintf(file,"%s%s/%s/%s",((*index_dir).dir).c_str(),r[0],subdir.c_str(),r[1]);
				if(file_exists(string(file)))
					break;
				else
					strcpy(file,"\0");
			}
			if(!strlen(file)) {
				if(nloop % 20 == 0) cout<<".";
				jump = "\n";
				continue;
			}
			else {
				cout<<jump;
				jump = "";
			}
			//sprintf(query,"update haarp set deleted=1 where file='%s' and domain='%s'",sqlconv(r[1]).c_str(),r[0]);
			sprintf(query,"DELETE FROM haarp where file='%s' and domain='%s'",sqlconv2(r[1]).c_str(),r[0]);
			if(mysql_query(connect,query)) {
				cout<<"MYSQL Error: Query: '"<<query<<"'; "<<mysql_error(connect)<<endl;
				continue;
			}
			sprintf(query,"rm -f %s",file);
			deleted +=atof(r[3]);
			num_entries++;
			if (num_entries % 100 == 0 ) 
				printf("Entry %i, %.2lfMB (of %.2lfGB total), Last requested : %s [Hits: %s], Path: %s\n", num_entries, atof(r[3])/1048576, deleted/1073741824,r[2], r[4], file);
			system(query);
			(*index_dir).mb_to_delete -= deleted/1048576;
			if((*index_dir).mb_to_delete <= 0) {
				list_dir_clean.erase(index_dir);
			}
		}
		cout<<jump;
		if (list_dir_clean.empty()) {
			cout<<"Finished, no need to delete more files."<<endl;
			break;
		}

		if (nloop == 0 && requested >= 1000) {
			cout<<"Finished, no more files to delete."<<endl;
			break;
			//~ day0 -= 15;
		}
		
		if ( nloop != NUM_FILES || !nloop ) {
			// defining request
			requested=requested*2;
			if (requested == 0)
				requested=1;
			else if (requested == 2)
				requested=5;
			else if (requested == 10)
				requested=20;
			// defining new date
			factor=factor*2;
			days = day0 + ceil(5*factor);
			//~ if (requested > 20)
				//~ requested = 1000;
		}
		if(last_requested == requested) {
			limit_inf += NUM_FILES;
		}
		else {
			limit_inf = 0;
			last_requested = requested;
		}
			
		//~ use_percent=disk_use(list_dir.at(directorio));
	}

	cout<<"Entries deleted : "<<num_entries<<endl;

	if (deleted > 1073741824)
		printf("Total Eliminado: %.2lf GB\n", deleted/1073741824);
	else
		printf("Total Eliminado: %.2lf MB\n", deleted/1048576);

	//~ } // for(int i=0;i < (int)list_dir.size();i++)

	return 1;
}
