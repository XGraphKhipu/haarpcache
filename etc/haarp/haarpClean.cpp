/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 * @autor Oscar Vaquero Montecino <osvamon@gmail.com>
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
#include <cstring>
#include <string>
#include <vector>
#include <sys/types.h>
#include <fcntl.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <utime.h>
#include <sys/statvfs.h>
//#include <blkid/blkid.h>
#include <fstab.h>
#include <cstdio>
#include <dirent.h>
#include <list>
#include <ctime>

//#include "database_mysql.cpp"
#define charmalloc(a) (char *)malloc(a*sizeof(char))  
#define NUM_FILES 20000
#define MAX_HIT 100
#define pb push_back
#define MYSQL_PING_TIME 50
#define p(a) cout<<a<<endl

using namespace std;

string domain;
string file;
struct tm * date_min = NULL;
struct tm * date_max = NULL;
vector<string> list_dir;
double total_delete_mb;
int numdelete = 0;
time_t m_tNextMysqlPingTime;

void SearchReplace2(string &source, string search, string replace) {
    string::size_type position = source.find(search);
    while (position != string::npos) {
        source.replace(position, search.size(), replace);
        position = source.find(search,position+replace.length());
    }
}
//~ class ldir{
	//~ public:
		//~ string dir;
		//~ double mb_to_delete;
//~ };
string sqlconv2(string sql) {
	SearchReplace2(sql,"'","\\'");
	SearchReplace2(sql,"$","\\$");
	SearchReplace2(sql,";","\\;");
	SearchReplace2(sql,"\"","\\\"");
	return sql;
}
string getValstr(const char *p) {
	string f = string(p);
	if(f[0] == '\'')
		f.erase(0,1);
	if(f[f.size() - 1] == '\'')
		f.erase(f.size() - 1,1);
	return f;
}
bool isnumber(char *p) {
	int l = strlen(p);
	int i;
	for( i = 0 ; i < l ; i++ ) 
		if( !isdigit(p[i]) )
			return false;
	return true;
}
void putzero(struct tm * t) {
	t->tm_sec = 0;
	t->tm_min = 0;
	t->tm_hour = 0;
	t->tm_isdst = 0;
}
int string2int( string s ) { 
	return atoi(s.c_str());
}
//~ Convert data of format: 'yyyy-mm-dd' to the structure tm.
struct tm * str2date(const char *pt) {
	struct tm *t1 = (struct tm *)malloc(sizeof(struct tm));
	vector<string> resul;
	stringexplode(string(pt),"-",&resul);
	t1->tm_mday = string2int(resul.at(2));
	t1->tm_mon = string2int(resul.at(1)) - 1;
	t1->tm_year = string2int(resul.at(0)) - 1900;
	putzero(t1);
	return t1;
}
//~ Convert data of format: 'dd/mm/yyyy' to the structure tm.
struct tm * str2date2(const 	char *pt) {
	struct tm *t1 = (struct tm *)malloc(sizeof(struct tm));
	vector<string> resul;
	stringexplode(string(pt),"/",&resul);
	t1->tm_mday = string2int(resul.at(0));
	t1->tm_mon = string2int(resul.at(1)) - 1;
	t1->tm_year = string2int(resul.at(2)) - 1900;
	putzero(t1);
	return t1;
}

string date2str(struct tm * t) {
	char p[30];
	//~ printf("date2str: %i\t%i\t%i\n", t->tm_mday, t->tm_mon, t->tm_year);
	strftime(p,30,"%Y-%m-%d",t);
	return string(p);
}

struct tm * getNewDateMax(int fase, int hit) {
	time_t d_min = mktime(date_min);
	time_t d_max = mktime(date_max);
	double kn = (4*pow(2,fase) - 1)/(4*pow(2,fase));
	double prop = 1;
	for(int i = 0; i <= hit; i++) {
		prop = prop*kn;
	}
	time_t d_end = d_min + floor((d_max - d_min)*prop);
	return localtime(&d_end);
}

MYSQL_RES * mysql_select_files(MYSQL* conn, struct tm *date_mx, int hit, int flag) {
	char q[400];
	string d = date2str(date_mx);
	string ds = date2str(date_min);
	if(!flag) {
		if( domain != "" ) {
			sprintf(q, "SELECT file, domain, filesize, downloaded FROM haarp WHERE domain = '%s' AND date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested <= %i*filesize limit %i", domain.c_str(), ds.c_str(), d.c_str(), hit, NUM_FILES);
		}
		else {
			sprintf(q, "SELECT file, domain, filesize, downloaded FROM haarp WHERE date(downloaded) >= '%s' AND  date(downloaded) <= '%s' AND bytes_requested <= %i*filesize  limit %i", ds.c_str(), d.c_str(), hit, NUM_FILES);
		}
	}
	else {
		if( domain != "" ) {
			sprintf(q, "SELECT file, domain, filesize, downloaded FROM haarp WHERE domain = '%s' AND date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested > %i*filesize limit %i", domain.c_str(), ds.c_str(), d.c_str(), hit, NUM_FILES);
		}
		else {
			sprintf(q, "SELECT file, domain, filesize, downloaded FROM haarp WHERE date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested > %i*filesize limit %i", ds.c_str(), d.c_str(), hit, NUM_FILES);
		}
	}
	if(mysql_query(conn, q)) {
		cout<<"MYSQL Error: Query: '"<<q<<"'; "<<mysql_error(conn)<<endl;
		exit(1);
	}
	//p(q);
	//p("Select files from the DB ...");
	return mysql_store_result(conn);
}

void mysql_delete_files(MYSQL * conn, struct tm * date_mx, int hit, int flag) {
	p("Delete files from DB .... ");
	char q[500];
	string d = date2str(date_mx);
	string ds = date2str(date_min);	
	if( !flag ) {
		if( domain == "" )
			sprintf(q,"DELETE from haarp WHERE date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested = %i*filesize limit %i", ds.c_str(), d.c_str(), hit, NUM_FILES);
		else 
			sprintf(q,"DELETE from haarp WHERE domain = '%s' AND date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested = %i*filesize limit %i", (sqlconv2(domain)).c_str(), ds.c_str(), d.c_str(), hit, NUM_FILES);
	} else {
		if( domain == "" )  
			sprintf(q,"DELETE from haarp WHERE date(downloaded) >= '%s' AND date(downloaded) <= '%s' AND bytes_requested >= %i*filesize limit %i", ds.c_str(), d.c_str(), hit, NUM_FILES);
		else 
			sprintf(q,"DELETE from haarp WHERE domain = '%s' AND date(downloaded) >= '%s' AND date(downloaded) <='%s' AND bytes_requested >= %i*filesize limit %i", (sqlconv2(domain)).c_str(), ds.c_str(), d.c_str(), hit, NUM_FILES);
	}
	//p(q);
	if( mysql_query(conn, q) ) {
		cout<<"MYSQL Error "<<mysql_errno(conn)<<": Query: '"<<q<<"' --> "<<mysql_error(conn)<<endl;
		exit(1);
	}
}
//~ Return 1 continue with the elimination.
//~ Return 0 end of the elimination.
int delete_by_block(MYSQL *connect, int fase, int hit, double *mb, int flag) {
	struct tm * date_new_max = getNewDateMax(fase, hit);
	MYSQL_RES *res;
	MYSQL_ROW r;
	vector<string>::iterator dir_index;
	string subdir;
	char f[600];
	char q[600];
	
	while(true) {
		//~ mysql_update_deleted(connect, date_new_max, hit, flag);
		res = mysql_select_files(connect, date_new_max, hit, flag);
		if(!mysql_num_rows(res)) {
			//printf("Salida por termino de archivos eliminados con hit '%i'\n",hit);
			if( flag && mktime(date_max) == mktime(date_new_max) )
				return 0;
			return 1;
		}
		bool file_exis = false;
		//~ select files and delete from the disk.
		while ( (r = mysql_fetch_row(res)) != NULL ) {
			subdir = ConvertChar(r[0]);
			file_exis = false;
			for(dir_index = list_dir.begin();dir_index != list_dir.end();dir_index++) {
				sprintf(f,"%s%s/%s/%s",(*dir_index).c_str(),r[1],subdir.c_str(),r[0]);
				//~ p(f);
				if(file_exists(string(f))) {
					file_exis = true;
					sprintf(q,"rm -f \"%s\"",f);
					numdelete++;
					if ( !(numdelete % 100) ) {
						if(atof(r[2]) > 1048576)
							printf("Entry %i, Delete %.1lf MB (total:%.2lf GB), Downloaded : %s [Hits<= %i], File: '%s'\n", numdelete, atof(r[2])/(1048576.0), *mb/1024, r[3], hit, f);
						else
							printf("Entry %i, Delete %.1lf KB (total:%.2lf GB), Downloaded : %s [Hits<= %i], File: '%s'\n", numdelete, atof(r[2])/(1024.0), *mb/1024, r[3], hit, f);
					}
					system(q);					
					*mb = *mb + atof(r[2])/(1048576.0);
				}
			}
			if ( !file_exis )
				p("[WARNING] The file: '"+ string(r[0]) + "' not EXIST in the disks!");
			if ( time(NULL) >= m_tNextMysqlPingTime ) {
				mysql_ping(connect);
				m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
			}
		}
		mysql_delete_files(connect, date_new_max, hit, flag);
		mysql_free_result(res);
		if( *mb > total_delete_mb ) {
			//p("SALIDA POR EL PASO DE LOS MB'S LIMITE.");
			return 0;
		}
	}
	return 1;
}

struct tm * getDateMin(MYSQL * connect) {
	struct tm * ttmp = NULL;
	char q[400];
	if(domain != "") 
		sprintf(q,"select min(date(downloaded)) from haarp where domain='%s' HAVING min(downloaded) is not null", domain.c_str());
	else
		sprintf(q,"select min(date(downloaded)) from haarp HAVING min(downloaded) is not null");
		
	if(mysql_query(connect, q)) {
		cout<<"MYSQL Error: Query: '"<<q<<"'; "<<mysql_error(connect)<<endl;
		exit(1);
	}
	MYSQL_RES *res = mysql_store_result(connect);
	MYSQL_ROW row;
	if( !mysql_num_rows(res) ) {
		printf("Files not found in the data base.\n");
		exit(1);
	}
	while ( (row = mysql_fetch_row(res)) != NULL ) {
		ttmp = str2date(row[0]);
		break;
	}
	mysql_free_result(res);
	return ttmp;
}
struct tm * getDateMax(MYSQL * connect) {
	struct tm * ttmp = NULL;
	char q[400];
	if(domain != "") 
		sprintf(q,"select max(date(downloaded)) from haarp where domain='%s' HAVING max(downloaded) is not null",domain.c_str());
	else
		sprintf(q,"select max(date(downloaded)) from haarp HAVING max(downloaded) is not null");
		
	if(mysql_query(connect, q)) {
		cout<<"MYSQL Error: Query: '"<<q<<"'; "<<mysql_error(connect)<<endl;
		exit(1);
	}
	MYSQL_RES *res = mysql_store_result(connect);
	MYSQL_ROW row;
	if( !mysql_num_rows(res) ) {
		printf("Files not found in the data base.\n");
		exit(1);
	}
	while ( (row = mysql_fetch_row(res)) != NULL ) {
		ttmp = str2date(row[0]);
		break;
	}
	mysql_free_result(res);
	return ttmp;	
}
void deleteEmptyRecords(MYSQL *connect) {
	if(mysql_query(connect, "SELECT count(*) FROM haarp WHERE rg='' and filesize=0 and date(modified) < date(now() - interval 1 day)")) {
		cout<<"MYSQL Error, "<<mysql_error(connect)<<endl;
		exit(1);
	}
	MYSQL_ROW r = mysql_fetch_row(mysql_store_result(connect));
	
	if(r == NULL || !strcmp(r[0], "0"))
		return;
	
	cout<<"Deleting "<<r[0]<<" empty record(s) in the database ...\n";
	if(mysql_query(connect, "DELETE FROM haarp WHERE rg='' and filesize=0 and date(modified) < date(now() - interval 1 day)")) {
		cout<<"MYSQL Error, "<<mysql_error(connect)<<endl;
		exit(1);
	}
}

void phelp(int flag) {
	if(flag)
		puts(" haarpClean 1.1 by Manolo Canales (keikurono01) <kei.haarpcache@gmail.com>");
	puts("\n\tUsage: \n");
	puts("\t-h, --help\t\tPrint help message.\n");
	puts("\t-f, --file <file>\tFile that we are looking on the data base for are remove, the \n\t\t\t\tfile can are a expression incluing characters '%', \n\t\t\t\texamples: ./haarpClear --file '1d233f1d3g1aF41-123%';\n");
	puts("\t-d, --domain <domain>\tSpecified the domain of the files to delete;\n");
	puts("\t--dates <(dd/mm/yyyy|begin)-(dd/mm/yyyy|end)>\tIs the range of dates that will are delete from the db, \n\t\t\t\t\t\t\texample: ./haarpClear --dates 12/10/2013-10/12/2013 or\n\t\t\t\t\t\t\t./haarpClean --dates begin-10/05/2011, for delete the \n\t\t\t\t\t\t\tfiles from a date initial to date 10/05/2011.\n");
}

int main(int carg, char **varg) {
	m_tNextMysqlPingTime = time(NULL);
	string date_tmp, cachedir,cachelimit,directory,completefilepath, subdir,host,user,pass,db;
	
	domain = file = date_tmp = "";
	total_delete_mb = 0;
	
	bool limit_surpassed = false;
	bool date_begin = false;
	bool date_end = false;
	
	double expected_deletion = 0;
	
	double disk_space;
	double use_percent;
	
	m_tNextMysqlPingTime = time(NULL) + MYSQL_PING_TIME;
	
	Params::SetParams(0,varg);
	cachedir = Params::GetConfigString("CACHEDIR");
	cachelimit = Params::GetConfigString("CACHE_LIMIT");
	host = Params::GetConfigString("MYSQL_HOST");
	user = Params::GetConfigString("MYSQL_USER");
	pass = Params::GetConfigString("MYSQL_PASS");
	db = Params::GetConfigString("MYSQL_DB");
	
	if(carg > 1) { 
		char opt[100];
		for(int i = 1; i < carg; i++) {
			strcpy(opt, varg[i]);
			if( !strcmp(opt, "--help") || !strcmp(opt, "-h") ) {
				phelp(1);
				return EXIT_SUCCESS;
			}
			else if( !strcmp(opt, "--file") || !strcmp(opt, "-f") ) {
				file = getValstr(varg[++i]);
			}
			else if( !strcmp(opt, "--domain") || !strcmp(opt, "-d") ) {
				domain = getValstr(varg[++i]);
			}
			else if( !strcmp(opt, "--dates") ) {
				date_tmp = getValstr(varg[++i]);
				//~ p(date_tmp);
				char *date = (char *)malloc(sizeof(char)*(date_tmp.size()+4));
				strcpy(date,date_tmp.c_str());
				char *p = strstr(date,"-");
				if( p == NULL ) { puts("Error, format to dates if incorrect."); return EXIT_FAILURE; }
				*p = '\0';
				int i, j, j0;
				j0 = 0;
				if( !strcmp(date,"begin") ) {
					date_begin = true;
					j0 = 1;
					date = p + 1;
				}
				*p = '/';
				strcat(date,"/");
				for(j = j0; j <= 1; j++) {
					if( j == 1 && !strcmp(date,"end/") ) {
						date_end = true;
						break;
					}
					for(i = 0; i <= 2; i++) {
						char *r = strstr(date, "/");
						if( r == NULL ) { puts("Error, format to dates if incorrect."); return EXIT_FAILURE; }
						*r = '\0';
						if ( !isnumber(date) || (atoi(date) > (i==0)*31 + (i==1)*12 + (i==2)*4000) || atoi(date) < 0 ) { puts("Error, format to dates if incorrect."); return EXIT_FAILURE; }
						date = ++r;
					}
				}
			} else {
				char *r = opt;
				while( r[0] == '-' ) { ++r; }
				printf("\t%s: unknown argument '%s'\n", varg[0], r);
				phelp(0);
				return EXIT_FAILURE;
			}
		}
	}
	p(date_tmp);
	// BD connection
	MYSQL *connect;
	MYSQL_RES *res;
	MYSQL_ROW r;
	connect = mysql_init(NULL);
	
	const unsigned int connect_timeout = 2400;
	const unsigned int wtimeout = 2400;
	const unsigned int rtimeout = 2400;
	
	mysql_options(connect, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
	mysql_options(connect, MYSQL_OPT_READ_TIMEOUT, &rtimeout);
	mysql_options(connect, MYSQL_OPT_WRITE_TIMEOUT, &wtimeout);
	
	if(!mysql_real_connect(connect,host.c_str(),user.c_str(),pass.c_str(),db.c_str(),MYSQL_PORT,NULL,0)) {
			cout<<"MYSQL Error: "<<mysql_error(connect)<<endl;
			return 1;
	}
	//~ 
	stringexplodetrim(cachedir,"|",&list_dir);
	//~ 
	if( file != "" ) {
		char q[300];
		sprintf(q, "select file, domain, filesize from haarp where file like '%s'", (sqlconv2(file)).c_str() );
		if( mysql_query(connect, q) ) {
			cout<<"MYSQL Error: Query: '"<<q<<"'; "<<mysql_error(connect)<<endl;
			return EXIT_FAILURE;
		}
		res = mysql_store_result(connect);
		string subdir;
		int num = 1;
		double mb = 0;
		char f[600];
		vector<string>::iterator dir_index;		
		while ( (r = mysql_fetch_row(res)) != NULL ) {
			subdir = ConvertChar(r[0]);
			bool file_exis = false;
			for(dir_index = list_dir.begin(); dir_index != list_dir.end(); dir_index++) {
				sprintf(f, "%s%s/%s/%s", (*dir_index).c_str(), r[1], subdir.c_str(), r[0]);
				if( file_exists(string(f)) ) {
					
					sprintf(q,"rm -f %s",f);
					
					if(atof(r[2]) > 1048576)
						printf("(%i) Deleting %s [%.1lf MB]\n", num, f, atof(r[2])/(1048576.0));
					else
						printf("(%i) Deleting %s [%.1lf KB]\n", num, f, atof(r[2])/(1024.0));
						
					system(q); //~ delete file in disk.
					
					mb = mb + atof(r[2])/(1048576.0);
					
					num++;
					
					file_exis = true;
				}
			}
			if(!file_exis) {
				p("The file: '"+ string(r[0]) + "' not exist in the disks, delete only in the DB");
			}
		}
		sprintf(q, "delete from haarp where file like '%s'", (sqlconv2(file)).c_str());
		if( mysql_query(connect, q) ) {
			cout<<"MYSQL Error: Query: '"<<q<<"'; "<<mysql_error(connect)<<endl;
			return EXIT_FAILURE;
		}
		mysql_free_result(res);
		
		if(mb > 1024)
			printf("Total deleting: %.1lf GB.\n", mb/1024.0);
		else
			printf("Total deleting: %.1lf MB.\n", mb);
		
		return EXIT_SUCCESS;
	}
	
	if( date_tmp == "" ) {
		limit_surpassed = true;
		for(int i=0;i < (int)list_dir.size();i++) {
			cachedir = list_dir.at(i); 
			disk_space = disk_size(cachedir);
			use_percent = disk_use(cachedir);

			printf("Disk size : %.2lf MB, %.2lf GB\n",disk_space/(1048576),disk_space/(1073741824));
			cout<<"Disk occupation percentage on partition hosting cache directory "<<cachedir<<" : "<<use_percent<<"%"<<endl;
			if( use_percent >= atof(cachelimit.c_str()) ) {
				expected_deletion = ( use_percent - atof( cachelimit.c_str() ) + 0.5 )*disk_space/100;
				total_delete_mb += expected_deletion/1048576;
				printf("In the disk: %s, space occupied over cache limit : %.2lf MB (%.2lf GB) %.2lf %% above limit in /etc/haarp/haarp.conf (%s %%)\n", cachedir.c_str(), expected_deletion/1048576, expected_deletion/1073741824, use_percent - atof(cachelimit.c_str()), cachelimit.c_str());
			}
			else {
				limit_surpassed = false;
				continue;
			}
		}
		if ( !limit_surpassed ) {	
			cout<<endl<<"Ocupation of cache directories within configuration limits; exiting..."<<endl;
			deleteEmptyRecords(connect);
			return 1;
		}
	} else 
		total_delete_mb = 1000000000;
		
	//~ Dates
	date_min = getDateMin(connect);
	date_max = getDateMax(connect);

	if( date_tmp != "" ) {
		char tmp[100];
		strcpy(tmp, date_tmp.c_str());
		char *p = strstr(tmp,"-");
		*p = '\0';
		if(!date_begin) 
			date_min = str2date2(tmp);
			
		if(!date_end)
			date_max = str2date2(++p);
	}
	//~ 
	double mb_eliminate = 0;
	
	int fase = -1;
	bool a = false;

	while( mb_eliminate < total_delete_mb ) {
		fase++;
		int hit = 0;
		int re = 1;
		for(hit = 0;hit < MAX_HIT;) {
			re = delete_by_block(connect, fase, hit, &mb_eliminate, 0);
			if(!re) {
				a = true;
				break;
			}
			if( hit > 15 ) 
				hit += 10;
			else 
				hit++;
		}
		if(a)
			break;
		re = delete_by_block(connect, fase, hit, &mb_eliminate, 1);
		if(!re)
			break;
	}
	
	cout<<"Entries deleted: "<<numdelete<<endl;

	if (mb_eliminate > 1024)
		printf("Total Eliminate: %.2lf GB\n", mb_eliminate/1024);
	else
		printf("Total Eliminate: %.2lf MB\n", mb_eliminate);
	
	mysql_close(connect);
	
	return 1;
}
