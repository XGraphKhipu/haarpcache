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
// g++ -I. -fPIC -shared -g -o mlstatic.com.so mlstatic.com.cpp
// regex
// http.*\.mlstatic\.com.*(\.jpg|\.png)
 
 
string get_filename(string url) {
                vector<string> resultado;
                stringexplode(url, "/", &resultado);
                return resultado.at(resultado.size()-1);           
}
 
extern "C" resposta getmatch(const string url) {
    resposta r;   
 
if ( (url.find(".mlstatic.com") != string::npos) and ((url.find(".jpg") != string::npos) or (url.find(".png") != string::npos))
   ) {
      
       r.file = get_filename(url);
      if (!r.file.empty()) {
         r.match = true;         
         r.domain = "mercadolibre_img";
      } else {
         r.match = false;
      }
   } else {
      r.match = false;
   }
   return r;
}
