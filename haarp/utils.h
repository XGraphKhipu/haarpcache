#ifndef UTILS_H
#define UTILS_H

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <vector>

using namespace std;

struct resposta {
	bool match;
	string domain;
	string file;
	int range_min;
	int range_max;
};
typedef struct lista {
	int a;
	int b;
	int p;
	struct lista * next;
} llista;

int getExtremeb(llista *primer);
int generateList(string ranges, string parts, llista **primer);
int getPointEnd(llista *primer);
llista *getRangeWork(llista **primer,int ra, int rb, bool *hit);
llista *getlastnode(llista *primer);
bool appendNode(llista **primer, llista *n);
bool appendSubNode(llista **primer, llista *n, int lenght_);
void ordenar(llista **primer);
void list2string(llista *primer,string &s1, string &s2);
void list_clear(llista **primer);
long int getFileSize(llista *primer);
bool is_all_hit(llista *p);
string trimstr(string str);

static const std::string base64_chars = 
			"nopq?rst@#u789+RST&UyzMNOPQhi"
             "abcdefgvwx$FGHIJKL"
             "012!34jklmA*BCDEVWXYZ56/";

string UpperCase( string CaseString );
void SearchReplace( string &source, string search, string replace );
int select_eintr( int fds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout );
bool MatchSubstr(string &hay, const char* needle, int startpos);
bool MatchBegin(string &hay, const char *needle, int needlelength);
void stringexplode(string str, string separator, vector<string>* results);
string getdomain(string url);
bool file_exists(string strFilename);
int64_t file_size( string szFileName );
void mkdir_p(const string &pathname);
string getfilepath(string path);
string getfilename(string path);
string regex_match(string er, string line);
string itoa(int val);
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

#endif
