#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <time.h>
#include "../utils.h"
#include "../utils.cpp"
#include <sys/time.h>

using namespace std;

// use this line to compile
// g++ -I. -ldl -rdynamic -o teste teste.c
/* ./teste ./justin.tv.so "http://media6.justin.tv/archives/2011-2-18/live_user_peruanojapones_1297987490.flv"*/


int main(int argc, char *argv[])  {
	struct timeval v,v2;
	gettimeofday(&v,NULL);
	
	unsigned long t1 = v.tv_sec*1000000 + v.tv_usec;
	
    using std::cout;
    using std::cerr;

    cout << "C++ dlopen demo\n\n";

    // open the library
    cout << "Opening " << argv[1] << endl;
    void* handle = dlopen(argv[1], RTLD_LAZY);
    
    if (!handle) {
        cerr << "Cannot open library: " << dlerror() << endl;
        return 1;
    }
    
    // load the symbol
    cout << "Loading symbol ...\n";
    typedef resposta (*plugin_t)(string);
    plugin_t plugin = (plugin_t) dlsym(handle, "hgetmatch2");
    if (!plugin) {
        cerr << "Cannot load symbol 'plugin': " << dlerror() <<
            '\n';
        dlclose(handle);
        return 1;
    }
    
    resposta r;
    r = plugin(string(argv[2])); //("http://v5.lscache2.c.youtube.com/videoplayback?ip=0.0.0.0&sparams=id%2Cexpire%2Cip%2Cipbits%2Citag%2Cburst%2Cfactor&itag=35&ipbits=0&signature=5098DA812AE5487F7DBD1DD8632B36A9DC826B2F.CCA1F7F2445B14EFBFEDC7BAD186814DCCBAED48&sver=3&expire=1249088400&key=yt1&factor=1.25&burst=40&id=5776b34376ab2b15");
//    string nada = "NADA";
    //r = plugin(nada);
//    plugin(nada, r);
    cout << "retorno:" << endl;
    cout << "Match: " << r.match << endl;
    cout << "Domain: " << r.domain << endl;
    cout << "File: " << r.file << endl;
    cout << "range_min: " << r.range_min << endl;
    cout << "range_max: " << r.range_max << endl;
    cout << "total_file_size: " << r.total_file_size << endl;
//    cout << "nada " << nada << endl;
    // close the library
    cout << "Closing library...\n";
    dlclose(handle);
    gettimeofday(&v2,NULL);
	unsigned long t2 = v2.tv_sec*1000000 + v2.tv_usec;
	unsigned long long tmp2 = t2-t1;
	printf("Time: %.5lf msec.\n",(double)(tmp2/1000.0));
}

