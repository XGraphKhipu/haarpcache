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
	string header_browser;
    public:
		bool flag_change_top_range_max;
		bool flag_truncate_top_range_max;
		long long int flag_truncate_old_range_max;

		lintervalPositionByteDisk lranges, lrangeswork;
		lintervalPositionByteDisk::iterator brange;
		long long int range_min, range_max;//ADD
		long long int acumulate;
		long long int bwrite;
		long long int limit;
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
		bool haveUpdateDB;
		bool isgeneralplugin;
		bool exists_transaction_editing_file; //This process blocked the edition the of file?
		int64_t contentLengthServer;
		int64_t acumulateBodyLength;
		lusercache lusers_db;
		string ip_browser;
		bool browserclosed;
		bool headerReader;
		double prob;
		time_t expires;

		void UpdateFileSizeinPartial( string header );
		void getLimitBytes(string &header);
		bool isForCache(string header);
		long long int getOnlyContentLength( string header );
		void Cache2( long long int cl );
		string getHeaderFromServer(int *errcode);
	void GetProbExpiresFromHeader(string header, double *proba, time_t *texpires);
        string request, domain,msghit;
        string readFileHeader;
	string headerServer;
		int port;
		string origin_header;
        resposta r;
        int64_t size_orig_file,filedownloaded,filesended,expiration,filesizeneto,oldfilesended;
        bool hit,downloading,rewrited,resuming,general,etag;        
        int Cache();
        int SetDomainAndPort( string domainT, int portT, string requestT="" );
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
        
	void initializeVariables(bool db);

        void saveClientIP(string ip);
			
        void Update();
        void SubUpdate(int64_t partialBytesSended);
        bool liberate_edition();
        int lockFile(int singleDomain);
        void Close();
        void CloseUpdate();
        
        void print_list_lranges(lintervalPositionByteDisk lRangesPositionDisk, string prefix);
        void print_range(intervalPositionByteDisk range, string prefix);
};

#endif

