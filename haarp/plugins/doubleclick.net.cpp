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
// g++ -I. -fPIC -shared -g -o doubleclick.net.so doubleclick.net.cpp

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	string find = "";
	string find2 = "";
	int len;
	if(url.find("www.youtube.com") != string::npos) {
		if ( ( (find = regex_match("[\\?&]ad_type=skippablevideo($|&)",url)) != "" || (find = regex_match("[\\?&]ad_type=video($|&)",url)) != "" || (find = regex_match("[\\?&]ad_type=text_image_flash($|&)",url)) != "" || (find = regex_match("[\\?&]ad_type=text($|&)",url)) != "" || (find = regex_match("[\\?&]ad_type=image($|&)",url)) != "" ) && ( (find2 = regex_match("[\\?&]url=http.{2,15}www.youtube.com([a-zA-Z0-9]|%|-|_)*&?",url)) != "" ) ) {
			r.domain = "rewrite";
			r.file = "googleads.g.doubleclick.net/pagead/ads";
			r.match = true;
			return r;
		}
	}
	r.match = false;
    return r;
}


