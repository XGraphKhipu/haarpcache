/*
 * (c) Copyright 2015 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o nflximg.net.so nflximg.net.cpp
// ^http.{3,15}\.nflximg(\.com|\.net)\/.*\/[0-9]+(\.webp|\.jpg)$
//
string get_filename(string url) {
	vector<string> resultado;
	stringexplode(url, "/", &resultado);
	return resultado.at(resultado.size() - 1);
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	
	if(url.find("?") == string::npos) {
		r.file = get_filename(url);
		r.match = true;
		r.domain = "nflximg";
	} else 
		r.match = false;
	return r;
}

