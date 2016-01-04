#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o plugin.so plugin.cpp
// ^http.{3,40}\.com\.edgesuite\.net\/(.*\/){2,}.*\.(mpq|MPQ)$
string get_videoid(string url){
	string separate = "";
	string filename = "";

	vector<string> stripurl;
	stringexplode(url, "/", &stripurl);

	int size = stripurl.size();

	for (int i = 2; i < size; i++) {
		filename += separate + stripurl.at(i);
		separate = "_";
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
		if (url.find("worldofwarcraft.com.edgesuite.net") != string::npos)
			r.domain = "worldofwarcraft";
		else if (url.find("blizzard.com.edgesuite.net") != string::npos)
			r.domain = "blizzard";
		else 
			r.domain = "edgesuite";
	}
	return r;
}

