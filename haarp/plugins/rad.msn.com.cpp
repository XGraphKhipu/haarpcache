#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o msn.com.so msn.com.cpp  

string rewriteurl(string url) {
	string banner = "zion2.zionlanhouse.com.br:8080/msn.htm";

	if ( url.find("rad.msn.com/ADSAdClient31.dll?") != string::npos ){
		url = banner;
		return url;
	} else 
		return "";
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	
	r.file = rewriteurl(url);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "rewrite";
	} else {
		r.match = false;
	}
	return r;
}
