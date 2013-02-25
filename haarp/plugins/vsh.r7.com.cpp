/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include "../utils.cpp"
 
using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o r7.com.so r7.com.cpp  

string get_filename(string url,int *ra, int *rb) {
	vector<string> resultado;
	stringexplode(url,"/",&resultado);
	
	return resultado.at(resultado.size() - 1);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if ( (url.find("?") == string::npos)
	) {
	    r.file = get_filename(url, &r.range_min, &r.range_max);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "r7";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

