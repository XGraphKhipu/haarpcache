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
// g++ -I. -fPIC -shared -g -o submanga.com.so submanga.com.cpp  

string get_filename(string url) {
		vector<string> res;
		stringexplode(url, "/", &res);
		int f = res.size()-1;
		string hd = "";
		if(url.find(".submanga.com/hdpages/") != string::npos)
			hd = "hd-";
		return hd + res.at(f - 2) + "-" + res.at(f - 1) + "-" + res.at(f);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if ( ((url.find(".submanga.com/pages/") != string::npos) || (url.find(".submanga.com/hdpages/") != string::npos)) && (url.find("?") == string::npos)
	) {
	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "submanga";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}


