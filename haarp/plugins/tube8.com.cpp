/* 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o tube8.com.so tube8.com.cpp

string get_filename(string url) {
		vector<string> resultado;
		if (url.find("?") != string::npos) {
			stringexplode(url, "?", &resultado);
			stringexplode(resultado.at(0), "/", &resultado);
			return resultado.at(resultado.size()-1);           
		} else {
			stringexplode(url, "/", &resultado);
			return resultado.at(resultado.size()-1);
		}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	
	if ( (url.find(".flv") == string::npos ) and (url.find("start=") == string::npos) // tirar start para nao fazer cache de file incompleto
	) {
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "tube8";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

