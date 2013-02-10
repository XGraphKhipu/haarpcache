#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o ytimg.com.so ytimg.com.cpp

string get_videoid(string url){
	vector<string> resultado;
	string retorna = "";
	
	int hq = 0;
	if(url.find("hqdefault.jpg") != string::npos)
		hq = 1;
	stringexplode(url, "/", &resultado);
	retorna  = resultado.at(resultado.size()-2);
	if(hq)
		retorna += "-hq";
	else // alone hqdefault 
	{
		string resullast = resultado.at(resultado.size() - 1);
		string str = "";
		if( ( str = regex_match("M[0-9]*.jpg\\?sigh=[\\_a-zA-Z0-9\\-]*$", resullast) ) != "" ) {
			retorna = str.substr(str.find("sigh=") + 5) + "-" + str.substr(1,str.find(".jpg") - 1);
		}
		else
			return "";
	}
	return retorna + ".jpg";
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
    r.range_min = 0;
	r.range_max = 0;
	r.file = get_videoid(url);
	if ( !r.file.empty() ) {
		r.match = true;
		r.domain = "ytimg";
	} else {
		r.match = false;
	}
    return r;
}


