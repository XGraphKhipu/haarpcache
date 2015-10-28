#include <iostream>
#include <cstring>
#include <vector>
#include "../md5.cpp"
#include "../utils.cpp"

#define p(a) cout<<a
#define pn(a) cout<<a<<endl
// use this line to compile
// g++ -I. -fPIC -shared -g -o porntube.com.so porntube.com.cpp  

string get_filename(string url, string &domain) {
	SearchReplaceAny(url, "http://" , "");
	SearchReplaceAny(url, "https://", "");
	string domain_ = getdomain(url);
		
	domain = domain_;
	vector<string> split;

	stringexplode(domain_, ".", &split);

	/*if(split.size() == 3)
		domain = split.at(1) + "." + split.at(2);
	else */
	if (split.size() == 2)
		domain = split.at(0);
	
	split.clear();
	size_t find = url.find("/");
	if( find != string::npos ) {
		stringexplode(url, "/", &split);
		return md5( domain_ + url.substr(find, -1) ) + DELIM + split.at(split.size() - 1);
	}
	else
		return "";
}

extern "C" resposta hgetmatch2(const string url) {
	resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	string domain; 
	if ( (url.find("?") == string::npos) && (regex_match("\\.\\w{2,4}$",url) != "") ) {
		r.file = get_filename(url, domain);
		if( !r.file.empty() ) {
			r.domain = domain;
			r.match = true;
		} else 
			r.match = false;
	}
	else 
		r.match = false;
	return r;
	
}

