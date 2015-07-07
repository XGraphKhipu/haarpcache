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
// g++ -I. -fPIC -shared -g -o imageshack.us.so imageshack.us.cpp
// regex 
// http.*\.imageshack\.us.*(\.jpg|\.png)
// http.*imageshack\.us.*(\.jpg|\.png)

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

  if ( (url.find(".imageshack.us/") != string::npos) 
     ) {
      r.file = get_filename(url);
      if (!r.file.empty()) {
         r.match = true;
         r.domain = "imageshack";       
      } else {
         r.match = false;
      }
   }
   
   else if ( (url.find("imageshack.us/") != string::npos) and ((url.find(".jpg") != string::npos) or (url.find(".png") != string::npos))  
   ) {      
       r.file = get_filename(url);
      if (!r.file.empty()) {
         r.match = true;
         r.domain = "imageshack-small";
      } else {
         r.match = false;
      }
   } 
   else 
   {
      r.match = false;
   }
   return r;
}
