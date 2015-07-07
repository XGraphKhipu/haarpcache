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

// g++ -I. -fPIC -shared -g -o mp3-buscador.com.so mp3-buscador.com.cpp
// regex
// http.*\.mp3-buscador\.com.*\.mp3
// http://musica.mp3-buscador.com/0001/wisin-y-yandel-pegate.mp3

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

	if ( (url.find(".mp3-buscador.com/") != string::npos) and ( (url.find(".mp3") != string::npos) ) 
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "mp3-buscador";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
