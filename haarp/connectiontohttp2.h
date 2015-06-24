#ifndef CONNECTIONTOHTTP2_H
#define CONNECTIONTOHTTP2_H

#include "connectiontohttp.h"
#include "utils.h"
#include "database_mysql.h"
#include "fstream"

using namespace std;

class ConnectionToHTTP2 : public ConnectionToHTTP {
    private:
        string pluginsdir, cachedir, subdir, completefilepath, completepath;
		int cache_limit;
        fstream cachefile;
        fstream outfile;
        double timerecord,timerecord2,timeout;
        ConnectionToHTTP downloader;
        Database domaindb;
        bool passouheader,closed;
        bool was_liberate;
    public:
		llista *lranges, *brange, *lrangeswork;
		int range_min, range_max;//ADD
		int acumulate;
		int bwrite;
		int limit;
		int np;
		int fileHeaderLengthUnread; 
		int count_wait;
		bool ext_webm; 
		bool hasupdate;
		bool unlimit;
		bool partial;//html5
		//~ bool knowhitmiss;
		bool bchrome;
		bool miss2hit;
		bool exists_transaction_editing_file; //This process blocked the edition the of file?
		int64_t contentLengthServer;
		int64_t acumulateBodyLength;
		
		void UpdateFileSizeinPartial( string header );
		void getLimitBytes(string &header);
		int getOnlyContentLength( string header );
		void Cache2( int cl );
		
        string domain,request,msghit;
        string readFileHeader;
		int port;
	string origin_header;
        resposta r;
        int64_t size_orig_file,filedownloaded,filesended,expiration,filesizeneto;
        bool hit,downloading,rewrited,resuming,general,etag;        
        void Cache();
        bool SetDomainAndPort( string domainT, int portT, string requestT="" );
        bool ConnectToServer();
        bool SendHeader( string header, bool ConnectionClose, string requestT="" );
        string GetIP();
        bool ReadHeader(string &headerT);
        bool ReadHeaderFromServer(string &headerT);
        bool AnalyseHeader( string &linesT );
        bool IsItKeepAlive();
        int64_t GetContentLength();
        bool IsItChunked();
        string PrepareHeaderForBrowser();
        int GetResponse();
        bool CheckForData( int timeout );
        ssize_t ReadBodyPart( string &bodyT, bool Chunked );
        void pintarlista( llista *p, string c );
        void Update();
        void SubUpdate();
        bool liberate_edition();
        int lockFile();
        void Close();
};

#endif

