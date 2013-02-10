#include "utils.h"
#include "params.h"

#include "curl/curl.h"

#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <cstring>
#include <string>
#include <vector>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <iostream>
#include <sstream>
#include <math.h>
#include <cstdlib>
#include <utime.h>
#include <sys/statvfs.h>
#include <blkid/blkid.h>
#include <fstab.h>
#include <cstdio>


bool remove_param(string &curl, string param) {
	int pos, next;
	
	if( (pos = (int)curl.find(param)) == (int)string::npos )
		return false;
	
	if( ( next = (int)curl.substr(pos).find("&") ) != (int)string::npos ) {
		next++;
		curl.erase(pos, next);
	}
	else
		curl.erase(--pos);
	return true;
}

bool is_all_hit(llista *p) {
	llista *n = p;
	while(n) {
		if( n-> p < 0 )
			return false;
		n = n->next;
	}
	return true;
}
long int getFileSize(llista *primer) {
	llista *n = primer;
	long int re = 0;
	while(n) {
		re += n->b - n->a + 1; 
		n = n->next;
	}
	return re;
}

int getExtremeb(llista *primer){
	llista *n = primer;
	int max = -1;
	while(n){
		if( n->b > max )
			max = n->b;
		n = n->next;
	}
	return max;
}

llista *getlastnode(llista *primer){
	llista *n, *m;
	int max; 
	
	n = primer;
	m = n;
	max = n->p;	
	while(n){
		if(n->p >  max)
		{
			max = n->p;
			m = n;
		}
		n = n->next;
	}
	return m;
}
bool appendNode(llista **primer, llista *n) {
	llista *last;
	
	if(!n)
		return false;
	if(!*primer){
		llista *nn = (llista *)malloc(sizeof(llista));
		nn->a = n->a;
		nn->b = n->b;
		nn->p = n->p;
		nn->next = NULL;
		
		*primer = nn;
		return true;
	}
	last = getlastnode(*primer);
	if( last->b + 1 == n->a ){
		last->b = n->b;
		return true;
	}
	llista *nn = (llista *)malloc(sizeof(llista));
	nn->a = n->a;
	nn->b = n->b;
	nn->p = n->p;
	
	nn->next = last->next;	
	last->next = nn;	
	return true;	
}
bool appendSubNode(llista **primer, llista *n, int lenght_) {
	llista *last;
	
	if(!n)
		return false;
	if(!*primer) {
		llista *nn = (llista *)malloc(sizeof(llista));
		nn->a = n->a;
		nn->b = lenght_ + n->a - 1;
		nn->p = n->p;
		nn->next = NULL;
		
		*primer = nn;
		return true;
	}
	last = getlastnode(*primer);
	if( last->b + 1 == n->a ) {
		last->b = lenght_ + n->a - 1;
		return true;
	}
	llista *nn = (llista *)malloc(sizeof(llista));
	nn->a = n->a;
	nn->b = lenght_ + n->a - 1;
	nn->p = n->p;
	
	nn->next = last->next;	
	last->next = nn;	
	return true;	
}
/*
 * Retorna el último lugar entero de bytes donde se pueden escribir más datos .
 */
int getPointEnd(llista *primer) {
	llista *n = primer;
	if(!n)
		return 0;
	int max = n->p;
	int interval = n->b - n->a;
	while(n) {
		if(n->p > max) {
			max = n->p;
			interval = n->b - n->a;
		}
		n = n->next;
	}
	return max + interval + 1;
}
/*
 * Consigue una lista de nodos del tipo llista
 * que contiene el orden de los nodos a trabajar (cada nodo es un bloque de datos de un archivo)
 * getRangeWork retorna el valor hit, y la lista de bloques a trabajar.
 * 
 */
llista *getRangeWork(llista **primer, int ra, int rb, bool *hit) {
	ordenar(primer);
	
	llista *pr = NULL;
	llista *ult = NULL;
	
	llista *n = *primer;
	*hit = 1;
	int tope = -1;
	while(n) {
		if( ra < n->a )
		{
			tope = ra;
			break;
		}
		else
		{
			if( ra >= n->a && ra <= n->b )
			{
				llista * nn = (llista *)malloc(sizeof(llista));
				if( rb <= n->b )
				{
					nn->a = ra;
					nn->b = rb;
					nn->p = n->p + ra - n->a;
					nn->next = NULL;
					*hit = 1;
					return nn;
				}
				nn->a = ra;
				nn->b = n->b;
				nn->p = n->p + ra - n->a;
				nn->next = NULL;
				pr = nn;
				ult = nn;
				tope = n->b + 1;
				n = n->next;
				break;
			}
		}
		n = n->next;
	}
	while( tope >= 0 && n )
	{
		if( rb < n->a )
		{
			llista * nn = (llista *)malloc(sizeof(llista));
			nn->a = tope;
			nn->b = rb;
			nn->p = -1;
			nn->next = NULL;
			if(ult)
				ult->next = nn;			
			else
				pr = nn;
			*hit = 0;
			tope = rb + 1;
			break;
		}
		else{
			if( tope < n->a ){
				llista * nn = (llista *)malloc(sizeof(llista));
				nn->a = tope;
				nn->b = n->a - 1;
				nn->p = -1;
				nn->next = NULL;
				if(ult)
					ult->next = nn;
				else			
					pr = nn;
				ult = nn;
				tope = n->a;
				*hit = 0;
			}
			if( rb >= n->a && rb <= n->b)
			{
				llista * nn = (llista *)malloc(sizeof(llista));
				nn->a = tope;
				nn->b = rb;
				nn->p = n->p;
				nn->next = NULL;
				ult->next = nn;				
				tope = n->b + 1;
				break;
			}
			else
			{
				llista * nn = (llista *)malloc(sizeof(llista));
				nn->a = n->a;
				nn->b = n->b;
				nn->p = n->p;
				nn->next = NULL;
				ult->next = nn;
				ult = nn;
			}
		}
		tope = n->b + 1;
		n = n->next;	
	}

	if(tope == -1)
	{
		llista * nn = (llista *)malloc(sizeof(llista));
		nn->a = ra;
		nn->b = rb;
		nn->p = -1;
		*hit = 0;
		nn->next = NULL;
		return nn;
	}
	if( rb > tope )
	{
		llista * nn = (llista *)malloc(sizeof(llista));
		nn->a = tope;
		nn->b = rb;
		nn->p = -1;
		nn->next = NULL;
		ult->next = nn;
		*hit = 0;
	}
	return pr;	
}
string trimstr(string str) {
	size_t pos = str.find_first_not_of(" \t");
	if(pos==string::npos)
		return "";
	return str.substr(pos,str.find_last_not_of(" \t") - pos + 1);
}
//~ Convert "123-200,11-22,.." + "0,123,444..." hacia lista
int generateList(string ranges, string parts, llista **primer){
	*primer = NULL;
	vector<string> resul,rang,part;
	stringexplode(ranges,",",&resul);
	stringexplode(parts,",",&part);
	if( part.size() != resul.size() )
	{
		return 0;
	}
	int i;
	for(i=0;i<(int)resul.size();i++)
	{
		lista * n = (lista *)malloc(sizeof(lista));
		stringexplode(resul.at(i),"-",&rang);
		if(rang.size() < 2)
		{
			return 0;
		}
		n->a = atoi(rang.at(0).c_str());
		n->b = atoi(rang.at(1).c_str());
		if(n->a > n->b)
		{
			return 0;
		}
		n->p = atoi(part.at(i).c_str());
		n->next = NULL;
		if(*primer)
			n->next = *primer;
		*primer = n;
		rang.clear();
	}
	return 1;
}

void list_clear(llista **primer){
	llista *n = *primer;
	while(n)
	{
		llista *tmp = n;
		n = n->next;
		free(tmp);
	}
	*primer = NULL;
}
/*
 * Ordena la lista primer por el valor a
 * */
void ordenar(llista **primer){
	llista *es;
	llista *pas = *primer;
	llista *ini = *primer;
	llista *prev;
	llista *prevtmp;
	llista *previni = NULL;
	int min;
	while(ini)
	{
		es = NULL;		
		pas = ini;
		min = pas->a;
		while(pas)
		{
			if(pas->a < min)
			{
				min = pas->a;
				es = pas;
				prev = prevtmp;
			}
			prevtmp = pas;
			pas = pas->next;			
		}
		if(es)
		{
			prev->next = es->next;
			es->next = ini;
			if(!previni)
				*primer = es;
			else
				previni->next = es;
			ini = es;
		}
		previni = ini;
		ini = ini->next;
	}
}

void list2string(llista *primer,string &s1, string &s2){
	stringstream out,out1;
	llista *n = primer;
	string coma = "";
	while(n){
		out<<coma<<n->a<<"-"<<n->b;
		out1<<coma<<n->p;
		coma = ",";
		n = n->next;
	}
	s1 = out.str();
	s2 = out1.str();
}

string UpperCase(string CaseString) {
    string::const_iterator si = CaseString.begin();
    string::size_type j = 0;
    string::size_type e = CaseString.size();
    while (j < e) {
        CaseString[j++] = toupper(*si++);
    }
    return CaseString;
}

void SearchReplace(string &source, string search, string replace) {
    string::size_type position = source.find(search);

    while (position != string::npos) {
        source.replace(position, search.size(), replace);
        position = source.find(search);
    }
}

int select_eintr(int fds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout) {
    if (timeout->tv_sec == 0) {
        return select(fds, readfds, writefds, errorfds, timeout);
    }

    int ret;

#ifndef __linux__
    time_t start = time(NULL);
    time_t now;
    int orig_timeout = timeout->tv_sec;
#endif

    while ((ret = select(fds, readfds, writefds, errorfds, timeout)) < 0 && errno == EINTR) {
#ifndef __linux__
        now = time(NULL);
        if ((now - start) < orig_timeout) {
            timeout->tv_sec = orig_timeout - (now - start);
            timeout->tv_usec = 0;
        }
#endif
    }

    return ret;
}

bool MatchBegin(string &hay, const char *needle, int needlelength) {
    return ( strncmp(hay.c_str(), needle, needlelength) == 0) ? true : false;
}

bool MatchSubstr(string &hay, const char *needle, int startpos) {
    if (startpos == -1) {
        return ( strstr(hay.c_str(), needle) != NULL) ? true : false;
    } else {
        return ( strstr(hay.c_str(), needle) == hay.c_str() + startpos) ? true : false;
    }
}

void stringexplode(string str, string separator, vector<string>* results) {
    size_t found;
    found = str.find_first_of(separator);
    while (found != string::npos) {
        if (found > 0) {
            results->push_back(str.substr(0, found));
        }
        str = str.substr(found + 1);
        found = str.find_first_of(separator);
    }
    if (str.length() > 0) {
        results->push_back(str);
    }
}

string getdomain(string url) {
    if (regex_match("^74\\.125\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?$)", url) != "") return "youtube.com";
    if (regex_match("^(205\\.196\\.|199\\.91\\.)[0-9]{2,3}\\.[0-9]{1,3}", url) != "") return "mediafire.com";
    vector<string> resultado;
    if (!url.empty()) {
        stringexplode(url, "/", &resultado);
        if (resultado.size() > 1) {
            url = resultado.at(0);
        }
        resultado.clear();
        stringexplode(url, ".", &resultado);
        if (resultado.size() > 1) {
            if ((resultado.at(resultado.size() - 2)).size() <= 3 &&
                    (resultado.at(resultado.size() - 1)).size() <= 3) {
                return resultado.at(resultado.size() - 3) + "." + resultado.at(resultado.size() - 2) + "." + resultado.at(resultado.size() - 1);
            } else {
                return resultado.at(resultado.size() - 2) + "." + resultado.at(resultado.size() - 1);
            }
        } else return url;
    } else return "";

}

bool file_exists(string strFilename) {
    struct stat stFileInfo;
    if (stat(strFilename.c_str(), &stFileInfo) == 0) {
        return true;
    } else {
        return false;
    }
}

int64_t file_size(string szFileName) {
    struct stat fileStat;
    int err = stat(szFileName.c_str(), &fileStat);
    if (0 != err) return -1;
    return (int64_t) fileStat.st_size;
}

void mkdir_p(const string &pathname) {
    umask(0);
    if (mkdir(pathname.c_str(), 0777) < 0) {
        if (errno == ENOENT) {
            // If we couldn't make the whole directory because of ENOENT (a
            // parent directory doesn't exist), then try recursively to make
            // the immediate parent directory.

            size_t slash = pathname.rfind('/');
            if (slash != string::npos) {
                string prefix = pathname.substr(0, slash);
                mkdir_p(prefix);
                mkdir(pathname.c_str(), 0777);
            }
        }
    }
}

string getfilepath(string path) {
    vector<string> dirs;
    string resultado = "/";
    stringexplode(path, "/", &dirs);
    for (unsigned int i = 0; i <= dirs.size() - 2; i++) {
        resultado += dirs.at(i);
        resultado += "/";
    }
    return resultado;
}

string getfilename(string path) {
    vector<string> dirs;
    stringexplode(path, "/", &dirs);
    return dirs.at(dirs.size() - 1);
}

string regex_match(string er, string line) {
    int error;
    regmatch_t match;
    regex_t reg;
    if ((regcomp(&reg, er.c_str(), REG_EXTENDED | REG_NEWLINE)) == 0) {
        error = regexec(&reg, line.c_str(), 1, &match, 0);
        if (error == 0) {
            //cout << "Tamanho: " << line.size() << " Inicio: " << match.rm_so << " Fim: " << match.rm_eo << endl;
            return line.substr(match.rm_so, match.rm_eo - match.rm_so);
        } else {
            return "";
        }
    } else {
        return "";
    }
}

string itoa(int val) {
    char res[1024];
    sprintf(res, "%d", val);
    string str = string(res);
    return str;
}

double now() {
    struct timeval tv;
    gettimeofday(&tv, 0);

    return tv.tv_sec + tv.tv_usec / 1e6;
}

long file_getmodif(string szFileName) {
    struct stat fileStat;
    int err = stat(szFileName.c_str(), &fileStat);
    if (0 != err) return 0;
    return fileStat.st_mtime;
}

int file_setmodif(string szFileName, long fdate) {
    struct utimbuf fmtime;
    if (fdate > 0) {
        fmtime.actime = fdate + 10;
        fmtime.modtime = fdate + 10;
        return utime(szFileName.c_str(), &fmtime);
    } else {
        static struct timespec newtime;
        fmtime.actime = newtime.tv_sec;
        fmtime.modtime = newtime.tv_sec;
        return utime(szFileName.c_str(), &fmtime);
    }

}

string url2host(string &url) {
    int pos = 0;
    if (!url.empty()) {
        pos = url.find("/");
        if (pos > 0) {
            return url.substr(0, pos);
        } else {
            return url;
        }
    }
}

string url2request(string &url) {
    int pos = 0;
    if (!url.empty()) {
        pos = url.find("/");
        if (pos > 0) {
            return url.substr(pos, url.size());
        } else {
            return "/";
        }
    }
    return "";
}

string ConvertChar(string lineT) {
    string line = "";
    for (unsigned int i = 0; i < lineT.length(); i++) {
		int caracter = lineT[i];
		if( caracter < 123 && caracter > 96 )
			line += itoa(caracter - 96);
		else 
			if (isdigit(lineT[i]))
				line += lineT[i];
			else
				line += "27";
    }

    unsigned long resultado = 0;
    string tempstr = "";
    if (line.length() <= 3)
        line += "282930";

    for (unsigned int fff = 3; fff < line.length(); fff += 3) {
        tempstr = line.substr(0, fff);
        resultado += atoi(tempstr.c_str());
        line = line.substr(fff);
		fff = 0;
    }

    tempstr = itoa(resultado);
    if (tempstr.length() > 3)
        tempstr = tempstr.substr(tempstr.length() - 3);

    string updir = itoa(atoi(tempstr.substr(1, 1).c_str()) + atoi(tempstr.substr(2, 2).c_str()));

    if (updir.size() > 1)
        updir = updir.substr(1, 1);

    /* END! */
    lineT = "";
    lineT += updir;
    lineT += "/";
    lineT += tempstr[0];
    lineT += "_";
    lineT += tempstr.substr(1);
    return lineT;
}

int disk_use(string path) {
    double bfree, btotal = 0;
    struct statvfs fiData;
    if ((statvfs(path.c_str(), &fiData)) < 0) {
        return -1;
    } else {
        bfree = (((double) fiData.f_frsize) * ((double) fiData.f_bfree));
        btotal = (((double) fiData.f_frsize) * ((double) fiData.f_blocks));
        return ((btotal - bfree) / btotal)*100;
    }
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
    std::string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4); i++)
                ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            char_array_3[j] = '\0';

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = char_array_3[2] & 0x3f;

        for (j = 0; (j < i + 1); j++)
            ret += base64_chars[char_array_4[j]];

        //while((i++ < 3))
        //  ret += '=';

    }

    return ret;

}

std::string XOR(string value, string key) {
    string retval(value);

    short unsigned int klen = key.length();
    short unsigned int vlen = value.length();
    short unsigned int k = 0;
    short unsigned int v = 0;

    for (v; v < vlen; v++) {
        retval[v] = value[v]^key[k];
        k = (++k < klen ? k : 0);
    }

    return retval;
}

const string getFileExtension(string file) {
	size_t pos = file.find_last_of(".");
	if (pos != string::npos)
		return UpperCase(file.substr(pos + 1));
	else
		return UpperCase(file);
}

const string getFileName(string file) {
	size_t pos = file.find_last_of("/");
	if (pos != string::npos) {
		return file.substr(pos + 1);
	    //file = file.substr(pos + 1);
		//pos = file.find_last_of("?");
		//if (pos != string::npos)
		//	return file.substr(0,pos);
		//else
		//	return file;
	} else
		return file;
}
