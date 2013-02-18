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

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o tumblr.com.so tumblr.com.cpp

string get_videoid(string url){
	vector<string> resultado;
	string retorna = "";
	
	stringexplode(url, "/", &resultado);
	return resultado.at(resultado.size() - 1);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	if ( url.find("?") == string::npos ) {
		r.file = get_videoid(url);
		r.match = true;
		r.domain = "tumblr";
	} else {
		r.match = false;
	}
    return r;
}


