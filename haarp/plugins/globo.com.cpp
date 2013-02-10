#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o globo.com.so globo.com.cpp

string get_filename(string url) {
		vector<string> resultado;
		if (url.find("?") != string::npos) {
			stringexplode(url, "?", &resultado);
			stringexplode(resultado.at(resultado.size()-2), "/", &resultado);
			return resultado.at(resultado.size()-1);           
		} else {
			stringexplode(url, "/", &resultado);
			return resultado.at(resultado.size()-1);
		}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	
	if ( (url.find("flashvideo.globo.com") != string::npos)  and
		 ( (url.find(".mp4") != string::npos) or (url.find(".flv") != string::npos) )
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "globo";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
