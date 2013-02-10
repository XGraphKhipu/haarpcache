#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0;

	vector<string> resultado;
	stringexplode(url,"/",&resultado);
	r.file = resultado.at(resultado.size() - 1);
	r.domain = "orkut";
    r.match = true;
	return r;
        
}


