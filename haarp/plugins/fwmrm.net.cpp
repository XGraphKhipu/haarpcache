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

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o ytimg.com.so ytimg.com.cpp
//~ http://2975c.v.fwmrm.net/ad/p/1? -- POST

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	r.match = false;
	string find = "";
	if( (find = regex_match(".*fwmrm\\.net\\/ad\\/p\\/1\\?$",url)) != "") {
		r.domain = "rewrite";
		r.file = url;
		r.file.at(r.file.size() - 1) = '\0';
		r.match = true;
	}
    return r;
}


