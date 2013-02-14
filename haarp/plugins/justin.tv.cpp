#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o justin.tv.so justin.tv.cpp
void get_videoid(string url, string &file, int *a, int *b){
	vector<string> resultado, url2;
	SearchReplace(url,"?","&");
	string lastpart = "";
	stringexplode(url, "/", &resultado);
	lastpart = resultado.at(resultado.size()-1);
	stringexplode(lastpart, "&", &url2);
	file = url2.at(0);

	for(int i=1;i<url2.size();i++) {
		vector<string> var;
		stringexplode(url2.at(i),"=", &var);
		if(var.size() != 2)
			continue;
		if( var.at(0) == "start" ) {
			if (regex_match("^[0-9]+$", var.at(1)) != "" ) {
				*a = atoi(var.at(1).c_str()) - 0;
				*b = -1;
				break;
			}
			else {
				file = "";
				return;
			}
		}
	}
}
// o regex retorna a parte do texto encontrada na linha
//regex_match(regex,texto);
//~ http://media12.justin.tv/archives/2012-11-9/highlight_338812701.flv?start=18051421
extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	get_videoid(url, r.file, &r.range_min, &r.range_max);
	if ( !r.file.empty() ) {
		r.match = true;
		r.domain = "justin";
	} else {
		r.match = false;
	}
	return r;
}

