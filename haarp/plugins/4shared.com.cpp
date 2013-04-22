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
// g++ -I. -fPIC -shared -g -o 4shared.com.so 4shared.com.cpp  
//~ 
string get_filename(string url, bool *pre, int *ra, int *rb) {
	vector<string> resultado;
	string::size_type pos = 0;
	string sg = regex_match("[\\?&]start=[0-9]+",url);
	if(sg != "") {
		sg.erase(0,7);
		*ra = atoi(sg.c_str());
		*rb = -1;
	}
	if( url.find(".com/download/") != string::npos ) {
		SearchReplace(url,"?","/");
		stringexplode(url,"/",&resultado);
		int size = resultado.size();
		if(size < 4)
			return "";
		return resultado.at(size - 3) + "-" + resultado.at(size - 2);		
	}
	else if( url.find(".com/img/") != string::npos ) {
		*pre = true;
		pos = url.find("_2Fdownload_2F");
		if( pos == string::npos)
			return "";
		url.erase(0,pos + 14);
		pos = url.find("_3Ftsid_");
		if( pos == string::npos)
			return "";
		url.erase(pos);
		SearchReplace(url,"_2F","-");
		SearchReplace(url,"_5F","_");
		if( url.find("_3F") != string::npos )
			return "";
		if( url.find("-") != string::npos )
			if(regex_match("\\.([0-9]|[a-zA-Z]){1,8}$", url) == "")
				url = url + ".flv";
		return url;
	}
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	bool preview;
	//~ 
	if ( (url.find("preview.flv?file=") == string::npos)
	) {
		
	    r.file = get_filename(url, &preview, &r.range_min, &r.range_max);
		if (!r.file.empty()) {
			r.match = true;
				r.domain = "4shared";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

