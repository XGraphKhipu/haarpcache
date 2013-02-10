#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o xtube.com.so xtube.com.cpp
//~ http.*\.(publicvideo|publicphoto)\.xtube\.com\/(videowall\/)?videos?\/.*(\.flv\?.*|\_Thumb\.flv$)
//~ 
string get_filename(string url, int *ra, int *rb) {
	vector<string> resultado;
	stringexplode(url,"/",&resultado);
	url = resultado.at(resultado.size() - 1);
	
	if (url.find("?") == string::npos) {
		return url;
	} else {
		resultado.clear();
		stringexplode(url, "?", &resultado);
		string tmp;
		if( (tmp = regex_match("[\\?&]fs=[0-9]+",resultado.at(1))) != "" ) {
			tmp.erase(0,4);
			*ra = atoi(tmp.c_str());
			*rb = -1;
		}
		return resultado.at(0);
	}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
    r.range_min = 0;
    r.range_max = 0;

	r.file = get_filename(url, &r.range_min, &r.range_max);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "xtube";
	} else {
		r.match = false;
	}
	return r;
}

