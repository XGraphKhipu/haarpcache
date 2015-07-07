#include "utils.h"
#include "params.h"


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
#include <fstream>
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

bool is_all_hit(lintervalPositionByteDisk &listIntervalPositionByteDisk, lintervalPositionByteDisk::iterator it) {
	while(it != listIntervalPositionByteDisk.end()) {
		if(it->position < 0)
			return false;
		++it;
	}
	return true;
}

long long int getFileSize(lintervalPositionByteDisk listIntervalPositionByteDisk) {
	lintervalPositionByteDisk::iterator it = listIntervalPositionByteDisk.begin();
	long long int re = 0;
	while(it != listIntervalPositionByteDisk.end()) {
		re += it->b - it->a + 1;
		++it;
	}
	return re;
}

long long int getExtremeb(lintervalPositionByteDisk listIntervalPositionByteDisk) {
	long long int max = -1;
	lintervalPositionByteDisk::iterator it = listIntervalPositionByteDisk.begin();
	while(it != listIntervalPositionByteDisk.end()) {
		if( it->b > max )
			max = it->b;
		++it;
	}
	return max;
}

lintervalPositionByteDisk::iterator getlastnode(lintervalPositionByteDisk &lIntervPositionByteDisk) {
	lintervalPositionByteDisk::iterator itListOfRanges, itLastRangeBytes;	
	itListOfRanges = lIntervPositionByteDisk.begin();
	
	long long int max_position = itListOfRanges->position;
	itLastRangeBytes = itListOfRanges;	
	
	while( itListOfRanges != lIntervPositionByteDisk.end() ) {
		if(itListOfRanges->position >  max_position)
		{
			max_position = itListOfRanges->position;
			itLastRangeBytes = itListOfRanges;
		}
		++itListOfRanges;
	}
	return itLastRangeBytes;
}

bool appendNode(lintervalPositionByteDisk &listIntervalPositionByteDisk, intervalPositionByteDisk newInterval) {
	//~ if (newInterval.position < 0)
		//~ return false;
	if (listIntervalPositionByteDisk.empty()) {
		listIntervalPositionByteDisk.push_front(newInterval);
		return true;
	}
	lintervalPositionByteDisk::iterator iteratorLastInterval = getlastnode(listIntervalPositionByteDisk);
	if( iteratorLastInterval->b + 1 == newInterval.a ){
		iteratorLastInterval->b = newInterval.b;
		return true;
	}
	++iteratorLastInterval;
	listIntervalPositionByteDisk.insert(iteratorLastInterval, newInterval);
	return true;	
}

bool appendSubNode(lintervalPositionByteDisk &listIntervalPositionByteDisk, intervalPositionByteDisk newInterval, long long int lenght_) {
	
	//~ if (newInterval.position < 0)
		//~ return false;
	if ( listIntervalPositionByteDisk.empty() ) {
		newInterval.b = lenght_ + newInterval.a - 1;
		listIntervalPositionByteDisk.push_front(newInterval);
		return true;
	}
	lintervalPositionByteDisk::iterator iteratorLastInterval = getlastnode(listIntervalPositionByteDisk);
	if ( iteratorLastInterval->b + 1 == newInterval.a ) {
		iteratorLastInterval->b = lenght_ + newInterval.a - 1;
		return true;
	}
	newInterval.b = lenght_ + newInterval.a - 1;
	++iteratorLastInterval;
	listIntervalPositionByteDisk.insert(iteratorLastInterval, newInterval);
	return true;
}

/*
 * Retorna el último lugar entero de bytes donde se pueden escribir más datos .
 */
long long int getPointEnd(lintervalPositionByteDisk listIntervalPositionByteDisk) {
	if ( listIntervalPositionByteDisk.empty() )
		return 0;
	lintervalPositionByteDisk::iterator iteratorLastInterval = getlastnode(listIntervalPositionByteDisk);
	return iteratorLastInterval->position + iteratorLastInterval->b - iteratorLastInterval->a + 1;
}

bool compareIntervals(intervalPositionByteDisk &i1, intervalPositionByteDisk &i2) {
	return i1.a < i2.a;
}

/*
 * Consigue una lista de nodos del tipo llista.
 * Esta contiene el orden de los nodos a trabajar (cada nodo es un bloque de datos de un archivo).
 * getRangeWork retorna el valor hit y la lista de bloques a trabajar.
 * 
 */
lintervalPositionByteDisk getRangeWork(lintervalPositionByteDisk &listIntervalPositionByteDisk, long long int interval_a, long long int interval_b, bool *hit) {
	
	lintervalPositionByteDisk pr;
	*hit = 1;
	long long int tope = -1;
	
	if ( !listIntervalPositionByteDisk.empty() ) {
		
		listIntervalPositionByteDisk.sort(compareIntervals);
		lintervalPositionByteDisk::iterator itIntervalPos = listIntervalPositionByteDisk.begin(); 
		
		while ( itIntervalPos != listIntervalPositionByteDisk.end() ) {
			if ( interval_a < itIntervalPos->a )
			{
				tope = interval_a;
				break;
			}
			else
			{
				if ( interval_a >= itIntervalPos->a && interval_a <= itIntervalPos->b )
				{
					if( interval_b <= itIntervalPos->b )
					{
						pr.clear();
						intervalPositionByteDisk nn;
						nn.a = interval_a;
						nn.b = interval_b;
						nn.position = itIntervalPos->position + interval_a - itIntervalPos->a;
						pr.push_front(nn);
						*hit = 1;
						return pr;
					}
					intervalPositionByteDisk nn;
					nn.a = interval_a;
					nn.b = itIntervalPos->b;
					nn.position = itIntervalPos->position + interval_a - itIntervalPos->a;
					pr.push_front(nn);
					tope = itIntervalPos->b + 1;
					++itIntervalPos;
					break;
				}
			}
			++itIntervalPos;
		}
		while( tope >= 0 && itIntervalPos !=  listIntervalPositionByteDisk.end() ) {
			if ( interval_b < itIntervalPos->a )
			{
				intervalPositionByteDisk nn;
				nn.a = tope;
				nn.b = interval_b;
				nn.position = -1;
				pr.push_back(nn);
				*hit = 0;
				tope = interval_b + 1;
				break;
			}
			else {
				if ( tope < itIntervalPos->a ) {
					intervalPositionByteDisk nn;
					nn.a = tope;
					nn.b = itIntervalPos->a - 1;
					nn.position = -1;
					pr.push_back(nn);
					tope = itIntervalPos->a;
					*hit = 0;
				}
				else {
					//warning
				}
				if( interval_b >= itIntervalPos->a && interval_b <= itIntervalPos->b)
				{
					intervalPositionByteDisk nn;
					nn.a = tope;
					nn.b = interval_b;
					nn.position = itIntervalPos->position;
					pr.push_back(nn);
					tope = itIntervalPos->b + 1;
					break;
				}
				else
					pr.push_back(*itIntervalPos);
			}
			tope = itIntervalPos->b + 1;
			++itIntervalPos;
		}
	}
	if(tope == -1)
	{
		pr.clear();
		intervalPositionByteDisk nn;
		nn.a = interval_a;
		nn.b = interval_b;
		nn.position = -1;
		*hit = 0;
		pr.push_back(nn);
		return pr;
	}
	if( interval_b > tope )
	{
		intervalPositionByteDisk nn;
		nn.a = tope;
		nn.b = interval_b;
		nn.position = -1;
		*hit = 0;
		pr.push_back(nn);
	}
	return pr;	
}
string trimstr(string str) {
	size_t pos = str.find_first_not_of(" \t");
	if(pos==string::npos)
		return "";
	return str.substr(pos,str.find_last_not_of(" \t") - pos + 1);
}
//~ Convert "123-200,11-22,.." + "0,123,444..." to list
int generateList(string ranges, string parts, lintervalPositionByteDisk &listIntervalPositionByteDisk) {
	listIntervalPositionByteDisk.clear();
	vector<string> resul,rang,part;
	stringexplode(ranges,",",&resul);
	stringexplode(parts, ",",&part);
	if ( part.size() != resul.size() )
		return 0;
	for ( int i=0; i < (int)resul.size(); i++ )
	{
		intervalPositionByteDisk n;
		stringexplode(resul.at(i),"-",&rang);
		if (rang.size() < 2)
			return 0;
		n.a = atoll(rang.at(0).c_str());
		n.b = atoll(rang.at(1).c_str());
		if (n.a > n.b)
			return 0;
		n.position = atoll(part.at(i).c_str());
		listIntervalPositionByteDisk.push_front(n);
		rang.clear();
	}
	return 1;
}

void list2string(lintervalPositionByteDisk listIntervalPositionByteDisk, string &intervals, string &positions) {
	stringstream out,out1;
	lintervalPositionByteDisk::iterator it = listIntervalPositionByteDisk.begin();
	string coma = "";
	while ( it != listIntervalPositionByteDisk.end() ) {
		out<<coma<<it->a<<"-"<<it->b;
		out1<<coma<<it->position;
		coma = ",";
		++it;
	}
	intervals = out.str();
	positions = out1.str();
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

int SearchReplace(string &source, string search, string replace) {
	int numberReplace = 0;
    string::size_type position = source.find(search);

    while (position != string::npos) {
        source.replace(position, search.size(), replace);
        position = source.find(search);
        numberReplace++;
    }
    return numberReplace;
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

void stringexplodetrim(string str, string separator, vector<string>* results) {
    size_t found;
    found = str.find_first_of(separator);
    while (found != string::npos) {
        if (found > 0) {
            results->push_back(trimstr(str.substr(0, found)));
        }
        str = str.substr(found + 1);
        found = str.find_first_of(separator);
    }
    if (str.length() > 0) {
        results->push_back(trimstr(str));
    }
}

void splitstring(string str, string separator, vector<string>* results) {
    size_t found;
    found = str.find(separator);
    while (found != string::npos) {
        if (found > 0) {
            results->push_back(str.substr(0, found));
        }
        str = str.substr(found + separator.size());
        found = str.find(separator);
    }
    if (str.length() > 0) {
        results->push_back(str);
    }
}

string getdomain(string url) {
    if (regex_match("^74\\.125\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?$)", url) != "") return "youtube.com";
    if (regex_match("^(205\\.196\\.|199\\.91\\.)[0-9]{2,3}\\.[0-9]{1,3}", url) != "") return "mediafire.com";
    if (regex_match("^(\\.|[a-z]|[0-9]|-)+(\\/\\w+)?(\\/speedtest)+\\/(random[0-9]+x[0-9]+\\.jpg|latency\\.txt)", url) != "") return "speedtest.net";
    if (regex_match("^[0-9]{2,3}\\.[0-9]{2,3}\\.[0-9]{2,3}\\.[0-9]{1,3}\\/youku\\/", url) != "") return "youku.com";
    if (regex_match("198\\.38\\.(9[6-9]|1[0-2][0-9])\\.[0-9]{1,3}\\/range\\/", url) != "") return "netflix.com";
    if (regex_match("108\\.175\\.(3[2-9]|4[0-9])\\.[0-9]{1,3}\\/range\\/", url) != "") return "netflix.com";

	
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
                    (resultado.at(resultado.size() - 1)).size() <= 3 && resultado.at(resultado.size() - 2) != "avg" ) {
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
string regex_match_nocase(string er, string line) {
    int error;
    regmatch_t match;
    regex_t reg;
    if ((regcomp(&reg, er.c_str(), REG_EXTENDED | REG_NEWLINE | REG_ICASE)) == 0) {
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

double disk_use(string path) {
    double bfree, btotal = 0;
    struct statvfs fiData;
    if ((statvfs(path.c_str(), &fiData)) < 0) {
        return -1;
    } else {
        bfree = (((double) fiData.f_bsize) * ((double) fiData.f_bfree));
        btotal = (((double) fiData.f_bsize) * ((double) fiData.f_blocks));
        return ((btotal - bfree) / btotal)*100;
    }
}

//______________________________________________________________________________

double disk_size(string path) {
    struct statvfs fiData;
    if ((statvfs(path.c_str(), &fiData)) < 0) {
        return -1;
    } else
        return ((double) fiData.f_bsize)*((double) fiData.f_blocks);
}

//______________________________________________________________________________

double disk_occupation(string path) {
    struct statvfs fiData;
    if ((statvfs(path.c_str(), &fiData)) < 0) {
        return -1;
    } else
        return ((double) fiData.f_bsize)*(((double) fiData.f_blocks)-((double) fiData.f_bfree));
}

//______________________________________________________________________________



//______________________________________________________________________________

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

void longSeekpFile(fstream &f, long long int pos) {
	int lint = 2000000000;
	long long int _div =  pos/lint;
	if (_div) {
		f.seekg(lint, ios::beg);
		--_div;
		pos -= lint;
		for(int i = 0; i < _div; i++) {
			f.seekg(lint, ios::cur);
			pos -= lint;
		}
		f.seekg(pos, ios::cur);
	} else {
		f.seekg(pos, ios::beg);
	}
}










