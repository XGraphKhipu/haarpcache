#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

string get_videoid(string url){
	vector<string> resultado;
	string retorna = "";
	stringexplode(url, "/", &resultado);
	retorna = resultado.at(resultado.size()-1);
	return retorna;
}
// o regex retorna a parte do texto encontrada na linha
//regex_match(regex,texto);
//http://vod01.netdna.com/vod/vodch01.globbtelevisions/Sorteo_MundoHackerTV.mp4
extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	
	r.file = get_videoid(url);
	if (	(!r.file.empty()) and
		(url.find(".netdna.com/vod/") != string::npos) and
		(url.find(".mp4") != string::npos) and
		(url.find("?") == string::npos)
	) {
		r.match = true;
		r.domain = "netdna";
	} else {
		r.match = false;
	}
	return r;
}

