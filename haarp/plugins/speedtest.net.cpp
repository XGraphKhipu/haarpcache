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
// g++ -I. -fPIC -shared -g -o speedtest.com.so speedtest.com.cpp  
//~ URL EXP:
//~ http://speedtest.movistar.com.pe/speedtest/latency.txt?x=1365780932390
//~ http://speedtest4.claro.com.pe/speedtest/random750x750.jpg?x=1367893686447&y=2
//~ haarp.lst:
//~ http.{1,4}speedtest(\w|-)*(\.|\w)+\/speedtest\/random.*(\.jpg)\?.*

string get_filename(string url) {
	vector<string> resultado;
	SearchReplace(url, "?", "/");
	stringexplode(url, "/", &resultado);
	if( resultado.size() > 6 )
		return "";
	
	return resultado.at(resultado.size() - 2);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	r.file = get_filename(url);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "speedtest";
	} else 
		r.match = false;
		
	return r;
}

