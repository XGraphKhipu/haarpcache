#include <iostream>
#include <cstring>
#include <string>
#include <vector>
#include "../utils.cpp"

using namespace std;


string get_filename(string url, bool *video) {
	*video = false;
	vector<string> res;
	size_t pos;
	int in;
	string prefix;
	
	prefix = "";	
	pos = url.find(".mp4?");
	in = -1;
	if( pos != string::npos ){
		//~ puts("url is video!");
		*video = true;
		url[pos + 4] = '/';
		in = -2;
	}
	stringexplode(url, "/", &res);
	if(!*video)	 {
		if(regex_match("^[a-zA-Z][0-9]{3,6}x[0-9]{3,6}$",res.at(res.size() - 2)) != "") {
			prefix = res.at(res.size() - 2) + "-";
		}
	}
	return prefix + res.at(res.size() + in);
}

extern "C" resposta hgetmatch2(const string url) {
	//~ http://a8.sphotos.ak.fbcdn.net/hphotos-ak-snc7/402023_317476088301946_100001186710776_805403_2090542299_n.jpg
	//~ http://video.ak.fbcdn.net/cfs-ak-snc6/342529/41/10150331284539792_22876.mp4?oh=b78281f6f8d092074a5c87965d56f69e&oe=4F820720&__gda__=1333921568_fe066ad13d80ed665179b3297eb44a75
	//~ ^http.*(s?photos|video)\.ak\.fbcdn\.net\/.*(\.mp4\?.*|\_n\.jpg$)
    resposta r;
    bool video;
    
	r.range_min = 0;	
	r.range_max = 0;
	
	r.file = get_filename(url, &video);
	if (!r.file.empty()) {
		r.match = true;
		if(video)
			r.domain = "fbcdn.videos";
		else
			r.domain = "fbcdn.photos";
	} else {
		r.match = false;
	}

	return r;
}


