/* 
 * (c) Copyright 2009 Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com>. Some Rights Reserved. 
 * @autor Joaquim Pedro França Simão (osmano807) <osmano807@gmail.com> 
 */

#include <iostream>
#include <cstring>
#include <vector>
#include "../utils.cpp"

using namespace std;

// use this line to compile
// g++ -I. -fPIC -shared -g -o microsoft.com.so microsoft.com.cpp
// copy microsoft.com.so to windowsupdate.com.so

bool in_array(const string &needle, const vector< string > &haystack) {
    int max = haystack.size();

    if (max == 0) return false;
		
	for (int iii = 0; iii < max; iii++) {
       	if (regex_match(haystack[iii], needle) != "") {
           	return true;
		}
	}
    return false;
}

string get_filename(string url) {
		vector<string> resultado;
		/*if (url.find("?") != string::npos){
			stringexplode(url, "?", &resultado);
			url = resultado.at(resultado.size()-2);
		}*/
	
		/*if (regex_match("[0-9]{10}$", url) != "") {
			// metadados
			stringexplode(url, "/", &resultado);
			return resultado.at(resultado.size()-3); 
		} else {*/
			stringexplode(url, "/", &resultado);
			return resultado.at(resultado.size()-1);
		//}                 

            
}

extern "C" resposta hgetmatch2(const string url) {
    resposta r;
	r.range_min = 0;
	r.range_max = 0; 
	
	vector<string> black_list; // tudo para não usar array? o que tem de mais em um array...
	black_list.push_back ("WuSetupHandler.cab");
	black_list.push_back ("muv3muredir.cab");
	black_list.push_back ("muredir.cab");
	black_list.push_back ("wuredir.cab");
	black_list.push_back ("muauth.cab");
	black_list.push_back ("musetup.cab");
	black_list.push_back ("wsus3setup.cab");
	black_list.push_back ("authrootstl.cab");
	black_list.push_back ("authrootseq.txt");
	black_list.push_back ("default.aspx");
	black_list.push_back ("wuident.cab");
	black_list.push_back ("\\.asmx");
	black_list.push_back ("stats\\.");
	black_list.push_back ("\\.asp");
    black_list.push_back ("cdm.cab");

	if (	/*( 
			(url.find("windowsupdate.com") != string::npos) or 
			(url.find("update.microsoft.com") != string::npos) or
			(url.find("download.microsoft.com") != string::npos) or 
			(url.find("dlservice.microsoft.com")!= string::npos) 
			) and
			(
			 (url.find(".cab") != string::npos) or 
			 (url.find(".exe") != string::npos) or 
			 (url.find(".iso") != string::npos) or
			 (url.find(".zip") != string::npos) or 
			 (url.find(".psf") != string::npos) or 
			 (url.find(".txt") != string::npos) or
		     (url.find(".crt") != string::npos)	
		    ) and*/
			(in_array(url, black_list) == false) and 
			url.find("?") == string::npos 
	) {

	    r.file = get_filename(url);
		if (!r.file.empty()) {
			r.match = true;
			r.domain = "windowsupdate";
		} else {
			r.match = false;
		}
	} else {
		r.match = false;
	}
	return r;
}
