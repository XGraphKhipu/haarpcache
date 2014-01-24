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

string get_filename(string url, int *ra, int *rb) {
	vector<string> purl;
	stringexplode(url, ".", &purl);
	return purl.at(purl.size() - 2) + "." + purl.at(purl.size() - 1);
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	if ( url.find("?") == string::npos ) {
		r.file = get_filename(url, &r.range_min, &r.range_max);
		r.match = true;
		r.domain = "cloudfront";
	} else 
		r.match = false;
	
	return r;
}

