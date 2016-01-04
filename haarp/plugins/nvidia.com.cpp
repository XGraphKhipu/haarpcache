#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

string get_videoid(string url){
	vector<string> stripurl;
	stringexplode(url, "/", &stripurl);

	return stripurl.at(stripurl.size() - 1);
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;
	r.range_min = 0;
	r.range_max = 0;
	
	if (url.find("?") != string::npos) {
		r.match = false;
	} else {
		r.file = get_videoid(url);
		r.match = true;
		r.domain = "nvidia";
	}
	return r;
}

