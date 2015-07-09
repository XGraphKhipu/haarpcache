/*
 * (c) Copyright 2013 Erick Colindres <firecoldangelus@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Erick Colindres <firecoldangelus@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o modovideo.com.so modovideo.com.cpp
// Regex
// http.*\.modovideo\.com.*(\.flv|\.mp4)

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
	 r.range_min = r.range_max = 0;


	if ( (url.find("modovideo.com/") != string::npos) and ( (url.find(".flv") != string::npos) or (url.find(".mp4") == string::npos) ) 
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "modovideo";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
