/*
 * (c) Copyright 2013 Erick Colindres <firecoldangelus@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Erick Colindres <firecoldangelus@gmail.com>
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o zgncdn.com.so zgncdn.com.cpp
// regex
// http.*\.zgncdn\.com.*(\.swf|\.mp3|\.png|\.jpg)

string dominiotxt="GF_zgncdn";
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
	 r.range_min = r.range_max = 0;
 
    if ( (url.find("empire") != string::npos) or (url.find("/empire/") != string::npos) 
       ) {
    dominiotxt="GF_zgncdn_empire";
    }
    if ((url.find("cityvillefb")!= string::npos) or (url.find("/city/") != string::npos) 
       ) {
    dominiotxt="GF_zgncdn_city";
    }
    if ( (url.find("farmville")!= string::npos) or (url.find("/farm/")!= string::npos) 
       ) {
    dominiotxt="GF_zgncdn_farm";
    }
   if ( (url.find("castle")!= string::npos) or (url.find("/castle/")!= string::npos) 
        ) {
    dominiotxt="GF_zgncdn_castle";
    }
    if ( (url.find("bubblesafari") != string::npos) or (url.find("/bubble/")!= string::npos) 
       ) {
    dominiotxt="GF_zgncdn_bubble";
    }
    if ( (url.find("familyville") != string::npos) 
       ) {
    dominiotxt="GF_zgncdn_ville";
    }
    if ( (url.find("hog") != string::npos) or (url.find("/hidden/")!= string::npos) 
       ) {
    dominiotxt="GF_zgncdn_hidden";
    }
    if ( (url.find(".zgncdn.com") != string::npos) and 
   ((url.find(".jpg")!= string::npos) or
   (url.find(".png")!= string::npos) or
   (url.find(".mp3")!= string::npos) or
   (url.find(".swf")!= string::npos))
        ) {
                
            r.file = get_filename(url);
                if (!r.file.empty()) {
                        r.match = true;
                       r.domain = dominiotxt;
                } else {
                        r.match = false;
                }
        } else {
                r.match = false;
        }
        return r;
}
