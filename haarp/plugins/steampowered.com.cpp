#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o steampowered.com.so steampowered.com.cpp  
// regex:
// http.{3,4}cdn\.(\w|\.)*cs\.steampowered\.com\/depot\/[0-9]+\/chunk\/\w{40}
//
string get_filename(string url) {
	vector<string> paths, direct;
	string file = "";
	
	stringexplode(url, "/", &paths);
	int psize = paths.size();
	short save = 0;
	for(int i=0;i<psize;i++) {
		if(save == 1) {
			file = paths.at(i);
			save = 0;
		}
		if(save == 2) {
			stringexplode(paths.at(i), "?", &direct);
			file = file + "_" + direct.at(0);
			return file;
		}
		if(paths.at(i) == "depot")
			save = 1;
		if(paths.at(i) == "chunk")
			save = 2;
		
	}
	return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;

	r.file = get_filename(url);
	r.domain = "steampowered";
	r.match = true;
	return r;
	
}

