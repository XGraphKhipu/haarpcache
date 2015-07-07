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

// g++ -I. -fPIC -shared -g -o hulkshare.com.so hulkshare.com.cpp
// regex
// http.*\.hulkshare\.com.*\.mp3
// http://cdn07.hulkshare.com/dev7/0/005/731/0005731392.fid/PSY_-_Gangnam_Style_remix_dj_dann.mp3?key=f075c34975332bb257d35f13c0609700&dl=1

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

	if ( (url.find(".hulkshare.com/") != string::npos) and ( (url.find(".mp3") != string::npos) ) 
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "hulkshare";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
