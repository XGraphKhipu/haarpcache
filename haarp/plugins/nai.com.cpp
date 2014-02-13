/* 
 * (c) Copyright 2009 Kei Kurono (keikurono01) <kei.haarpcache gmail.com>. Some Rights Reserved.
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o nai.com.so nai.com.cpp
// copy nai.com.so to update.nai.com.so

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
	black_list.push_back ("engmin.zip");
	black_list.push_back ("SiteStat.xml");
	black_list.push_back ("PkgCatalog.z");
	black_list.push_back ("V2enginstall.mcs");
	black_list.push_back ("V2engdet.mcs");
	black_list.push_back ("gdeltaavv.ini");
	black_list.push_back ("catalog.z");
	
	if ( (url.find(".nai.com/") != string::npos)  and
	   ( (url.find(".zip") != string::npos) or (url.find(".exe") != string::npos) or (url.find(".gem") != string::npos) or
	     (url.find(".tar") != string::npos) ) and (in_array(url, black_list) == false)
	) {
		
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "nai";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
