#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"


// use this line to compile
// g++ -I. -fPIC -shared -g -o avg.com.so avg.com.cpp  
// copy avg.com.so to grisoft.com.so and grisoft.cz.so

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

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;

	if ( ( (url.find("avg.com/") != string::npos) or (url.find("grisoft.com/") != string::npos) or (url.find("grisoft.cz/") != string::npos) )  and
		 ( (url.find(".exe") != string::npos) or (url.find(".bin") != string::npos) )
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "avg";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	
	return r;
	
}
