#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -o edgecastcdn.net.so edgecastcdn.net.cpp

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
	
	if (url.find("edgecastcdn.net/") != string::npos) {
	    r.match = false;		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			if ( (r.file.find(".mp4") != string::npos) or (r.file.find(".flv") != string::npos) or (r.file.find(".m4a") != string::npos) ) {
				r.match = true;
				r.domain = "edgecastcdn";
			} else if (r.file.find(".jpg") != string::npos) {
				r.match = true;
				r.domain = "edgecastcdn/thumbs";
			}
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	
	return r;
	
}
