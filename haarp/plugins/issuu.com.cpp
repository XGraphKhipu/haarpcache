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
// g++ -I. -fPIC -shared -g -o issuu.com.so issuu.com.cpp  
//~ 
string get_filename(string url) {
	vector<string> resultado;
	
	stringexplode(url,"/",&resultado);
	int size = resultado.size();
	if(size < 4)
		return "";
	return resultado.at(size - 3) + "_" + resultado.at(size - 2) + "_" + resultado.at(size - 1);
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if(url.find("?") == string::npos) {
		r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "issuu";
		} else {
			r.match = false;
		}
	}
	else {
		r.match = false;
	}
	return r;
}

