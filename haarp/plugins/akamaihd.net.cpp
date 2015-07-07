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
// g++ -I. -fPIC -shared -g -o akamaihd.net.so akamaihd.net.cpp
// regex
// http.*\.akamaihd\.net.*(\.jpg|\.png|\.swf|\.mp3)
    
string dominiotxt="GAMESF_akamaihd";
string get_filename(string url) {
        vector<string> resultado;
        if (url.find("?") != string::npos) {
            stringexplode(url, "?", &resultado);
            stringexplode(resultado.at(resultado.size()-2), "/", &resultado);
            return resultado.at(resultado.size()-4) + "_" + resultado.at(resultado.size()-3) + "_" + resultado.at(resultado.size()-2) + "_" +resultado.at(resultado.size()-1);           
        } else {
            stringexplode(url, "/", &resultado);
            return resultado.at(resultado.size()-4) + "_" + resultado.at(resultado.size()-3) + "_" + resultado.at(resultado.size()-2) + "_" +resultado.at(resultado.size()-1); ;
        }
}
 
extern "C" resposta hgetmatch2(const string url) {
    resposta r; 
	r.range_min = r.range_max = 0;
   if ( (url.find("/shockwave/") != string::npos)
       ) {
    dominiotxt="GF_akamaihd_city2";
    }
    if ((url.find("/farm2/")!= string::npos)
       ) {
    dominiotxt="GF_akamaihd_farm2";
    }
    if ( (url.find("/slingo/")!= string::npos) 
       ) {
    dominiotxt="GF_akamaihd_slingo";
    }
   if ( (url.find("dchoc1")!= string::npos) 
        ) {
    dominiotxt="GF_akamaihd/galaxy";
    }
    if ( (url.find("/cooking/") != string::npos)
       ) {
    dominiotxt="GF_akamaihd_cheff";
    }
    if ( (url.find(".akamaihd.net") != string::npos) and 
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
