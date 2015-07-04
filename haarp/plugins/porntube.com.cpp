#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o porntube.com.so porntube.com.cpp  

string get_filename_vid(string url) {
	vector<string> resultado, uri;
	stringexplode(url, "/", &resultado);
	string file = "";
	int size = resultado.size();
	for(int i = 10; i >= 2; i--) {
		file = file + resultado.at(size - i) + "_";
	}	
	stringexplode(resultado.at(size - 1), "?", &uri);
	return file + uri.at(0);
}

string get_filename_img(string url) {
	vector<string> resultado, uri;
	stringexplode(url, "/", &resultado);
	string file = "";
	int size = resultado.size();
	for(int i = 11; i >= 2; i--) {
		file = file + resultado.at(size - i) + "_";
	}
	return file + resultado.at(size - 1);
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;

	if ( url.find("videos") != string::npos ) {
		if( (url.find("start=") != string::npos) && (regex_match("(\\?|&)start=0(&|$)", url) == "") ) {
			r.match = false;
			return r;
		}
		r.file = get_filename_vid(url);
		r.domain = "porntube_vid";
		r.match = true;
	} else {
		if( url.find("?") != string::npos ) {
			r.match = false;
			return r;
		}
		r.file = get_filename_img(url);
		r.domain = "porntube_img";
		r.match = true;
	}
	return r;
	
}

