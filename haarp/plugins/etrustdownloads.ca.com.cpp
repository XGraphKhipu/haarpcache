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
// g++ -I. -fPIC -shared -g -o etrustdownloads.ca.com.so etrustdownloads.ca.com.cpp  

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
	
	if ( (url.find(".pkg") != string::npos) or (url.find(".exe") != string::npos) or
			(url.find(".zip") != string::npos) or (url.find(".tar") != string::npos)
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "etrust";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}

	return r;
}

