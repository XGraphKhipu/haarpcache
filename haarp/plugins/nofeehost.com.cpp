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

// g++ -I. -fPIC -shared -g -o nofeehost.com.so nofeehost.com.cpp
// regex
// http.*\.nofeehost\.com.*\.mp3
// http://users6.nofeehost.com/sosoniderito/Regueton/Wisin%20Y%20Yandel%20-%2008%20-%20Yandel.%20Franco%20(El%20Gorilla)%20-%20Vamo%20a%20Hacerlo.mp3

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

	if ( (url.find(".nofeehost.com/") != string::npos) and ( (url.find(".mp3") != string::npos) ) 
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "nofeehost";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
