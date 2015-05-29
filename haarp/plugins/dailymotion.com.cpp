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
// g++ -I. -fPIC -shared -g -o dailymotion.com.so dailymotion.com.cpp  
//~ 
string get_filename(string url) {
	vector<string> resultado;
	string path, filename;
	stringexplode(url,"/",&resultado);
	int size = resultado.size();
	for (int i = 0; i < size; i++) {
		path = resultado.at(i);
		if( regex_match("^sec\\([0-9a-zA-Z]*\\)$", path) != "" ) {
			SearchReplace(path, "(", "_");
			SearchReplace(path, ")", "_");
			filename = path;
		} else if ( regex_match("^frag\\([0-9]*\\)$", path) != "" ) {
			SearchReplace(path, "(", "_");
			SearchReplace(path, ")", "_");
			return filename + path + resultado.at(size - 1);
		}
	}
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if(url.find("?") == string::npos) {
		r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "dailymotion";
		} else {
			r.match = false;
		}
	}
	else {
		r.match = false;
	}
	return r;
}

