/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.h"
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o kaspersky.com.so kaspersky.com.cpp
// copy kaspersky.com.so to kaspersky-labs.com.so
// copy kaspersky.com.so to kasperskyusa.com.so

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
	
	if ( (url.find(".bz2") != string::npos) or (url.find("/diffs/") != string::npos) or (url.find(".avc") != string::npos) or
			(url.find(".kdc") != string::npos) or (url.find(".klz") != string::npos) or (url.find(".dif") != string::npos) or
			(url.find(".dat") != string::npos) or (url.find(".exe") != string::npos) or (url.find(".kdz") != string::npos) or
			(url.find(".kdl") != string::npos) or (url.find(".kfb") != string::npos)
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "kaspersky";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}

	return r;	
}

