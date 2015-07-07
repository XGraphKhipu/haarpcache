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
// g++ -I. -fPIC -shared -g -o bitdefender.com.so bitdefender.com.cpp
// ^http.{3,4}download\.bitdefender\.com\/((\w|\_|-)*\/){7}(\w|\_|-)*(\.exe)$
//
string get_filename(string url) {
	vector<string> resultado;
	stringexplode(url, "/", &resultado);
	int size = resultado.size();
	if(size < 9)
		return "";	

	string fname = resultado.at(size - 8);

	for(int i = 7; i >= 1; i--) 
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
			r.domain = "bitdefender";
		} else
			r.match = false;
		
	} else 
		r.match = false;
	
	return r;
}

