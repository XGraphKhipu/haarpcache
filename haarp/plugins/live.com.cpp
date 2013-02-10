/* 
 * (c) Copyright 2009 Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br>. Some Rights Reserved. 
 * @autor Rodrigo Medeiros (rodrigomanga) <rodrigomanga@yahoo.com.br> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o live.com.so live.com.cpp  

string rewriteurl(string url) {
	string meu_partner = "partner-pub-9370461587396013:l0rtjl-aokf";
	string q ="";
	string urlt=url;
	vector<string> resultado,valor;

	if ( (urlt.find("/results.aspx?") != string::npos ||urlt.find("/search?") != string::npos) ){
		SearchReplace(urlt,"?","&");
		stringexplode(urlt, "&", &resultado);
		for (int i=0; i <= resultado.size()-1;i++){
			valor.clear(); 
			stringexplode(resultado.at(i), "=", &valor);
			if (valor.at(0) == "q") {
				q = valor.at(1);
				break;
			}
		}
		if (!q.empty()){
			url = "www.google.com.pe/search?q="+q+"&cx="+meu_partner;
			return url;
		} else {
			return url;
		}
	}
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	
	r.file = rewriteurl(url);
	if (!r.file.empty()) {
		r.match = true;
		r.domain = "rewrite";
	} else {
		r.match = false;
	}
	return r;
}
