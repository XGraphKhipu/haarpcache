/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */
 

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o rncdn3.com.so rncdn3.com.cpp
//
string get_filename(string url, long long int *ra, long long int *rb) {
	vector<string> resultado;
	stringexplode(url,"/",&resultado);

	url = resultado.at(resultado.size() - 1);

	resultado.clear();

	SearchReplace(url,"?","/");
	stringexplode(url, "/", &resultado);
	return resultado.at(0);
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
    r.range_min = 0;
	r.range_max = 0;
	
	r.file = get_filename(url, &r.range_min, &r.range_max);
	if ( !r.file.empty() && (regex_match("[&\\?]ms=[0-9]+", url) == "") ) {
		r.match = true;
		r.domain = "pornhub_vid";
	} else {
		r.match = false;
	}
	return r;
}

