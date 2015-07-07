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

// use this line to compile
// g++ -I. -fPIC -shared -g -o phncdn.com.so phncdn.com.cpp

string get_filename(string url) {
	vector<string> resultado;
	string tmp;
	string::size_type pos;
	stringexplode(url,"/",&resultado);
	int size = resultado.size();
	return resultado.at(size - 4) + "_" + resultado.at(size - 2) + "_" + resultado.at(size - 1);
}
string get_filenamevideo(string url) {
	vector<string> resultado;
                
	stringexplode(url,"/",&resultado);

	url = resultado.at(resultado.size() - 1);

	resultado.clear();

	SearchReplace(url,"?","/");
	stringexplode(url, "/", &resultado);
	return resultado.at(0);
}


extern "C" resposta hgetmatch2(const string url) {
	resposta r;
    r.range_min = 0;
	r.range_max = 0;

	if ( regex_match("\\.mp4(\\?|$)", url) != "" ) {
		if( (url.find("start=") != string::npos) && (regex_match("(\\?|&)start=0(&|$)", url) == "") ) {
			r.match = false;
			return r;
		}
		r.file = get_filenamevideo(url);
		if( r.file.empty() ) {
			r.match = false;
			return r;
		} 
		r.match = true;
		r.domain = "pornhub_vid";
		return r;
	}
	else if ( url.find("?") == string::npos ) {
		r.file = get_filename(url);
		if( !r.file.empty()  ) {
			r.match = true;
			r.domain = "pornhub_img";
		} else
			r.match = false;
	} else 
		r.match = false;
	
	return r;
}

