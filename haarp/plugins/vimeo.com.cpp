/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */
 
 
#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

// use this line to compile
// g++ -I. -fPIC -shared -g -o vimeo.com.so vimeo.com.cpp  
// copy vimeo.com.so to bitgravity.com.so
//~ http://av.vimeo.com/97429/176/96652680.mp4?aksessionid=e8f148be6071ac7af73f9e05cd52de8c&token=1336336394_f9b8110a996092d9180ef9b604df6d58
//~ http://av.vimeo.com/66396/406/95877980.mp4?aksessionid=3b6887a8e1f9815ab319fa95e9234251&token=1336336481_962af5a1244510c8a039f45056b15149
//~ http://av.vimeo.com/17006/420/87032681.mp4?aktimeoffset=274.17&aksessionid=67f6a706ff68cb7d536d4780a6a4b547&token=1360340090_df3e2e41cd75dedd44cdf2b34b09e894
//~ http://pdl.vimeocdn.com/18839/916/216734641.mp4?token2=1389465784_7d86f62d3a036e2ca2957653fb1b54af&aksessionid=03eae128c014a885
//~ ^http.{1,4}(\w|\.|-)*\.vimeo(cdn)?.com\/.*\.mp4(\?.*)?$
string get_filename(string url, int *ra, int *rb) {
	vector<string> resultado;
	stringexplode(url,"/",&resultado);
	url = resultado.at(resultado.size() - 1);
	if (url.find("?") != string::npos) {
		resultado.clear();
		stringexplode(url, "?", &resultado);
		return resultado.at(0);           
	} else {
		return url;
	}
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;	
	r.range_min = 0;
	r.range_max = 0;
	
	if ( regex_match("[&\?]\\w*(time|start)", url) == ""  ) {
	    r.file = get_filename(url, &r.range_min, &r.range_max);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "vimeo";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}

