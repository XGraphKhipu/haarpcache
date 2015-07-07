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

// g++ -I. -fPIC -shared -g -o ning.com.so ning.com.cpp
// regex
// http.*\.ning\.com.*\.mp3
// http://api.ning.com/files/cVbHHr*PLJtLaAnj3*Rq96R5KqQuvQcKJYdz7czJj92wxIrFqvHlvVhs*g-5RMQSNvSZ*yK-NxpmLo1RcfZxM-3QSkiAL4d8/ep-Um56kmKyWeJiVqWSLiJKboTQ$.mp3

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

extern "C" resposta getmatch(const string url) {
    resposta r;

	if ( (url.find(".ning.com/") != string::npos) and ( (url.find(".mp3") != string::npos) ) 
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "ning";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
