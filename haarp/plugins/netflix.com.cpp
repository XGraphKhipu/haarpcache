/*
 * (c) Copyright 2015 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o netflix.com.so netflix.com.cpp
// 198\.38\.(9[6-9]|1[0-2][0-9])\.[0-9]{1,3}\/range\/[0-9]+-[0-9]+\?.*watchid=[0-9]+
// ^http.{3,4}secure\.netflix\.com\/.*\/html\/wwwplayer-2.0000.2[0-9]{2}.011.js\?.*watchid=[0-9]+
// ^http.{3,4}www\.netflix\.com\/watch\/[0-9]+(\?.*|$)
// 
string getNID(string url) {
	vector<string> liststr;
	stringexplode(url, "?", &liststr);
	url = liststr.at(0);
	liststr.clear();
	stringexplode(url, "/", &liststr);
	return liststr.at(liststr.size() - 1);
}
string getNIDjs(string url) {
	//~ http://secure.netflix.com/en_us/ffe/player/html/wwwplayer-2.0000.257.011.js?watchid=80033768
	vector<string> liststr;
	stringexplode(url, "?", &liststr);
	url = liststr.at(1);
	liststr.clear();
	stringexplode(url, "=", &liststr);
	return liststr.at(1);
}
string getNIDrange(string url, long long int *ra, long long int *rb) {
	vector<string> lstr, lstr1, lstr2;
	string nameID, code;
	nameID = code = "";
	string s1,s2;
	stringexplode(url, "?", &lstr);
	s1 = lstr.at(0);
	s2 = lstr.at(1);
	lstr.clear();
	
	stringexplode(s1, "/", &lstr);
	stringexplode(lstr.at(lstr.size() - 1), "-", &lstr2);
	*ra = atoll(lstr2.at(0).c_str());
	*rb = atoll(lstr2.at(1).c_str());
	lstr.clear(); lstr1.clear();
	stringexplode(s2, "&", &lstr);
	for( int i = 0; i < lstr.size(); i++ ){
		stringexplode(lstr.at(i), "=", &lstr1);
		if( lstr1.size() != 2 )
			continue;
		if( lstr1.at(0) == "watchid" )
			nameID = lstr1.at(1);
		else if( lstr1.at(0) == "o" )
			code = lstr1.at(1);
		lstr1.clear();
	}
	if( !nameID.empty() && !code.empty() ) {
		return nameID + DELIM + code;
	}
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;

	if( regex_match("\\/watch\\/[0-9]+", url) != "" ) {
			r.file = getNID(url);
			r.domain = "NetflixID";
			r.match = false;
	} else if( url.find("wwwplayer") != string::npos || url.find("cadmium-playercore") != string::npos ) {
			//r.file = getNIDjs(url);
			r.file = "NetflixjsID";
			r.domain = "NetflixjsID";
			r.match = false;
	} else if( url.find("/range/") != string::npos ) {
			r.file = getNIDrange(url, &r.range_min, &r.range_max);
			if( !(r.file).empty() && r.range_min ) {
				r.domain = "netflix";
				r.match = true;
			} else {
				r.domain = "netflix";
				r.match = false;
			}
	} else
		r.match = false;
	return r;
}

