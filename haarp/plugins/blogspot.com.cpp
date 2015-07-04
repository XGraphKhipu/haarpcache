#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile 
// g++ -I. -fPIC -shared -g -o blogspot.com.so blogspot.com.cpp
// Regex
// ^http.{1,10}\.bp\.blogspot\.com\/.*(\.jpg|\.mp4|\.flv|\.swf)$
//
string get_filename(string url) {
	vector<string> resultado;
	string filename = "";

	stringexplode(url, "/", &resultado);

	int size = resultado.size();

	if ( size < 6 ) 
		return "";

	for( int i = 5;i >= 2; i-- ) 
		filename = filename + resultado.at(size - i) + "_";

	return filename = filename + resultado.at(size - 1);
}
 
extern "C" resposta hgetmatch2(string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if( url.find("?") != string::npos ) {
		r.match = false;
		return r;
	}
	
	r.file = get_filename(url);
	if( (r.file).empty() ) {
		r.match = false;
		return r;
	}
	r.domain = "blogspot";
	r.match = true;
	return r;
	
}
