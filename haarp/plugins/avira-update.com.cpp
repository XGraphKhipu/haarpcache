/*
 * (c) Copyright 2015 Manolo Canales (keikurono01) <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales (keikurono01) <kei.haarpcache@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o avira-update.com.so avira-update.com.cpp
// ^http.{3,4}install\.avira-update\.com\/package\/((\w|\_|-)*\/){3}(\w|\_|-)*(\.exe)$
//
string get_filename(string url) {
	vector<string> resultado;
	stringexplode(url, "/", &resultado);
	int size = resultado.size();

	string fname = resultado.at(size - 4);

	for(int i = 3; i >= 1; i--) 
		fname = fname + "_-_" + resultado.at(size - i); 

	return fname;	
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	
	if( url.find("?") == string::npos ) {
		r.file = get_filename(url);
		if ( !r.file.empty() ) {
			r.match = true;
			r.domain = "avira";
		} else
			r.match = false;
		
	} else 
		r.match = false;
	
	return r;
}

