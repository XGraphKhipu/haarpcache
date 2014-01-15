#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

void get_videoid(string url, string &file, int *a, int *b) {
	vector<string> resultado,valor;
	
	string itag;
	bool exist_cm2, range;
	int size, clen;
	
	clen = 0;

	file = itag = "";
	exist_cm2 = range = false;
	
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
		    if(valor.size() < 2)
				continue;
		    if (valor.at(0) == "id" || valor.at(0) == "video_id") {
			    file  = valor.at(1);
			    if ( file.size() > 40 ) { // For videos with variables id.
					file = "";
					return;
				}
			}
		    else if (valor.at(0) == "itag" && valor.at(1) != "34") 
			    itag  = "-" + valor.at(1);
		    else if( valor.at(0) == "range") {
				range = true;
				vector<string> interval;
				stringexplode(valor.at(1),"-", &interval);
				if(interval.size() < 2) {
					file = "";
					return;
				}
				*a = atoi(interval.at(0).c_str()) - 0;
				*b = atoi(interval.at(1).c_str()) - 0;
			}
			else if( valor.at(0) == "cm2" && valor.at(1) == "0" ) {
				exist_cm2 = true;
			}
			else if ( valor.at(0) == "clen" ) {
				clen = atoi(valor.at(1).c_str());
			}
			/*else if( valor.at(0) == "mime" ) {
				if( valor.at(1).find("video") != string::npos )
					mime = "-vid";
				else if (valor.at(1).find("audio") != string::npos )
					mime = "-aud";
			}*/
	    }
    }
    if( exist_cm2 && !range ) {
		file = "";
		return;
	}
	if(!file.empty())
		file = file + itag;
	if(clen && clen >= *a && clen <= *b) {
		file = "";
		return;
	}
}

extern "C" resposta hgetmatch2(string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;

	if ( regex_match("[\\?&]begin=[0-9]*[1-9]+[0-9]*", url) == "" ) {
		get_videoid(url, r.file, &r.range_min, &r.range_max);
		if ( !r.file.empty() ) {
			r.match = true;
			r.domain = "youtube";
			/*if( regex_match("-aud(-[0-9]+)?$", r.file) != "" )
				r.file += ".mp4a";
			else if( regex_match("-vid(-[0-9]+)?$", r.file) != "" )
				r.file += ".mp4";
			else*/
				r.file += ".flv";
		}
		else
			r.match = false;
	} else {
		r.match = false;
	}
	return r;
}
