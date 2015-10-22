#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp
typedef long long int lli;

bool getRanges(string range, lli *a, lli *b) {
	vector<string> interval;
	stringexplode(range,"-", &interval);
	if(interval.size() < 2) {
		return false;
	}
	*a = atoll(interval.at(0).c_str()) - 0;
	*b = atoll(interval.at(1).c_str()) - 0;
	return true;
}

string getiValue(string regex, string url, int index) {
	vector<string> explode;
	string regex_trash = regex_match(regex, url);
	stringexplode(regex_trash, "/", &explode);
	if( !explode.size() || explode.size() <= index )
		return "";
	return explode.at(index);
}

void get_videoid(string url, string &file, bool *exist_range, lli *a, lli *b, lli *clen) {
	vector<string> resultado,valor;
	
	string sclen, itag, mime;
	bool exist_cm2, range, watchID;
	int size;
	
	watchID = false;

	sclen = file = itag = mime = "";

	*exist_range = exist_cm2 = range = false;
	
	stringexplode(url, "?", &resultado);
	size = resultado.size();
	if ( size > 1 ) {
	    url = resultado.at(size - 1);
	    resultado.clear();
	    stringexplode(url, "&", &resultado);
	    for (int i=0; i <= resultado.size() - 1;i++) {
		    valor.clear(); 
		    stringexplode(resultado.at(i), "=", &valor);
		    if(valor.size() < 2)
				continue;
		    if(!valor.at(1).size()) {
				file = "";
				return;
		    }
		    if ( (valor.at(0) == "id" || valor.at(0) == "video_id") && !watchID) {
			    file  = valor.at(1);
			    if ( file.size() > 40 ) { // For videos with variables id.
					file = "";
				}
			}
		    else if (valor.at(0) == "itag" && valor.at(1) != "34") 
			    itag  = "-" + valor.at(1);
		    else if( valor.at(0) == "range") {
				range = true;
				if( !getRanges(valor.at(1), a, b) ) {
					file = "";
					return;
				}
				*exist_range = true;
			}
			else if( valor.at(0) == "cm2" && valor.at(1) == "0" ) {
				exist_cm2 = true;
			}
			else if ( valor.at(0) == "clen" ) {
				*clen = atoll(valor.at(1).c_str());
				sclen = "-" + valor.at(1);
			}
			else if ( valor.at(0) == "watchid" ) {
				file = valor.at(1);
				watchID = true;
			}
			else if( valor.at(0) == "mime" ) {
				if( valor.at(1).find("video") != string::npos )
					mime = "-vid";
				else if (valor.at(1).find("audio") != string::npos )
					mime = "-aud";
			}
	    }
	}
	if( exist_cm2 && !range ) {
		file = "";
		return;
	}
	if( !file.empty() )
		file = file + itag + mime;
	if(*clen && *clen >= *a && *clen <= *b) {
		file = "";
		return;
	}
	return;
}
void get_videoid2(string url, string &file, bool *exist_range, lli *a, lli *b, lli *clen) {
	vector<string> url_get, url_param, url_path, paths, values;
	bool watchID = false;
	string codeVide = "";
	stringexplode(url, "?", &url_get);
	if( url_get.size() != 2 ) {
		file = "";
		return;
	}
	stringexplode(url_get.at(1), "&", &url_param);
	for(int i = 0; i < url_param.size(); i++) {
		stringexplode(url_param.at(i), "=", &values);
		if( values.size() != 2 ) {
			values.clear();
			continue;
		}
		if ( values.at(0) == "watchid" ) {
			codeVide = values.at(1);
			watchID = true;
			break;
		}
		values.clear();
	}
	string urlDir = url_get.at(0);
	if ( !watchID ) {
		string id = getiValue("/id/\\w{16}/", urlDir, 1);
		if ( id.empty() ) {
			file = "";
			return;
		} 
		codeVide = id;
	}
	string itag = getiValue("/itag/[0-9]+/", urlDir, 1);
	if ( urlDir.find("/mime/video") != string::npos ) 
		file = codeVide + "-" + itag + "-vid";
	else
		file = codeVide + "-" + itag + "-aud";

	string ranges = getiValue("/range/[0-9]+-[0-9]+$", urlDir, 1);

	if ( !getRanges(ranges, a, b) ) {
		file = "";
		return;
	}
	*exist_range = true;

	string clen_str = getiValue("/clen/[0-9]+/", urlDir, 1);
	*clen = atoll(clen_str.c_str());
	
	if(*clen && *clen >= *a && *clen <= *b) {
		file = "";
		return;
	}
}
void get_watchID(string url, string &watchid) {
	vector<string> resultado, valor;
	int size;
	
	watchid = "";

	SearchReplace(url,"?","&");
	stringexplode(url, "/", &resultado);
	size = resultado.size();
	if ( size > 1 ) {
		url = resultado.at(size - 1);
		resultado.clear();
		stringexplode(url, "&", &resultado);
		for (int i=0; i <= resultado.size() - 1;i++) {
			valor.clear();
			stringexplode(resultado.at(i), "=", &valor);
			if(valor.size() != 2)
				continue;
			if(!valor.at(1).size()) {
				watchid = "";
				return;
			}
			if (valor.at(0) == "v" ) {
				watchid = valor.at(1);
				return;
			}
		}
	}
}

extern "C" resposta hgetmatch2(string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	r.exist_range = false;
	r.total_file_size = 0;
	
	if( url.find("/videoplayback?") != string::npos ) {
		if ( regex_match("[\\?&]begin=[0-9]*[1-9]+[0-9]*", url) == "" && regex_match("[\\?&]cms_redirect=yes(&.*)?$", url) == "" && regex_match("[\\?&]redirect_counter=1(&.*)?$", url) == "" &&  url.find("&ir=1") == string::npos && url.find("&rr=12") == string::npos && url.find("source=yt_live") == string::npos && url.find("&otf=1") == string::npos ) {
			get_videoid(url, r.file, &r.exist_range, &r.range_min, &r.range_max, &r.total_file_size);
			if ( !r.file.empty() ) {
				r.match = true;
				r.domain = "youtube";
				r.file += ".flv";
			}
			else
				r.match = false;
		} else 
			r.match = false;
	} else if (url.find("/videoplayback/") != string::npos ) {
		if ( regex_match("begin[/=][0-9]*[1-9]+[0-9]*", url) == "" && regex_match("cms_redirect[=/]yes", url) == "" && regex_match("redirect_counter[=/]1", url) == "" &&  regex_match("[\\?&/]ir[=/]1", url) == "" && regex_match("[\\?&/]rr[=/]12", url) == "" && regex_match("source[=/]yt_live", url) == "" && regex_match("[\\?&/]otf[/=]1", url) == "" && regex_match("cmd2[=/]0", url) == "" ) {
			get_videoid2(url, r.file, &r.exist_range, &r.range_min, &r.range_max, &r.total_file_size);
			if( !r.file.empty() ) {
				r.match = true;
				r.domain = "youtube";
				r.file += ".flv";
			} else 
				r.match = false;

		} else
			r.match = false;
	}
	else if ( url.find(".com/watch?") != string::npos ) {
		get_watchID(url, r.file);
		if( !r.file.empty() ) {
			r.domain = "youtube_IDs";
			r.match = false;
		} 
		else r.match = false;
	} else
		r.match = false;

	return r;
}
