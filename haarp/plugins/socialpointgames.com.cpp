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

using namespace std;

// regex
// http.*\.socialpointgames\.com.*(\.jpg|\.png|\.swf|\.mp3)
// use this line to compile
// g++ -I. -fPIC -shared -g -o socialpointgames.com.so socialpointgames.com.cpp

string dominiotxt="socialpg";
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
 
extern "C" resposta getmatch(const string url) {
    resposta r; 

    if ( (url.find("/dragoncity/") != string::npos) 
       ) {
    dominiotxt="socialpg/dragoncity";
    }
    if ( (url.find("/socialwars/") != string::npos) 
       ) {
    dominiotxt="socialpg/socialwars";
    }
    if ( (url.find("/socialempires/") != string::npos) 
       ) {
    dominiotxt="socialpg/socialempires";
    }
    if ( (url.find(".socialpointgames.com/") != string::npos) and 
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
