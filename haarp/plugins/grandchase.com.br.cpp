#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o grandchase.com.br.so grandchase.com.br.cpp

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
	
	if ( (url.find("fpatch.grandchase.com.br/") != string::npos)  and
	     ((url.find(".kom") != string::npos)||(url.find(".mkom") != string::npos)||(url.find(".mp3") != string::npos))
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "grandchase";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	
}
