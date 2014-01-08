#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

string get_videoid(string url){
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
// o regex retorna a parte do texto encontrada na linha
//regex_match(regex,texto);

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	r.file = get_videoid(url) + ".mp4";

	if ( !r.file.empty() ) {
		r.match = true;
		r.domain = "wrzuta";
	} else {
		r.match = false;
	}
	return r;
}

