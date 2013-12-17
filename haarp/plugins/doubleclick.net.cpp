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
void get_param(string url, string value, string &param) {
        vector<string> resultado,valor;
        param = "";

        SearchReplace(url,"?","&");
        stringexplode(url, "/", &resultado);
        url = resultado.at(resultado.size() - 1);
        resultado.clear();
        stringexplode(url, "&", &resultado);
        for (int i=0; i <= resultado.size() - 1;i++) {
	        valor.clear();
        	stringexplode(resultado.at(i), "=", &valor);
                if(valor.size() < 2)
	           	continue;	
		if(valor.at(0) == value) {
			param = valor.at(1);
			return;
		}
	}
}
string getRename(string url) {
	int surl = (int)url.size();
	char *curl = (char *)malloc(sizeof(char)*(surl + 1));
	strcpy(curl,url.c_str());
	int pos = -1;
	for(int i =0;i<surl;i++) 
		if(curl[i] == '/')
			pos = i;	
	if( (pos > 0) && (pos < surl) )
		curl[++pos] = '\0';
	return string(curl);
}

extern "C" resposta hgetmatch2(const string url) {
    	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	string find = "";
	string find2 = "";
	int len;
	string param;
	if( url.find("/pfadx/") == string::npos ) {
		if(url.find("www.youtube.com") != string::npos) {
			get_param(url, "ad_type", param);
			if( ( param == "skippablevideo" || param == "video" || param == "text_image_flash" \
				 || param == "text" || param == "image" ) && ( regex_match("[\\?&]url=http.{2,15}www.youtube.com([a-zA-Z0-9]|%|-|_)*&?",url) != "" ) ) {
				r.domain = "rewrite";
				r.file = "googleads.g.doubleclick.net/pagead/ads";
				r.match = true;
				return r;
			}
		}
	}
	else {
		if( regex_match("/main.*;", url) != "" ) {
			r.domain = "rewrite";
			r.file = getRename(url);
			r.match = true;
			return r;
		}
	}
	r.match = false;
    return r;
}


