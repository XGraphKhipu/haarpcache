#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <list>
#include <iostream>

//~ typedef long long int LLI;

using namespace std;

#define DELIM "_-DELHAARP-_"
#define LIMIT_USER_CACHE_DB 10

struct resposta {
	bool match;
	string domain;
	string file;
	long long int range_min;
	long long int range_max;
	long long int total_file_size;
	bool exist_range;	
};

class usercache {
	public:
		string ip;
		time_t date_downloaded;
		time_t date_modified;
		long long int bytes_acumulate;
		long long int bytes_requested;
};

class intervalPositionByteDisk {
	public:
		long long int a;
		long long int b;
		long long int position;
};	

typedef list<usercache> lusercache;
typedef list<intervalPositionByteDisk>  lintervalPositionByteDisk;

void addUserCache(lusercache &luc, string ip, time_t date_modified, long long int bytes, bool hit);
bool str2lusercache(string str, lusercache &lre);
string lusercache2str(lusercache luc);
bool compareUserCache(usercache &l1, usercache &l2);
long long int getFileSize(lintervalPositionByteDisk listIntervalPositionByteDisk);
bool is_all_hit(lintervalPositionByteDisk &listIntervalPositionByteDisk, lintervalPositionByteDisk::iterator it);
long long int getExtremeb(lintervalPositionByteDisk listIntervalPositionByteDisk);
lintervalPositionByteDisk::iterator getlastnode(lintervalPositionByteDisk &listIntervalPositionByteDisk);
int generateList(string ranges, string parts, lintervalPositionByteDisk &listIntervalPositionByteDisk);
long long int getPointEnd(lintervalPositionByteDisk listIntervalPositionByteDisk);
lintervalPositionByteDisk getRangeWork(lintervalPositionByteDisk &listIntervalPositionByteDisk, long long int interval_a, long long int interval_b, bool *hit);
bool appendNode(lintervalPositionByteDisk &listIntervalPositionByteDisk, intervalPositionByteDisk newInterval);
bool appendSubNode(lintervalPositionByteDisk &listIntervalPositionByteDisk, intervalPositionByteDisk n, long long int lenght_);
//~ void ordenar(lintervalPositionByteDisk listIntervalPositionByteDisk);
bool compareIntervals(intervalPositionByteDisk &a, intervalPositionByteDisk &b);
void list2string(lintervalPositionByteDisk listIntervalPositionByteDisk,string &s1, string &s2);

string trimstr(string str);

static const std::string base64_chars = 
			"nopq?rst@#u789+RST&UyzMNOPQhi"
             "abcdefgvwx$FGHIJKL"
             "012!34jklmA*BCDEVWXYZ56/";

string UpperCase( string CaseString );
int SearchReplace( string &source, string search, string replace );
int SearchReplaceAny( string &source, string search, string replace );
int select_eintr( int fds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout );
bool MatchSubstr(string &hay, const char* needle, int startpos);
bool MatchBegin(string &hay, const char *needle, int needlelength);
void stringexplode(string str, string separator, vector<string>* results);
void stringexplodetrim(string str, string separator, vector<string>* results);
void splitstring(string str, string separator, vector<string>* results);
string getdomain(string url);
bool file_exists(string strFilename);
int64_t file_size( string szFileName );
void mkdir_p(const string &pathname);
string getfilepath(string path);
string getfilename(string path);
string regex_match(string er, string line);
string regex_match_nocase(string er, string line);
string itoa(int val);
string llitoa(long long int val);
string lldtoa(long long int val);
double now();
long file_getmodif( string szFileName );
int file_setmodif( string szFileName,long fdate =0); 
string url2host(string &url);
string url2request(string &url);
string ConvertChar(string lineT);
double disk_use(string path);
double disk_size(string path);
double disk_occupation(string path);
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len);
std::string XOR(string value,string key);
const string getFileExtension(string file);
const string getFileName(string file);
const string sqlconv(string sql);
void longSeekpFile(fstream &f, long long int pos);
#endif
