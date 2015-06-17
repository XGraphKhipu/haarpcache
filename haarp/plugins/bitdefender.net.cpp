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
// g++ -I. -fPIC -shared -g -o bitdefender.net.so bitdefender.net.cpp
// ^http.{3,10}patches.{1,40}\.bitdefender\.net\/.*\/patches\/.*(\.gzip|\.zip)$
// 
string get_filename(string url) {
	vector<string> resultado;
	stringexplode(url, "/", &resultado);
	string fname = resultado.at(resultado.size() - 1);
	if( fname.size() > 70 )
		return fname;
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	
	if( url.find("?") == string::npos ) {
		r.file = get_filename(url);
		if ( !r.file.empty() ) {
			r.match = true;
			r.domain = "bitdefender";
		} else
			r.match = false;
		
	} else 
		r.match = false;
	
	return r;
}

