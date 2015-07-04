#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o redtube.com.so redtube.com.cpp

string get_filename(string url, int *ra, int *rb) {
	vector<string> resultado;
	string tmp;
	if (url.find(".flv?") != string::npos) {
		stringexplode(url, "/", &resultado);
		url = resultado.at(resultado.size() - 1);
		resultado.clear();
		stringexplode(url, "?", &resultado);
		return resultado.at(0);
	} else {
		stringexplode(url, "/", &resultado);
		tmp = resultado.at(resultado.size()-1);
		if( regex_match(".flv$", tmp) == "" )
			return "";
		else
			return tmp;
	}
	return "";
}
extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	if( ( (url.find("start=") != string::npos) && (regex_match("(\\?|&)start=0(&|$)", url) == "") ) \
		|| ( url.find("ec_seek") != string::npos && regex_match("(\\?|&)ec_seek=0(&|$)", url) == "" ) ) {
		r.match = false;
		return r;
	}
	r.file = get_filename(url, &r.range_min, &r.range_max);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "redtubefiles";
	} else {
		r.match = false;
	}
	return r;
}

