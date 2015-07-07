/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. 
 * Some Rights Reserved. 
 * 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 * @autor Manolo Canales (keikurono01) <kei.haarpcache@gmail.com>
 */
 
#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o vimeo.com.so vimeo.com.cpp  
// copy vimeo.com.so to bitgravity.com.so
//~ ^http.{1,4}(\w|\.|-)*\.vimeo(cdn)?.com\/.*\.mp4(\?.*)?$
string get_filename(string url, long long int *ra, long long int *rb) {
	vector<string> resultado;
	stringexplode(url,"/",&resultado);
	url = resultado.at(resultado.size() - 1);
	if (url.find("?") != string::npos) {
		resultado.clear();
		stringexplode(url, "?", &resultado);
		return resultado.at(0);           
	} else {
		return url;
	}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	
	if ( regex_match("[&\?]\\w*(time|start)", url) == ""  ) {
	    r.file = get_filename(url, &r.range_min, &r.range_max);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "vimeo";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

