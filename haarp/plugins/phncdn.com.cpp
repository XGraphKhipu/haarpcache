/* 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o pornhub.com.so pornhub.com.cpp

string get_filename(string url, int *ra, int *rb) {
		vector<string> resultado;
		string tmp;
		string::size_type pos;
		stringexplode(url,"/",&resultado);
		
		url = resultado.at(resultado.size() - 1);
		resultado.clear();
		
		if( (tmp = regex_match("[&\\?]fs=[0-9]+",url)) != "" )
		{
			tmp.erase(0,4);
			*ra = atoi(tmp.c_str()) - 13;
			*rb = -1;
		}
		SearchReplace(url,"?","/");
		stringexplode(url, "/", &resultado);
		return resultado.at(0);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
//~ http://cdn1b.video.pornhub.phncdn.com/videos/000/116/527/116527.flv?rs=125&ri=600&s=1333999318&e=1334001118&h=772e6913&fs=19081624
//~ http://cdn1b.video.pornhub.phncdn.com/videos/200809/29/162165/240P_400K_162165.mp4?rs=150&ri=1400&s=1360282464&e=1360284264&h=5f5f639bb36d8aade7ac46d6f9791d2b&ms=1354
	r.file = get_filename(url, &r.range_min, &r.range_max);
	if ( !r.file.empty() && (regex_match("[&\\?]ms=[0-9]+", url) == "") ) {
		r.match = true;
		r.domain = "pornhub";
	} else {
		r.match = false;
	}
	return r;
}

