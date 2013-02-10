
#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o mediafire.com.so mediafire.com.cpp

string get_filename(string url) {
	vector<string> resultado;
	stringexplode(url, "/", &resultado);
	if( resultado.size() ==  4 )
		return resultado.at(2) + "-" + resultado.at(3);
	else
		return "";
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if ( url.find("?") == string::npos )
	{
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "mediafire";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
