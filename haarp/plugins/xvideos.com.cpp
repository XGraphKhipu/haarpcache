#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o xvideos.com.so xvideos.com.cpp

string get_filename(string url, int *ra, int *rb) {
	vector<string> resultado;
	string::size_type pos;
	string tmp;
	
	stringexplode(url,"/",&resultado);
	url = resultado.at(resultado.size() - 1);
	resultado.clear();
	
	if (url.find("?") != string::npos) {
		stringexplode(url, "?", &resultado);
		if( ( tmp = regex_match("[\\?&]fs=[0-9]+", resultado.at(1)) ) != "" ) {
			return "";
		}
		url = resultado.at(0);
		if( (pos = url.find(";")) != string::npos )
			url.erase(pos);
		return url;
	} else {
		if( (pos = url.find(";")) != string::npos )
			url.erase(pos);
		return url;
	}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	r.file = get_filename(url, &r.range_min, &r.range_max);
	if ( !r.file.empty() ) {
		r.match = true;
		r.domain = "xvideos";
	} else {
		r.match = false;
	}
	return r;
}

