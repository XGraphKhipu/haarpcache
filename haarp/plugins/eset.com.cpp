#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o eset.com.so eset.com.cpp
// http.{1,10}update\.eset\.com\/v[0-9]+(.*\/){2}.*\.nup$
//
string get_filename(string url) {
	vector<string> split;
	stringexplode(url, "/", &split);
	int len = split.size();
	if ( len >= 3 ) {
		return split.at(len-3) + DELIM + split.at(len-2) + DELIM + split.at(len-1);
	} else 
		return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
		
	if ( url.find("?") == string::npos ) {
	    	r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "eset";
		} else 
			r.match = false;
	} else 
		r.match = false;
	return r;
}
