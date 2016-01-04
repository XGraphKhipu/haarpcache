#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp

string get_videoid(string url){
	string separate = "";
	string filename = "";
	bool save = false;

	vector<string> stripurl;
	stringexplode(url, "/", &stripurl);

	int size = stripurl.size();

	for (int i = 0; i < size; i++) {
		if (!save && regex_match("([0-9]+\\.){2,5}[0-9]+$", stripurl.at(i)) != "") 
			save = true;	
		if (save) {
			filename += separate + stripurl.at(i);
			separate = "_";
		}
	}
	return filename;
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
		r.domain = "lol";
	}
	return r;
}

