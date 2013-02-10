#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o avgate.net.so avgate.net.cpp
// copy avgate.net.so to avgate.com.so, freeav.net.so, freeav.com.so

bool in_array(const string &needle, const vector< string > &haystack) {
    int max = haystack.size();

    if (max == 0) return false;
    
	for (int iii = 0; iii < max; iii++) {
        if (regex_match(haystack[iii], needle) != "") {
            return true;
		}
	}
    return false;
}


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
		
	vector<string> black_list; // tudo para não usar array? o que tem de mais em um array...
	black_list.push_back ("ave2.info.gz");
	black_list.push_back ("info-wks-classic-nt-en.info.gz");
	black_list.push_back ("specvir-nt.info.gz");
	black_list.push_back ("vdf.info.gz");
	black_list.push_back ("classic-nt-en.info.gz");
	
	if ( ( (url.find(".avgate.net/") != string::npos) or (url.find(".avgate.com/") != string::npos) or
		   (url.find(".freeav.net/") != string::npos) or (url.find(".freeav.com/") != string::npos) )  and
	     ( (url.find(".gz") != string::npos) or (url.find(".zip") != string::npos) or (url.find(".exe") != string::npos) ) and 
		 (in_array(url, black_list) == false)
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "avgate";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
