#include "connectiontohttp2.h"
#include "logfile.h"
#include "utils.h"
#include "params.h"
#include "database_mysql.h"
#include "x64compat.h"

#include <stdio.h> 
#include <iostream> 
#include <vector> 
#include <dlfcn.h>
#include <cstdlib> 
#include <sstream>
#include <cmath>


extern int LL; //LogLevel

void ConnectionToHTTP2::print_list_lranges(lintervalPositionByteDisk lRangesPositionDisk, string prefix) {
	if(lRangesPositionDisk.empty()) {
		LogFile::ErrorMessage("[DEBUG print_list_ranges %s] Range Empty!\n", prefix.c_str());
		return;
	}
	lintervalPositionByteDisk::iterator it = lRangesPositionDisk.begin();
	while(it != lRangesPositionDisk.end()) {
		LogFile::ErrorMessage("[DEBUG print_list_ranges %s] pos: %lli<%lli,%lli>\n", prefix.c_str(), it->position, it->a, it->b);
		++it;
	}
}

void ConnectionToHTTP2::print_range(intervalPositionByteDisk range, string prefix) {
	LogFile::ErrorMessage("[DEBUG print_list_ranges %s] pos: %lli<%lli,%lli>\n", prefix.c_str(), range.position, range.a, range.b);
}	

void ConnectionToHTTP2::initializeVariables(bool db) {
	lusers_db.clear();
	acumulate = bwrite = oldfilesended = filesizeneto = size_orig_file = filedownloaded = filesended = expiration = 0;
	exists_transaction_editing_file = true;
	headerServer = "";
	headerReader = false;
	flag_change_top_range_max   = false;
	flag_truncate_top_range_max = false;
	flag_truncate_old_range_max = -2;
	browserclosed = db;
}
void ConnectionToHTTP2::saveClientIP(string ip) {
	vector<string> lip;
	stringexplode(ip, " ", &lip);
	ip_browser = lip.at(lip.size() - 1);
}

void ConnectionToHTTP2::getLimitBytes(string &header) {
	if (LL > 1) LogFile::ErrorMessage("******************** NEW CONNECTION ********************\n");
	header_browser = header;
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Header Browser: \"%s\"\n", header_browser.c_str());
	
	range_min = 0;
	range_max = 0;
	np = 0;
	partial = false;
	vector<string> lines, value, rangeb;
	origin_header = "";
	stringexplode(header, "\r\n", &lines);
	for (unsigned int i = 0; i <= lines.size() - 1; ++i) {
		if (!i && lines.at(0).find(" 206 ") != string::npos && lines.at(0).find("HTTP") != string::npos) {
			if (LL > 1) LogFile::AccessMessage("Header with status code 206 found\n");
			partial = true;
		}

		if (lines.at(i).find("Range: bytes=") == 0) {
			stringexplode(lines.at(i), "=", &value);
			if (value.size() <= 1)
				continue;
			partial = true;
			stringexplode(value.at(1), "-", &rangeb);
			if (!rangeb.size())
				continue;
			if (rangeb.size() == 1) {
				range_min = atoll(rangeb.at(0).c_str());
				range_max = -1;
			} else {
				range_min = atoll(rangeb.at(0).c_str());
				range_max = atoll(rangeb.at(1).c_str());
			}
			//break;
		}
		if (lines.at(i).find("Origin:") == 0) {
			origin_header = lines.at(i).substr(8);
		}
	}
}
bool ConnectionToHTTP2::isForCache(string header) {
	vector<string> lines;
	stringexplode(header, "\r\n", &lines);
	for (int i = 0; i <= (int) lines.size() - 1; i++) 
		if (lines.at(i).find("HTTP") == 0 && lines.at(i).find("200") == string::npos && lines.at(i).find("206") == string::npos)
			return false;
	return true;
}

long long int ConnectionToHTTP2::getOnlyContentLength(string header) {
	if (!isForCache(header))
		return -1;	
	string scl = getValuesHeader(header, "Content-Length:", V_INT);
	if (scl.empty())
		return -1;
	return atoll(scl.c_str());
}
/* 
 * size_orig_file es el tamaño del archivo en disco.
 * size es el tamaño del archivo real, en el servidor (0: desconocido).
 */
void ConnectionToHTTP2::Cache2(long long int cl) {
	if (LL>2) LogFile::ErrorMessage("Cache2()-INIT-contentLenght: %lli| rmin:%lli - rmax:%lli = %lli\n", cl, range_min, range_max, range_max - range_min + 1);
	if ( (!size_orig_file && range_max <= 0) || flag_change_top_range_max || flag_truncate_top_range_max ) {
		if (flag_truncate_top_range_max) {
			range_max = flag_truncate_old_range_max;
			lrangeswork.clear();
		}
		if (flag_change_top_range_max) {
			range_max = cl - 1;
			lrangeswork.clear();
		}
		size_orig_file = cl + range_min;
		if (general)
			domaindb.set("UPDATE haarp SET size=" + lldtoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
		else
			domaindb.set("UPDATE haarp SET size=" + lldtoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
	}
	if (range_max <= 0) {
		range_max = cl + range_min - 1;
		lrangeswork.clear();
	} else {
		if (range_max != cl + range_min - 1) {
			if (LL>0) LogFile::ErrorMessage("Warning: redimention of the Range Max (%lli to %lli) in the file %s[%lli-%lli]\n", range_max, cl + range_min - 1, r.file.c_str(), range_min, range_max);
			range_max = cl + range_min - 1;
			lrangeswork.clear();
		}
	}
	filesizeneto = cl;
	if (lrangeswork.empty()) {
		if (LL>2) print_list_lranges(lranges, "Cache2()-lranges");
		if (LL>2) LogFile::ErrorMessage("Before-lrangeswork.empty|Cache2()-%s[%lli-%lli]\n", r.file.c_str(), range_min, range_max);
		lrangeswork = getRangeWork(lranges, range_min, range_max, &hit); // update HIT?.
		if (LL>2) print_list_lranges(lrangeswork, "Cache2()-lrangeswork");
		if (LL>2) LogFile::ErrorMessage("After-lrangeswork.empty|Cache2()-%s[%lli-%lli]\n", r.file.c_str(), range_min, range_max);
		if (hit) {
			msghit = "HIT";
			//~ if(!exists_transaction_editing_file) liberate_edition();
			domaindb.set("UPDATE haarp SET last_request=now() WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
		}
	}
}
/*
 * En cabeceras partial, se obtiene lo siguiente: "Content-Range: bytes a-b/c", donde c expresa la longitud total del archivo de verdad.
 * Aqui se pretende actualizar el valor de size en la bd con el valor del tamaño del archivo (c).
 */
void ConnectionToHTTP2::UpdateFileSizeinPartial(string header) {
	vector<string> lines, value;
	stringexplode(header, "\r\n", &lines);
	for (unsigned int i = 1; i <= lines.size() - 1; ++i) {
		if (lines.at(i).find("Content-Range: bytes") == 0) {
			stringexplode(lines.at(i), "/", &value);
			if (value.size() <= 1)
				continue;
			size_orig_file = atol(value.at(1).c_str());
			domaindb.set("UPDATE haarp SET size=" + lldtoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
			return;
		}
	}
}
void ConnectionToHTTP2::Update() {
	string rang_, part_;
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Update() ---- ..(db.id=%li).. ---- \n", domaindb.getID());
	if (r.match && !hit) {
		if (acumulate && bwrite) {
			if (cachefile.is_open()) {
				cachefile.flush();
			}
			brange->b = brange->a + acumulate - 1;
			if (!appendNode(lranges, *brange))
				if (LL > 1) LogFile::ErrorMessage("Error!, block range is NULL and acumulate not is empty!\n");

		}
		if (!hasupdate) {
			list2string(lranges, rang_, part_);
			long long int f_n = getFileSize(lranges);
			stringstream stmp0, stmp1;
			stmp0 << f_n;
			stmp1 << np;
			addUserCache(lusers_db, ip_browser, time(NULL), (long long int)(filesended-oldfilesended), 0);
			string strUserDB = 	lusercache2str(lusers_db);
			if (LL > 2) LogFile::ErrorMessage("[DEBUG-Update] list of users to DB (MISS): '%s'\n", strUserDB.c_str());
			if (LL > 1) LogFile::AccessMessage("Update data base with ranges=%s, partition=%s, domain=%s and file=%s\n", rang_.c_str(), part_.c_str(), r.domain.c_str(), r.file.c_str());
			if (domaindb.isconnect() && (domaindb.set("UPDATE haarp set modified=now(), rg='" + rang_ + "', pos='" + part_ + "', filesize='" + stmp0.str() + "', np=np+" + stmp1.str() + ", users='" + strUserDB + "' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") < 0))
				if (LL > 1) LogFile::ErrorMessage("Error updating the database or is not connected\n");
			hasupdate = true;
			if (!exists_transaction_editing_file) liberate_edition();
		}
		acumulate = 0;
	} else if (hit) {
		if (!hasupdate && filesended) {
			stringstream bsended;
			bsended << filesended;
			
			addUserCache(lusers_db, ip_browser, time(NULL), (long long int)filesended, 1);
			string strUserDB = 	lusercache2str(lusers_db);
			
			if (LL > 2) LogFile::ErrorMessage("[DEBUG-Update] list of users to DB (HIT): '%s'\n", strUserDB.c_str());
			if (LL > 2) LogFile::ErrorMessage("[DEBUG-Update] Update the database with new bytes_requested: file=%s[%lli] - HIT\n", r.file.c_str(), filesended);

			if (domaindb.isconnect() && (domaindb.set("UPDATE haarp SET last_request=now(), bytes_requested=bytes_requested+" + bsended.str() + ", users='" + strUserDB + "' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") < 0))
				if (LL > 1) LogFile::ErrorMessage("Error updating the database or is not connected\n");
			hasupdate = true;
		}
	}
	if (r.match)
		if (!exists_transaction_editing_file) liberate_edition();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Update() ---- ..for clear list of information of parts.. ---- \n");
	lranges.clear();
	lrangeswork.clear();
}
void ConnectionToHTTP2::SubUpdate(int64_t partialBytesSended) {
	string rang_, part_;
	list2string(lranges, rang_, part_);
	long long int f_n = getFileSize(lranges);
	stringstream stmp0, stmp1;
	stmp0 << f_n;
	stmp1 << np;
	addUserCache(lusers_db, ip_browser, time(NULL), (long long int)partialBytesSended, 0);
	string strUserDB = 	lusercache2str(lusers_db);
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-Update] list of users to DB (MISS): '%s'", strUserDB.c_str());
	if (LL > 1) LogFile::AccessMessage("Saving part in the database with: ranges='%s', partition='%s', domain='%s' and file='%s'\n", rang_.c_str(), part_.c_str(), r.domain.c_str(), r.file.c_str());
	if (domaindb.set("UPDATE haarp SET modified=now(), rg='" + rang_ + "', pos='" + part_ + "', filesize='" + stmp0.str() + "', np=np+" + stmp1.str() + ", users='" + strUserDB + "' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") < 0)
		if (LL > 1) LogFile::ErrorMessage("Error, in sub-updating the data base: '%s' \n", domaindb.getError().c_str());
}

/* 
 * return '-1' for error of mysql, '1' for OK, and '0' for not OK.
 * */
int ConnectionToHTTP2::lockFile(int singleDomain) {
	vector<string> lname;

	if (singleDomain) splitstring(r.file, DELIM, &lname);

	if (singleDomain && lname.size() == 1) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] Problem with the file '%s', format not correct!\n", r.file.c_str());
		return 0;
	} else if (singleDomain) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] file '%s', parts: '%s', '%s'!\n", r.file.c_str(), lname.at(0).c_str(), lname.at(1).c_str());
	}
	if (domaindb.set("START TRANSACTION;") != 0) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 1\n");
		
		return -1;
	}

	if (singleDomain == 0) {
		string mysql_query = "SELECT file,abs(TIMESTAMPDIFF(SECOND,now(),GREATEST(modified,last_request)))<=30 as difftime, file_used FROM haarp WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "' FOR UPDATE;";
		//if (domaindb.get("SELECT * FROM haarp WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "' FOR UPDATE;") != 0) {
		if (domaindb.get(mysql_query) != 0) {
			domaindb.set("ROLLBACK;");
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 2\n");
			return -1;
		}
	} else {
		//string query_mysql = "SELECT * FROM haarp WHERE file like '" + domaindb.sqlconv(lname.at(0)) + DELIM"%"DELIM + domaindb.sqlconv(lname.at(1)) + "' and domain='" + r.domain + "' FOR UPDATE;";
		string query_mysql = "SELECT file,abs(TIMESTAMPDIFF(SECOND,now(),GREATEST(modified,last_request)))<=30 as difftime, file_used FROM haarp WHERE file like '" + domaindb.sqlconv(lname.at(0)) + DELIM"%"DELIM + domaindb.sqlconv(lname.at(1)) + "' and domain='" + r.domain + "' FOR UPDATE;";
		if (domaindb.get(query_mysql) != 0) {
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Error, mysql query: '%s'!!\n", query_mysql.c_str());
			domaindb.set("ROLLBACK;");
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 3\n");
			return -1;
		}
		if (LL > 2) LogFile::ErrorMessage("[DEBUG-lockfile] mysql query: '%s'!!\n", query_mysql.c_str());
	}
	if (!domaindb.get_num_rows()) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] NOT EXISTS ON DB =========================.===========\n", r.file.c_str(), range_min, range_max);
		if (singleDomain == 0) {
			addUserCache(lusers_db, ip_browser, time(NULL), 0, 0);
			if (domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request, file_used, users) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, now(),now(),0,now(), 1,'" + lusercache2str(lusers_db) + "');") != 0) {
				domaindb.set("ROLLBACK;");
				if (LL > 0) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] Error on INSERT! (NOT EXISTS FILE ON DB) ========.=======\n", r.file.c_str(), range_min, range_max);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 4\n");
				return -1;
			}
		}
		domaindb.set("COMMIT;");
		return 1;
	} else {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] EXISTS ON DB =============================.===========\n", r.file.c_str(), range_min, range_max);
		if (singleDomain) {
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Change r.file for netflix; old name: '%s'\n", r.file.c_str());
			r.file = domaindb.get("file", 1);
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Change r.file for netflix; new name: '%s'\n", r.file.c_str());
		}
		/*string mysql_query = "SELECT abs(TIMESTAMPDIFF(SECOND,now(),GREATEST(modified,last_request)))<=30 as difftime, file_used FROM haarp WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';";
		if (domaindb.get(mysql_query) != 0) {
			domaindb.set("ROLLBACK;");
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 5\n");
			return -1;
		}*/
		string difftime = domaindb.get("difftime", 1);
		string file_used = domaindb.get("file_used", 1);
		if (!atoi(file_used.c_str())) {
			if (domaindb.set("UPDATE haarp set modified=now(), file_used=1 WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) {
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] status is ..... BLOCKED or ERROR MYSQL!.\n", r.file.c_str(), range_min, range_max);
				domaindb.set("ROLLBACK;");
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 6\n");
				return -1;
			}
			if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] BLOCKING from mysql.\n", r.file.c_str(), range_min, range_max);
			domaindb.set("COMMIT;");
			return 1;
		}
		//if (LL > 2) LogFile::ErrorMessage("[DEBUG - lockfile()] difftime: '%s', toint: '%i'\n", difftime.c_str(), atoi(difftime.c_str()));
		if (difftime != "") {
			if (atoi(difftime.c_str())) { // After of 60 seconds without modifications, is secure that the file not is working as cache.
				if (LL > 1) LogFile::ErrorMessage("Info: file '%s' with persistent changes\n", r.file.c_str());
				if (LL > 1) LogFile::AccessMessage("The file on disk is working as cache, go direct to internet.\n");
				domaindb.set("COMMIT;");
				return 0;
			} else {
				if (domaindb.set("UPDATE haarp set modified=now(), file_used=1 WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) {
					domaindb.set("ROLLBACK;");
					if (LL > 2) LogFile::ErrorMessage("[DEBUG] Aqui 7\n");
					return -1;
				}
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] BLOCKING STRICT from mysql.\n", r.file.c_str(), range_min, range_max);
				domaindb.set("COMMIT;");
				return 1;
			}
		} else {
			if (LL > 1) LogFile::ErrorMessage("Warning: not exist the file '%s' in the DB?, please check your column 'modified'.\n", r.file.c_str());
			domaindb.set("ROLLBACK;");
			return -1;
		}
	}
	domaindb.set("COMMIT;");
	return 0;
}

bool ConnectionToHTTP2::liberate_edition() {
	if (was_liberate || exists_transaction_editing_file) 
		return true;		
	bool re = domaindb.set("UPDATE haarp set file_used=0 WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';");
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s [%lli-%lli] LIBERATE edition from mysql.\n", r.file.c_str(), range_min, range_max);
	if (!re)
		was_liberate = true;
	return re;
}

void ConnectionToHTTP2::GetProbExpiresFromHeader(string header, double *proba, time_t *texpires) {
	string lm  	= getValuesHeader(header, "Last-Modified:", 	V_DATE);
	string date  	= getValuesHeader(header, "Date:", 		V_DATE);
	string expires 	= getValuesHeader(header, "Expires:", 		V_DATE);
	string maxage 	= getValuesHeader(header, "max-age=", 		V_INT);
	time_t tdate, tmaxage = 0, tlm;
	if ( !date.empty() )
		tdate = dateformat2epoch(date);
	else
		tdate = tnow();
	if ( !expires.empty() )
		*texpires = dateformat2epoch(expires);
	else
		*texpires = 0;
	if ( !maxage.empty() ) {
		tmaxage = atoi(maxage.c_str());
		if ( !tmaxage )
			*texpires = tdate + 3600*24*3;
		else
			*texpires = tdate + tmaxage;
	} else if ( !*texpires || *texpires < tdate )
		*texpires = tdate + 3600*24*3;
	if ( !lm.empty() )
		tlm = dateformat2epoch(lm);
	else
		tlm = tdate;
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Date: %li, Last-Modified: %li, *texpires: %li, tmaxage: %li\n", tdate, tlm, *texpires, tmaxage);
	
	*proba = 1/(fabs((tdate - tlm)/(*texpires - tdate + 0.0)) + 1.0);
}
string ConnectionToHTTP2::getHeaderFromServer(int *errcode) {
	*errcode = 0;
	string header;
	bool s1 = ConnectionToHTTP::SetDomainAndPort(domain, port);
	if ( s1 ) {
		if(LL>2) LogFile::ErrorMessage("IN getHeaderFromServer, paso #1! -\n");
		s1 = ConnectionToHTTP::ConnectToServer();
		if ( s1 ) {
			if (LL>2) LogFile::ErrorMessage("IN getHeaderFromServer, paso #2! -\n");
			s1 = ConnectionToHTTP::SendHeader(header_browser, browserclosed);
			if ( s1 ) {
				if (LL>2) LogFile::ErrorMessage("IN getHeaderFromServer, paso #3! -\n");
				s1 = ConnectionToHTTP::ReadHeader(header);
				if ( s1 ) {
					if (LL>2) LogFile::ErrorMessage("IN getHeaderFromServer, paso #4 - OK!! -\n");
					headerReader = true;
					return header;
				}
				else
					*errcode = -4;
			} else
				*errcode = -3;
		} else 
			*errcode = -2;
		
	} else 
		*errcode = -1;

	return "";	
}

int ConnectionToHTTP2::Cache() {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por Cache (1.2)\n");
	msghit = "MISS";
	hasupdate = false;
	unlimit = false;
	miss2hit = false;
	haveUpdateDB = false;
	count_wait = 0;
	exists_transaction_editing_file = true;
	was_liberate = false;
	hit = downloading = r.match = rewrited = resuming = passouheader = general = etag = closed = false;
	limit = 0;
	np = 0;

	isgeneralplugin = false;

	lrangeswork.clear();
	lranges.clear();
	
	fileHeaderLengthUnread = LENGTHHEADERFILE;
	contentLengthServer = 0;
	acumulateBodyLength = 0;
	readFileHeader = "";

	srand(time(0));

	struct timeval v1,v2;
	gettimeofday(&v1,NULL);
	if (LL > 0) LogFile::AccessMessage("Url %s%s\n", domain.c_str(), request.c_str());
	if (LL > 2) LogFile::ErrorMessage("[DEBUG_Cache] Url %s%s\n", domain.c_str(), request.c_str());
	string domaintmp = getdomain(domain + request); //youtube.com
	string pluginpath = pluginsdir + domaintmp + ".so";
	if (file_exists(pluginpath)) {
		if (LL > 0) LogFile::AccessMessage("Loading plugin %s\n", pluginpath.c_str());
		if (LL > 2) LogFile::ErrorMessage("[DEBUG_Cache] Loading plugin %s\n", pluginpath.c_str());
		void *handle = dlopen(pluginpath.c_str(), RTLD_LAZY);
		if (!handle) {
			if (LL > 0) LogFile::ErrorMessage("Cannot open library: %s\n", dlerror());
		}
		typedef resposta( *plugin_t)(string);
		plugin_t plugin = (plugin_t) dlsym(handle, "hgetmatch2");
		if (!plugin) {
			if (LL > 0) LogFile::ErrorMessage("Cannot load symbol: %s\n", dlerror());
			dlclose(handle);
		} else { // Plugin cargado correctamente, ok para atribuir valores a struc
			r = plugin(domain + request);
			if (!partial || (partial && !range_max)) {
				range_min = r.range_min;
				range_max = r.range_max;
				if (partial)
					if (LL > 2) LogFile::ErrorMessage("Partial file: '%s' [%lli-%lli]\n", (r.file).c_str(), range_min, range_max);
			} else { //is partial and range_max != 0.
				if (r.domain == "youtube") {
					if (!r.exist_range) {
						if ( r.file != "" ) r.file = "206-" + r.file;
						int postmp;
						if ((postmp = (int)(r.file).rfind(".flv")) != (int) string::npos) {
							(r.file).replace(postmp, 4, ".webm");
						}
					} else {
						r.match = false;
					}
				} else {
					if ( r.file != "" )
						r.file = "206-" + r.file;
				}

			}
			if (r.domain != "youtube") 
				r.total_file_size = 0;

			if (LL > 1) LogFile::AccessMessage("Ranges received: range_min=%lli, range_max=%lli\n", range_min, range_max);
			if (LL > 0) LogFile::AccessMessage("Resposta Match %d Domain %s File %s\n", r.match, r.domain.c_str(), r.file.c_str());
			if (LL > 2) LogFile::ErrorMessage("[DEBUG_Cache] Ranges received: range_min=%lli, range_max=%lli\n", range_min, range_max);
			if (LL > 2) LogFile::ErrorMessage("[DEBUG_Cache] Resposta Match %d Domain %s File %s\n", r.match, r.domain.c_str(), r.file.c_str());
		}
	}
	else {
		pluginpath = pluginsdir + "generalplugin.so";
		if (file_exists(pluginpath)) {
			if (LL > 0) LogFile::AccessMessage("Loading plugin %s\n", pluginpath.c_str());
			void *handle = dlopen(pluginpath.c_str(), RTLD_LAZY);
			if (!handle) {
				if (LL > 0) LogFile::ErrorMessage("Cannot open library: %s\n", dlerror());
			}
			typedef resposta( *plugin_t)(string);
			plugin_t plugin = (plugin_t) dlsym(handle, "hgetmatch2");
			if (!plugin) {
				if (LL > 0) LogFile::ErrorMessage("Cannot load symbol: %s\n", dlerror());
				dlclose(handle);
			} else { // Plugin cargado correctamente, ok para atribuir valores a struc
				r = plugin(domain + request);
				if (partial) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG] Partial file with the generic plugin: '%s' [%lli-%lli]\n", (r.file).c_str(), range_min, range_max);
					//r.file = "206-" + r.file;
				} else {
					range_min = r.range_min;
					range_max = r.range_max;
					isgeneralplugin = true;
				}
				r.total_file_size = 0;
				if (LL > 2) LogFile::ErrorMessage("Info: '%s' [%lli-%lli]\n", (r.file).c_str(), range_min, range_max);
			}
		}		
	}
	gettimeofday(&v2,NULL);
	if (LL > 0) LogFile::ErrorMessage("[DEBUG] Time of loading of plugin: %.4lf msec\n", timevaldiff(&v1,&v2));
	if (r.match) { // Es la url de un dominio
		if (LL > 2) LogFile::ErrorMessage("[DEBUG] match! file: '%s', domain: '%s' [%lli-%lli]\n", (r.file).c_str(), r.domain.c_str(), range_min, range_max);
		int lfile;
		if( !isgeneralplugin ) {
			gettimeofday(&v1,NULL);
			if (r.domain == "netflix")
				lfile = lockFile(1);
			else
				lfile = lockFile(0);
			if (lfile <= 0) {
				if (lfile == -1) if (LL > 0) LogFile::ErrorMessage("Error mysql: '%s'!\n", domaindb.getError().c_str());
				exists_transaction_editing_file = true;
				r.match = hit = false;
				return 0;
			}
			gettimeofday(&v2,NULL);
			if (LL > 0) LogFile::ErrorMessage("[DEBUG] Time of lock db!: %.4lf msec\n", timevaldiff(&v1,&v2));
		} else 
			was_liberate = true;
		
		exists_transaction_editing_file = false;

		subdir = ConvertChar(r.file);

		vector<string> list_dir;
		string directory, directorytmp, completefilepathtmp, completepathtmp;
		stringexplode(cachedir, "|", &list_dir);

		bool disco_con_espacio = false;
		bool file_in_cache = false;

		for (int i = 0; i < (int) list_dir.size(); i++) {
			directory = trimstr(list_dir.at(i));

			completepath = directory + r.domain;
			if (r.domain == "netflix") {
				vector<string> lfilename;
				string fnewname;
				splitstring(r.file, DELIM, &lfilename);
				if (lfilename.size() < 3) {
					if (LL > 1) LogFile::ErrorMessage("Error with the Netflix domain: the file name ('%s') format unknow\n", r.file.c_str());
					r.match = hit = false;
					return 0;
				}
				fnewname = lfilename.at(0) + DELIM + lfilename.at(1);
				subdir = ConvertChar(fnewname);
				completefilepath = completepath + "/" + subdir + "/" + fnewname;
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-Cache] Netflix domain: the file path to cache, is '%s'\n", completefilepath.c_str());
			} else {
				completefilepath = completepath + "/" + subdir + "/" + r.file;
			}
			if (file_exists(completefilepath)) {
				file_in_cache = true;
				break;
			}
			if (!disco_con_espacio && disk_use(completepath) <= cache_limit) {
				disco_con_espacio = true;
				directorytmp = directory;
				completefilepathtmp = completefilepath;
				completepathtmp = completepath;
			}
		}
		if (!file_in_cache && disco_con_espacio) {
			completefilepath = completefilepathtmp;
			completepath = completepathtmp;
			directory = directorytmp;
		}
		if (LL > 0) LogFile::AccessMessage("Cache limit (%.0lf/%d) %s\n", disk_use(completepath), cache_limit, directory.c_str());
		if (LL > 0) LogFile::AccessMessage("File: %s\n", string(completefilepath).c_str());
		if (range_max > 0)
			filesizeneto = range_max - range_min + 1;
		
		if ( isgeneralplugin ) {
			gettimeofday(&v1,NULL);
			// comprobate if the file is being used
			if ( domaindb.get("SELECT prob, expires, modified, file_used FROM haarp WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") ) {
				LogFile::ErrorMessage("Error, can't select prob, expires an others entries of the mysql: '%s'\n", domaindb.getError().c_str());
				r.match = hit = false;
				return 0;
			}
			/*string dt = domaindb.get("difftime", 1);
			if ( dt != "" && dt.find("1") != string::npos ) {
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-Cache] MISS: the file '%s' is being used/modified!\n", r.file.c_str());
				r.match = hit = false;
				return 0;
			}*/
			if ( !domaindb.get_num_rows() ) {
				if ( header_browser.find("Content-Length:") != string::npos ) {
					r.match = hit = false;
					return 0;
				}
				int ecode;
				
				headerServer = getHeaderFromServer(&ecode);
				if (LL>2) LogFile::ErrorMessage("[DEBUG] IN HEADER READER PREV: \"%s\"\n", headerServer.c_str());
				if ( ecode < 0 || headerServer.empty() || headerServer.find(" 200 OK") == string::npos ) {
					r.match = hit = false;
					return ecode;
				}
				GetProbExpiresFromHeader(headerServer, &prob, &expires);
				if (LL>2) LogFile::ErrorMessage("[DEBUG] Not exist register in the DB, calculated prob and expires from HEADER: (%.5lf,%li)! -\n", prob, expires);
				addUserCache(lusers_db, ip_browser, time(NULL), 0, 0);
				string msg_pet = "INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request, file_used, users, prob, expires) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, now(),now(),0,now(), 0,'" + lusercache2str(lusers_db) + "', " + lftoa(prob) + ", '" + time2DateStr(expires)  + "' );";
				if ( domaindb.set(msg_pet) != 0 ) {
					LogFile::ErrorMessage("Error!, \"%s\", file =  \"%s\", domain = \"%s\"\n", domaindb.getError().c_str(), domaindb.sqlconv(r.file).c_str(), r.domain.c_str());
					r.match = hit = false;
					return 0;
				}
			} else {
				time_t tmodified;
				string pro  = domaindb.get("prob", 1);
				string exp  = domaindb.get("expires", 1);
				string modi = domaindb.get("modified", 1);
				if ( !pro.empty() )
					prob = atof(pro.c_str());
				else {
					if (LL > 0) LogFile::ErrorMessage("WARNING, there is no probabilily!\n");
					prob = 1;
				}
				if ( !exp.empty() )
					expires = dateStr2Time(exp);
				else {
					if (LL > 0) LogFile::ErrorMessage("WARNING, there is no expiration date!\n");
					//expires = dateStr2Time("2100-10-10 10:10:10");
					expires = 0;
				}
				if ( !modi.empty() )
					tmodified = dateStr2Time(modi);
				else
					tmodified = 0;
				if (LL>2) LogFile::ErrorMessage("[DEBUG] IN isgeneralplugin! - prob: %.4lf, expires: %li IN DB! -\n", prob, expires);
				if ( expires < tnow() ) {
					if (LL>2) LogFile::ErrorMessage("IN isgeneralplugin! - Expires(%li) < tnow() (%li)! -\n", expires, tnow());
					double drand = rand()/(RAND_MAX + 0.0);
					if ( drand < prob ) {
						if (LL > 2) LogFile::ErrorMessage("[DEBUG] rand (%.4lf) < prob (%.4lf)!\n", drand, prob);
						int ecode;
						headerServer = getHeaderFromServer(&ecode);
						if (LL>2) LogFile::ErrorMessage("[DEBUG] IN HEADER READER: \"%s\"\n", headerServer.c_str());
						if ( ecode < 0 || headerServer.empty() || headerServer.find(" 200 OK") == string::npos ) {
							r.match = hit = false;
							return ecode;
						}
						time_t tlastmod = 0;
						string llm  = getValuesHeader(headerServer, "Last-Modified:", V_DATE);
						if (!llm.empty()) 
							tlastmod = dateStr2Time(llm);
						else
							tlastmod = tnow();
						GetProbExpiresFromHeader(headerServer, &prob, &expires);
						if (LL>2) LogFile::ErrorMessage("IN isgeneralplugin! - get prob and expires from HEADER: (%.5lf,%li)! -\n", prob, expires);
						if (tlastmod > tmodified) {
							if (LL>2) LogFile::ErrorMessage("[DEBUG] FILE MODIFIED FROM SERVER - get prob and expires from HEADER: (%.5lf,%li)! -\n", prob, expires);
							if (domaindb.set("UPDATE haarp SET prob=" + lftoa(prob) + ", expires='" + time2DateStr(expires)  +"', rg='', pos='', size=0, filesize=0, modified=now() WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) {
								LogFile::ErrorMessage("Error, '%s'\n", domaindb.getError().c_str());
								r.match = hit = false;
								return 0;
							}
						} else {
							if (LL>2) LogFile::ErrorMessage("[DEBUG] FILE <NOT> MODIFIED FROM SERVER - get prob and expires from HEADER: (%.5lf,%li)! -\n", prob, expires);
							if (domaindb.set("UPDATE haarp SET prob=" + lftoa(prob) + ", expires='" + time2DateStr(expires)  +"'  WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) {
								LogFile::ErrorMessage("Error, '%s'\n", domaindb.getError().c_str());
								r.match = hit = false;
								return 0;
							}
						}
					} else 
						if (LL > 2) LogFile::ErrorMessage("[DEBUG] rand (%.4lf) >= prob (%.4lf)!, is misss or hit? ....\n", drand, prob);
				} else 
					if (LL > 2) LogFile::ErrorMessage("[DEBUG] the file '%s' expired (%s)!\n", r.file.c_str(), exp.c_str());
			}
			gettimeofday(&v2,NULL);
			if (LL > 0) LogFile::ErrorMessage("[DEBUG] Time of create file in  db - general plugin!: %.4lf msec\n", timevaldiff(&v1,&v2));
		}
		gettimeofday(&v1,NULL);
			
		if (r.domain == "rewrite") {
			r.match = false;
			rewrited = true;
			if (LL > 1) LogFile::AccessMessage("Rewrite: %s \n", r.file.c_str());
		} else if (!file_in_cache) { /* the file not are in disk - MISS!!?? */
			if (LL > 0) LogFile::AccessMessage("The file NOT is ON disk\n");
			/* MISS */
			lranges.clear();
			if (range_max > 0) {
				lrangeswork = getRangeWork(lranges, range_min, range_max, &hit);
				if (LL>2) print_list_lranges(lrangeswork, "Cache()-lrangeswork");
			}
			if (disco_con_espacio) {
				if (domaindb.operation != "INSERT") {
					if (domaindb.set("UPDATE haarp SET modified=now(), last_request=now(), rg='', pos='', np=0, deleted=0, static=0, size=" + llitoa(r.total_file_size) + " WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) {
						LogFile::ErrorMessage("Error, it can not update the record of mysql: '%s'\n", domaindb.getError().c_str());
						r.match = hit = false;
						return 0;
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG] File %s BLOCKED edition from mysql -inserting new row-.\n", r.file.c_str());
				}
				if (!file_exists(completepath + "/" + subdir + "/")) {
					mkdir_p(completepath + "/" + subdir + "/");
				}
				if (LL > 0) LogFile::AccessMessage("MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
			} else 
				hit = r.match = false;

		} else { /* The file are in disk */
			if (LL > 1) LogFile::AccessMessage("The file is ON disk!\n");
			if (domaindb.get("SELECT size, rg, pos, users FROM haarp WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';") != 0) {
				LogFile::ErrorMessage("Error select mysql: %s\n", domaindb.getError().c_str());
				r.match = hit = false;
				/* Free use of file from mysql */
				if (!exists_transaction_editing_file) liberate_edition();
				return 0;
			}
			if (domaindb.get_num_rows() == 0) { // No existe en la db, pero sí en disco.
				if ( isgeneralplugin ) {
					if (LL > 2) LogFile::AccessMessage("[DEBUG] generalplugin is run!, The file not exists IN the DB!\n");
					addUserCache(lusers_db, ip_browser, time(NULL), 0, 0);
					if (domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request, file_used, users) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, now(),now(),0,now(), 0,'" + lusercache2str(lusers_db) + "');") != 0) {
						if (LL > 0) LogFile::ErrorMessage("Error mysql!: '%s'\n", domaindb.getError().c_str());
						r.match = hit = false;
						return 0;
					}
				} else {
					LogFile::ErrorMessage("Error, the row of the file '%s' not exist on the db!", r.file.c_str());
					r.match = hit = false;
					if (!exists_transaction_editing_file) liberate_edition();
					return 0;
				}
			} else { // Existe el archivo en disco y en la base de datos, miss o hit?.
				/* 'size' es el tamaño del archivo, si estubiera entero (como en el mismo servidor de video)*/
				size_orig_file = atol(domaindb.get("size", 1).c_str());
				
				if (r.total_file_size > 0 && size_orig_file && size_orig_file != r.total_file_size) {
					if (LL > 0) LogFile::ErrorMessage("Warning: file corrupted? ... file size db != clen (%li != %lli)\n", size_orig_file, r.total_file_size);
					r.match = hit = false;
					if (!exists_transaction_editing_file) liberate_edition();
					return 0;
				}
				string str_users_db = domaindb.get("users", 1);
				
				if ( !str2lusercache(str_users_db, lusers_db) ) 
					if (LL > 0) LogFile::ErrorMessage("Warning: incorrect format of users in db ('%s')\n", str_users_db.c_str());
				
				
				if(!size_orig_file && r.total_file_size) {
					if (domaindb.set("UPDATE haarp SET size=" + llitoa(r.total_file_size) + " WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") != 0) 
						LogFile::ErrorMessage("Error, it can not update the size of file in mysql: '%s'\n", domaindb.getError().c_str());
					size_orig_file = r.total_file_size;
				}
				string ranges = domaindb.get("rg", 1);
				string position = domaindb.get("pos", 1);
				if (LL > 0) LogFile::AccessMessage("In DB: ranges ('%s') and position ('%s')\n", ranges.c_str(), position.c_str());
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] In DB: ranges ('%s') and position ('%s')\n", ranges.c_str(), position.c_str());

				if (!generateList(ranges, position, lranges)) { //raro que entre
					if (LL > 0) LogFile::ErrorMessage("Format incorrect: Ranges=%s, Parts=%s\n", ranges.c_str(), position.c_str());
					if (LL > 1) LogFile::AccessMessage("Format incorrect!: Reset ranges and positions in the DB\n");
					domaindb.set("UPDATE haarp set rg='', pos='' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';");
					lranges.clear();
				}
				if (size_orig_file) {
					if (range_max <= 0) {
						flag_change_top_range_max = true;
						range_max = size_orig_file - 1; // update range_max
					}
					if (size_orig_file > range_min + 1 && size_orig_file < range_max + 1) {
						flag_truncate_top_range_max = true;
						flag_truncate_old_range_max = range_max;
						range_max = size_orig_file - 1; // truncate
						if (LL > 1) LogFile::AccessMessage("Maximum range, trucate to: %i\n", range_max);
					}
					filesizeneto = range_max - range_min + 1;
					lrangeswork = getRangeWork(lranges, range_min, range_max, &hit); /* hit, can not change it later */
					if (LL>2) print_list_lranges(lrangeswork, "Cache()/size_orig_file-lrangeswork");
					if (LL>2) {
						if (hit) LogFile::ErrorMessage("Cache()/size_orig_file - HIT -\n");
						else  LogFile::ErrorMessage("Cache()/size_orig_file - MISS -\n");
					}
				} else {
					if (range_max > 0 && (getExtremeb(lranges) < range_min || range_max <= getExtremeb(lranges))) {
						if (LL>2) print_list_lranges(lranges, "Cache()/!size_orig_file-lranges");
						lrangeswork = getRangeWork(lranges, range_min, range_max, &hit); /* hit, can not change it later */
						if (LL>2) print_list_lranges(lrangeswork, "Cache()/!size_orig_file-lrangeswork");
					} else { // range_max <= 0 || range_min <= exteme < range_max
						hit = false; /* can be changed later (function Cache2) */
					}
				}

				if (hit) {
					if (headerReader) {
						if (LL > 2) LogFile::ErrorMessage("[DEBUG] HIT, Close connect for previus header reader in general plugin!\n");
						ConnectionToHTTP::Close();
					}
					msghit = "HIT";
					//~ if(!exists_transaction_editing_file) liberate_edition();
					domaindb.set("UPDATE haarp SET last_request=now() WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
					if (LL > 0) LogFile::AccessMessage("HIT: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					if (LL > 2) LogFile::ErrorMessage("HIT: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					
				}
			}
			domaindb.clear();
		}
		gettimeofday(&v2,NULL);
		if (LL > 0) LogFile::ErrorMessage("[DEBUG] Time of determinate MISS/HITS - Any plugins!: %.4lf msec\n", timevaldiff(&v1,&v2));
	} else if (!rewrited) { // sistema de cache de arquivos ESTÁTICOS GENERAL
		if (Params::GetConfigInt(getFileExtension(getFileName(request)) + "_EXP") > 0 && request.find_first_not_of("()[]?&=;,´`'\"") == string::npos) { // se está configurada essa extensão
			//~ if (LL > 0) LogFile::AccessMessage("(2)**** Entró !rewrited!!\n");
			r.file = getFileName(request);
			r.domain = domaintmp;
			subdir = request.substr(1, request.length() - r.file.length() - 2);
			vector<string> list_dir;
			stringexplode(cachedir, "|", &list_dir);
			cachedir = trimstr(list_dir.at(0));
			completepath = cachedir + "general/" + r.domain.at(0) + "/" + ConvertChar(r.domain) + "/" + r.domain;
			completefilepath = completepath + "/" + subdir + "/" + r.file;
			if (string(subdir + "/" + r.file).length() <= 767) {
				r.match = general = true;
				//if (LL>1) LogFile::AccessMessage("Arquivo: %s\n",completefilepath.c_str());
				if (LL > 1) LogFile::AccessMessage("file: %s subdir: %s completepath: %s\n", r.file.c_str(), subdir.c_str(), completepath.c_str());
				if (!file_exists(completefilepath)) {
					if (disk_use(completepath) <= cache_limit) {
						// cria diretorios e etc
						if (!file_exists(completepath + "/" + subdir + "/")) {
							mkdir_p(completepath + "/" + subdir + "/");
						}
						domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request, static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
						if (LL > 0) LogFile::AccessMessage("MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					} else {
						LogFile::ErrorMessage("Cache limit (%.0lf/%d) %s%s\n", disk_use(completepath), cache_limit, cachedir.c_str(), r.domain.c_str());
					}
				} else if (file_exists(completefilepath)) {
					if (domaindb.get("SELECT size,(unix_timestamp(now())-unix_timestamp(downloaded)) as expiration FROM haarp WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';") != 0) {
						LogFile::ErrorMessage("erro select mysql: %s\n", domaindb.getError().c_str());
						r.match = general = false;
					}
					size_orig_file = atol(domaindb.get("size", 1).c_str());
					expiration = atol(domaindb.get("expiration", 1).c_str());

					if (LL > 1) LogFile::AccessMessage("Size db: "LLD" File size: "LLD" Timeout: %d Expiration: %d\n", size_orig_file, file_size(completefilepath), (now() - file_getmodif(completefilepath)), expiration);

					if (domaindb.get_num_rows() == 0) // se nao existir o registro do arquivo do db
					{
						domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request,static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
						if (LL > 0) LogFile::AccessMessage("MISS DB: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					} else if (
						((now() - file_getmodif(completefilepath)) < 10) &&
						(
							(size_orig_file > file_size(completefilepath)) || (size_orig_file == 0)
						)
					) {
						hit = true;
						downloading = true;
						if (LL > 0) LogFile::AccessMessage("HIT DOWN: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
						msghit = "HIT";
					} else if (size_orig_file == file_size(completefilepath)) {
						if (expiration <= Params::GetConfigInt(getFileExtension(getFileName(request)) + "_EXP")) {
							hit = true;
							downloading = false;
							domaindb.set("UPDATE haarp SET last_request=now() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
							if (LL > 0) LogFile::AccessMessage("HIT!: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							if (LL > 0) LogFile::AccessMessage("EXPIRED: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							domaindb.set("UPDATE haarp SET downloaded=now() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
						}
					} else if (size_orig_file > 0) { // o arquivo foi baixado parcialmente
						if (disk_use(completepath) <= cache_limit) {
							hit = resuming = downloading = true;
							downloader.ClearVars();
							downloader.SetDomainAndPort(domain, port);
							if (LL > 0) LogFile::AccessMessage("HIT RESUME: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							LogFile::ErrorMessage("Cache limit (%.0lf/%d) %s%s\n", disk_use(completepath), cache_limit, cachedir.c_str(), r.domain.c_str());
						}
					}
					domaindb.clear();
				}
			}
		}
	}
	return 1;
}
int ConnectionToHTTP2::SetDomainAndPort(string domainT, int portT, string requestT) {
	if (LL > 2) LogFile::ErrorMessage("Pasando por SetDomainAndPort (1)\n");
	if (!domaindb.connected)
		if (domaindb.open(Params::GetConfigString("MYSQL_HOST"), Params::GetConfigString("MYSQL_USER"), Params::GetConfigString("MYSQL_PASS"), Params::GetConfigString("MYSQL_DB")) != 0) {
			LogFile::ErrorMessage("Error, connection mysql: %s\n", domaindb.getError().c_str());
			Close();
		}
	domain = domainT;
	port = portT;
	request = requestT;
	pluginsdir = Params::GetConfigString("PLUGINSDIR");
	cachedir = Params::GetConfigString("CACHEDIR");
	cache_limit = Params::GetConfigInt("CACHE_LIMIT");

	int ecode = Cache();
	
	if (ecode < 0)
		return ecode;

	if (hit)
		return true;
	else
		if (headerReader)
			return true;
		else
			return (int)ConnectionToHTTP::SetDomainAndPort(domain, port);
}
bool ConnectionToHTTP2::ConnectToServer() {
	//if (LL > 0) LogFile::AccessMessage("Pasando por ConnectToServer\n");
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por ConnectToServer (2)\n");
	if (rewrited) return true;
	if (resuming) {
		if (!downloader.ConnectToServer()) {
			if (LL > 0) LogFile::AccessMessage("downloader not conect\n");
			Close();
		}
	}
	if (hit) return true;
	else
		if (headerReader)
			return true;
		else
			return ConnectionToHTTP::ConnectToServer();
}
// header cabecera a enviar al servidor (la misma del browser!). no depende de hit u otras variables parecidas!
// ConnectionClose = "DropBrowser"
// requestT = ToBrowser.GetRequest()
//
bool ConnectionToHTTP2::SendHeader(string header, bool ConnectionClose, string requestT) {
	//if (LL > 0) LogFile::AccessMessage("Pasando por SendHeader (2)\n");
	if (passouheader) {
		if (cachefile.is_open()) cachefile.close();
		if (outfile.is_open()) outfile.close();
		if ((filesended < filesizeneto) && ((r.match && !hit) || (r.match && resuming)))
			file_setmodif(completefilepath, 1);
		downloader.Close();

		request = requestT;
		Cache();
	}
	if (!passouheader) passouheader = true;
	if (rewrited) return true;

	if (resuming) {
		vector<string> lines;
		char linha[5000];
		string headertmp = "";
		stringexplode(header, "\r\n", &lines);
		for (unsigned int i = 0; i <= lines.size() - 1; ++i) {
			if (i == lines.size() - 1) {
				sprintf(linha, "Range: bytes="LLD"-"LLD"\r\n", file_size(completefilepath), size_orig_file);
			} else {
				sprintf(linha, "%s\r\n", lines.at(i).c_str());
			}
			headertmp.append(linha);
		}
		downloader.SendHeader(headertmp, true);
	}
	if (header.find("If-None-Match") != string::npos) etag = true;

	if (hit) {
		return true;
	} else {
		if (headerReader)
			return true;
		else {
			int status;
			bool tmp = ConnectionToHTTP::SendHeader(header, ConnectionClose, &status); //la misma que envia el browser aqui
			return tmp;
		}
	}
}
string ConnectionToHTTP2::GetIP() {
	//if (LL > 0) LogFile::AccessMessage("Pasando por GetIP (3)\n");
	if (hit || rewrited) return "0.0.0.0";
	else
		return ConnectionToHTTP::GetIP();
}
bool ConnectionToHTTP2::ReadHeaderFromServer(string &headerT) {
	//if (LL > 0) LogFile::AccessMessage("Pasando por ReadHeaderFromServer (4)\n");
	bool result;
	if (headerReader) {
		headerT = headerServer;
		result = true;
	} else
		result = ConnectionToHTTP::ReadHeader(headerT);
	if (result) {
		if (r.domain == "youtube" && getFileExtension(r.file) == "FLV") {
			size_t pos;
			char line[90];
			//if (LL > 0) LogFile::AccessMessage("BEFORE-HEADER: %s\n", headerT.c_str());
			if ((pos = headerT.find("Content-Type:")) != string::npos && (headerT[pos - 1] == '\n' || headerT[pos - 1] == '\r')) {
				sscanf((headerT.substr(pos + 14)).c_str(), "%s", line);
				//headerT.replace(pos,strlen(line) + 15,"Content-Type: video/x-flv");
				headerT.replace(pos, strlen(line) + 15, "Content-Type: application/octet-stream");
			}
			//if (LL > 0) LogFile::AccessMessage("AFTER-HEADER: %s\n", headerT.c_str());
		}
	}	
	//if (LL > 2) LogFile::ErrorMessage("[DEBUG] AFTER-HEADER: \n%s\n", headerT.c_str());
	return result;
}
/* 
 * headerT: Edit the Header for to be sended to the browser.
 * */
bool ConnectionToHTTP2::ReadHeader(string &headerT) {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por ReadHeader (3.5)\n");
	bool result = false;
	if (!size_orig_file && r.match && !hit) //truncar, partial, !knowhitmiss
	{
		if (LL > 2) LogFile::ErrorMessage("[DEBUG - READHEADER] size_orig_file == 0\n");
		result = ConnectionToHTTP2::ReadHeaderFromServer(headerT);
		if (partial) {
			UpdateFileSizeinPartial(headerT);
			if (range_max <= 0)
				range_max = size_orig_file - 1;
		}
		long long int cl = getOnlyContentLength(headerT);
		if (LL > 2) LogFile::ErrorMessage("[DEBUG - READHEADER] header: \"%s\"\n*content_length:%lli\n", headerT.c_str(), cl);
		if (cl < 0) 
			return result;
		
		Cache2(cl); // generate lrangeswork!
	}
	if (resuming) { // no entra casi nunca!!!
		string headertmp = "";
		if (downloader.GetResponse() == 0) downloader.ReadHeader(headertmp);
		downloader.AnalyseHeader(headertmp);
		if ((downloader.GetResponse() == 302) || (downloader.GetResponse() == 303)) {
			headerT = headertmp;
			hit = resuming = downloading = false;
			rewrited = true;
			return true;
		} else if (downloader.GetResponse() != 206) {
			return false;
		}
	}

	if (rewrited) {
		stringstream tmp;
		tmp.str("");

		tmp << "HTTP/1.0 302 Moved\r\n";
		tmp << "Location: http://" << r.file << "\r\n";
		tmp << "Content-Length: 0\r\n";
		tmp << "Connection: close\r\n\r\n";

		headerT = tmp.str();
		return true;
	} else if (hit && etag) {
		stringstream tmp;
		tmp.str("");

		tmp << "HTTP/1.0 304 Not Modified\r\n";
		tmp << "Connection: close\r\n\r\n";

		headerT = tmp.str();
		rewrited = true;
		r.match = hit = false;
		if (!exists_transaction_editing_file) liberate_edition();
		return true;
	} else if (hit) {

		stringstream tmp;
		tmp.str("");
		if (!partial)
			tmp << "HTTP/1.0 200 OK\r\n";
		else
			tmp << "HTTP/1.0 206 Partial\r\n";
		
		string extension = getFileExtension(r.file);
		if (extension == "SWF")
			tmp << "Content-Type: application/x-shockwave-flash\r\n";
		else if (extension == "HTML")
            		tmp << "Content-Type: text/html\r\n";
	 	else if (extension == "HTM")
            		tmp << "Content-Type: text/html\r\n";
	 	else if (extension == "ASP")
            		tmp << "Content-Type: text/html\r\n";
	 	else if (extension == "PHP")
            		tmp << "Content-Type: text/html\r\n";
	 	else if (extension == "BMP")
            		tmp << "Content-Type: image/bmp\r\n";
		else if (extension == "IEF")
            		tmp << "Content-Type: image/ief\r\n"; 
		else if (extension == "JPEG")
            		tmp << "Content-Type: image/jpeg\r\n"; 
		else if (extension == "JPG")
            		tmp << "Content-Type: image/jpeg\r\n"; 
		else if (extension == "JPE")
            		tmp << "Content-Type: image/jpeg\r\n"; 
		else if (extension == "TIFF")
           		tmp << "Content-Type: image/tiff\r\n"; 
		else if (extension == "TIF")
            		tmp << "Content-Type: image/tiff\r\n"; 
		else if (extension == "WOFF")
            		tmp << "Content-Type: application/font-woff\r\n"; 
		else if (extension == "RAS")
            		tmp << "Content-Type: image/x-cmu-raster\r\n"; 
		else if (extension == "PNM")
            		tmp << "Content-Type: image/x-portable-anymap\r\n"; 
		else if (extension == "PBM")
            		tmp << "Content-Type: image/x-portable-bitmap\r\n"; 
		else if (extension == "PGM")
            		tmp << "Content-Type: image/x-portable-graymap\r\n"; 
		else if (extension == "PPM")
            		tmp << "Content-Type: image/x-portable-pixmap\r\n"; 
		else if (extension == "RGB")
            		tmp << "Content-Type: image/x-rgb\r\n"; 
		else if (extension == "JS")
            		tmp << "Content-Type: application/x-javascript\r\n"; 
		else if (extension == "CSS")
            		tmp << "Content-Type: text/css\r\n"; 
		else if (extension == "XBM")
            		tmp << "Content-Type: image/x-xbitmap\r\n"; 
		else if (extension == "XPM")
            		tmp << "Content-Type: image/x-xpixmap\r\n"; 
		else if (extension == "XWD")
            		tmp << "Content-Type: image/x-xwindowdump\r\n"; 
		else if (extension == "AU")
			tmp << "Content-Type: audio/basic\r\n"; 
		else if (extension == "XML")
            		tmp << "Content-Type: text/xml\r\n";  
		else if (extension == "XLS")
            		tmp << "Content-Type: text/xml\r\n";  
		else if (extension == "FLV")
			tmp << "Content-Type: application/octet-stream\r\n";
		else if (extension == "MP4")
			tmp << "Content-Type: video/mp4\r\n";
		else if (extension == "MP4A" && r.domain == "youtube")
			tmp << "Content-Type: audio/mp4\r\n";
		else if (extension == "WEBM")
			tmp << "Content-Type: video/webm\r\n";
		else
			tmp << "Content-Type: application/octet-stream\r\n";
		tmp << "Content-Length: ";
		tmp << filesizeneto;
		tmp << "\r\n";
		if (partial > 0) {
			tmp << "Accept-Ranges: bytes\r\n";
			tmp << "Content-Range: bytes " << range_min << "-" << range_max << "/" << size_orig_file << "\r\n";
		}
		tmp << "X-Cache: HIT from Haarp\r\n";
		tmp << "Haarp: HIT from " << r.domain << "\r\n";
		//tmp << "Access-Control-Allow-Credentials: false\r\n";
		if (r.domain == "youtube") {
			tmp << "Accept-Ranges:bytes\r\n";
			tmp << "Alternate-Protocol:80:quic\r\n";
		}
		if (origin_header != "") {
			tmp << "Access-Control-Allow-Credentials:true\r\n";
			tmp << "Access-Control-Allow-Origin:" << origin_header << "\r\n";
			tmp << "Timing-Allow-Origin:" << origin_header << "\r\n";
		}
		headerT = tmp.str();
		return true;

	} else {
		if (result)
			return true;
		bool re = ConnectionToHTTP2::ReadHeaderFromServer(headerT);
		long long int cl = getOnlyContentLength(headerT);
		if (!partial && cl >= 0 && cl != size_orig_file) {
			if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadHeader] Re-EDIT lrangesWork, size_orig_file:%lli, contentLength:%lli\n", size_orig_file, cl);
			Cache2(cl); // generate lrangeswork!
		}
		if (r.domain.find("NetflixID") != string::npos || r.domain.find("NetflixjsID") != string::npos) {
			string clength = regex_match("Content-Length: [0-9]+(\r|\n|\t)*", headerT);
			if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadHeader] clength = '%s'\n", clength.c_str());
			if (clength != "") {
				char c1[20], c2[30];
				sscanf(clength.c_str(), "%s %s", c1, c2);
				string sc1 = c1;
				string sc2 = c2;
				SearchReplace(headerT, sc1 + " " + sc2, sc1 + " " + itoa(atoi(c2) + 3000));
				sc1 = sc1 + " " + itoa(atoi(c2) + 3000);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadHeader] clength_new = '%s'\n", sc1.c_str());
			}
		}
		if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadHeader] Header_Server = \n\"%s\"\n", headerT.c_str());
		return re;
	}

}
bool ConnectionToHTTP2::AnalyseHeader(string &linesT) {
	//if (LL > 0) LogFile::AccessMessage("Pasando por AnalyseHeader (5)\n");
	if (hit) return true;
	else return ConnectionToHTTP::AnalyseHeader(linesT);
}
bool ConnectionToHTTP2::IsItKeepAlive() {
	//if (LL > 0) LogFile::AccessMessage("Pasando por IsItKeepAlive (6)\n");
	if (hit) return true;
	else return ConnectionToHTTP::IsItKeepAlive();
}
int64_t ConnectionToHTTP2::GetContentLength() {
	//if (LL > 0) LogFile::AccessMessage("Pasando por GetContentLength (7)\n");
	//if (LL > 2) LogFile::ErrorMessage("Pasando por GetContentLength (7)\n");
	int64_t tmp;
	if (hit)
		tmp = filesizeneto;
	else {
		int64_t ContentLengthReference;
		ContentLengthReference = ConnectionToHTTP::GetContentLength(); //del server..
		contentLengthServer = ContentLengthReference;
		if (LL > 2) LogFile::ErrorMessage("ContentLength: "LLD"\n", ContentLengthReference);
		if (r.match) {
			if ((
					ContentLengthReference < Params::GetConfigInt(getFileExtension(r.file) + "_MIN") ||
					(ContentLengthReference > Params::GetConfigInt(getFileExtension(r.file) + "_MAX") && Params::GetConfigInt(getFileExtension(r.file) + "_MAX") > 0)
				) || (
					ContentLengthReference < Params::GetConfigInt(UpperCase(r.domain) + "_MIN") ||
					(ContentLengthReference > Params::GetConfigInt(UpperCase(r.domain) + "_MAX") &&
						Params::GetConfigInt(UpperCase(r.domain) + "_MAX") > 0
					)
				)) {
				r.match = general = false;
				if (!exists_transaction_editing_file) liberate_edition();
				if (LL > 0) LogFile::AccessMessage("MAXMIN CANCEL: Domain: %s File: %s Size: "LLD"\n", r.domain.c_str(), r.file.c_str(), filesizeneto);
			}
		}
		tmp = ContentLengthReference;
	}
	return tmp;
}
bool ConnectionToHTTP2::IsItChunked() {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por IsItChunked (6)\n");
	bool tmp;
	if (hit) return false;
	else tmp = ConnectionToHTTP::IsItChunked();
	if(tmp)
		if (LL > 2) LogFile::AccessMessage("[DEBUG-IsItChunked] It Is CHUNKED (!HIT-%s) %%%%%%%%%%!\n", r.file.c_str());
	return tmp;
}
string ConnectionToHTTP2::PrepareHeaderForBrowser() {
	//if (LL > 0) LogFile::AccessMessage("Pasando por PrepareHeaderForBrowser (7)\n");
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por PrepareHeaderForBrowser (7)\n");
	if (hit) {
		string header;
		ReadHeader(header);
		return header;
	} else if (r.match) {
		timerecord = time(NULL);
		stringstream tmp;
		tmp.str("");
		tmp << ConnectionToHTTP::PrepareHeaderForBrowser();
		tmp << "Haarp: MISS from " << r.domain << "\r\n";
		return tmp.str();

	} else return ConnectionToHTTP::PrepareHeaderForBrowser();
}
int ConnectionToHTTP2::GetResponse() {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por GetResponse (8)\n");
	int retorno;
	if (hit) retorno = 200;
	else if (!hit && r.match) {
		retorno = ConnectionToHTTP::GetResponse();
		if (retorno != 200 && retorno != 206) {
			r.match = hit = resuming = false;
			if (!exists_transaction_editing_file) liberate_edition();
		}
	} else retorno = ConnectionToHTTP::GetResponse();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Pasando por GetResponse (8) - retorno = %i\n", retorno);
	return retorno;
}
bool ConnectionToHTTP2::CheckForData(int timeout) {
	if (LL > 2) LogFile::ErrorMessage("Pasando por CheckForData (9)\n");
	if (hit) return false;
	else return ConnectionToHTTP::CheckForData(timeout);
}

//filedownloaded - Cantidad de bytes descargados.
ssize_t ConnectionToHTTP2::ReadBodyPart(string &bodyT, bool Chunked) {
	if (rewrited) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] REWRITED!\n");
		bodyT.append("\r\n", 2);
		return 2;
	}
	ssize_t BodyLength = 0;

	string bodyTTemp = "";
	
	
	if (resuming) {
		if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] RESUMING!\n");
		string bodyTmp = "";
		BodyLength = downloader.ReadBodyPart(bodyTmp, Chunked);
		if (!outfile.is_open()) {
			outfile.open(string(completefilepath).c_str(), ios::out | ios::app | ios::binary);
		}
		if (BodyLength > 0)
			outfile.write(bodyTmp.c_str(), BodyLength);
		if ((time(NULL) - timerecord2) > 1) {
			outfile.flush();
			file_setmodif(completefilepath);
		}
	}

	if (hit || miss2hit) {
		BodyLength = 0;
		int tbuffer = MAXRECV;
		char memblock[MAXRECV];
		memset(memblock, '\0', sizeof(memblock));
		bodyT = "";
		while (BodyLength == 0) {
			if (!cachefile.is_open()) {
				if (LL > 2) print_list_lranges(lrangeswork, "ReadBodyPart/miss2hit||hit - lrangeswork");
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] Opening file: '%s'!\n", completefilepath.c_str());
				bwrite = 0;
				cachefile.open(string(completefilepath).c_str(), ios:: in | ios::binary);
				/**/
				brange = lrangeswork.begin();
				acumulate = 0;
				if (brange->position < 0) {
					BodyLength = -1;
					cachefile.close();
					r.match = hit = false;
					if (!exists_transaction_editing_file) liberate_edition();
					break;
				}
				longSeekpFile(cachefile, brange->position);
				limit = brange->b - brange->a + 1;
				/**/
				timeout = timerecord = time(NULL);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] Opened file: '%s'!\n", completefilepath.c_str());
			}
			if (acumulate + tbuffer > limit) // acumulate es siempre menor que limit!
				tbuffer = limit - acumulate;

			cachefile.tellg();
			cachefile.read(memblock, tbuffer);
			BodyLength = cachefile.gcount(); // Acomoda
			if (BodyLength > 0) {
				filesended += BodyLength;
				acumulate += BodyLength;
				bodyT.append(memblock, BodyLength);
				if (acumulate == limit) {
					++brange;
					acumulate = 0;
					if (brange != lrangeswork.end()) {
						longSeekpFile(cachefile, brange->position);
						limit = brange->b - brange->a + 1;
					}
				}
				timerecord = time(NULL);
			} else
				usleep(rand() % 300000);


			if ((time(NULL) - timerecord) >= 10) { // TIMEOUT
				/*Update?*/
				cachefile.close();
				return -1;
			}
			if (brange == lrangeswork.end() && filesended == filesizeneto) {
				cachefile.close();
				break;
			} else
			if (brange == lrangeswork.end() || filesended == filesizeneto) {
				BodyLength = -1;
				cachefile.close();
				break;
			}
			//~ if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] Opened file: '%s'!\n", completefilepath.c_str());
		}
	} else { //miss
		BodyLength = 0;
		ssize_t BodyLengthTmp = 0;
		BodyLength = ConnectionToHTTP::ReadBodyPart(bodyT, Chunked);
		BodyLengthTmp = BodyLength;
		// HACEMOS UN TRACK DEL VIDEO
		if (r.domain.find("youtube_IDs") != string::npos) {
			bodyTTemp = bodyT;
			SearchReplace(bodyTTemp, "videoplayback%3F", "TEMPORAL_HAARP");
			SearchReplace(bodyTTemp, "TEMPORAL_HAARP", "videoplayback%3Fwatchid%3D" + r.file + "%26");
			string range_trash = regex_match("%2Frange%2F[0-9]+-[0-9]+%3F", bodyTTemp);
			if(range_trash != "") {
				SearchReplace(bodyTTemp, range_trash, "TEMPORAL_HAARP");
				SearchReplace(bodyTTemp, "TEMPORAL_HAARP", range_trash + "watchid%3D" + r.file + "%26");
			}

			bodyT = bodyTTemp;
		} else if (r.domain.find("NetflixID") != string::npos) {
			bodyTTemp = bodyT;
			string strobject = "wwwplayer-2.0000.267.011.js\"";
			string strreplace = "wwwplayer-2.0000.267.011.js?watchid=" + r.file + "\"";
			int n = SearchReplace(bodyTTemp, strobject, strreplace);
			bodyT = bodyTTemp;
			BodyLength += n * (strreplace.size() - strobject.size());
			acumulateBodyLength += BodyLength;
			int64_t diffLength = contentLengthServer - acumulateBodyLength;
			if (LL > 2) LogFile::ErrorMessage("[DEBUG]BodyLength: '%i', acuBodyLength: "LLD", contentLengthServer: "LLD"\n", BodyLength, acumulateBodyLength, contentLengthServer);
			if (diffLength <= 3000) {
				//~ if (LL > 2) LogFile::ErrorMessage("[DEBUG] OK entro!!!!: diff : "LLD"\n", diffLength);
				string spaces(diffLength, ' ');
				bodyT += spaces;
				BodyLength += spaces.size();
				acumulateBodyLength += spaces.size();
				if (LL > 2) LogFile::ErrorMessage("[DEBUG]Entro! BodyLength: '%i', acuBodyLength: "LLD", contentLengthServer: "LLD"\n", BodyLength, acumulateBodyLength, contentLengthServer);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] OK entro!!!!: BODY[%i]\n", BodyLength);
			} else {
				//~ if (LL > 2) LogFile::ErrorMessage("[DEBUG] No entro!: diff : "LLD"\n", diffLength);
			}
		} else if (r.domain.find("NetflixjsID") != string::npos) {
			int n;
			string strobject, strreplace;
			bodyTTemp = bodyT;

			strobject = "function fd(a,b,c,d){var f=";
			//string strreplace = "function fd(a,b,c,d){console.log(b);if(b.search(\"?o=\")>=0){b+='&watchid=" + r.file + "';} var";
			strreplace = "function fd(a,b,c,d){var urlcache=document.location.href;if(/^http.{3,4}www.netflix.com\\/watch\\/[0-9]+(\\?.*|$)/.test(urlcache) && /\\?o=/.test(b)){var watchid=(urlcache.split('?'))[0].split('watch/')[1];b+='&watchid='+watchid;} if(/^http.{3,4}www.netflix.com\\/WiPlayer\\?.*movieid=[0-9]+/.test(urlcache) && /\\?o=/.test(b) && !/episodeId=/.test(urlcache)){var watchid=urlcache.split('movieid=')[1].split('&')[0];b+='&watchid='+watchid;} var f=";
			n = SearchReplace(bodyTTemp, strobject, strreplace);
			BodyLength += n * (strreplace.size() - strobject.size());

			acumulateBodyLength += BodyLength;
			bodyT = bodyTTemp;
			int64_t diffLength = contentLengthServer - acumulateBodyLength;
			if (LL > 2) LogFile::ErrorMessage("[DEBUG]BodyLength: '%i', acuBodyLength: "LLD", contentLengthServer: "LLD"\n", BodyLength, acumulateBodyLength, contentLengthServer);
			if (diffLength <= 3000) {
				string spaces(diffLength, ' ');
				bodyT += spaces;
				BodyLength += spaces.size();
				acumulateBodyLength += spaces.size();
				if (LL > 2) LogFile::ErrorMessage("[DEBUG]Entro! BodyLength: '%i', acuBodyLength: "LLD", contentLengthServer: "LLD"\n", BodyLength, acumulateBodyLength, contentLengthServer);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] OK entro!!!!: BODY[%i]\n", BodyLength);
			} else {
				if (LL > 2) LogFile::ErrorMessage("[DEBUG] No entro!: diff : "LLD"\n", diffLength);
			}
		} else if (r.domain == "netflix" && !r.match) {
			acumulateBodyLength += BodyLength;
			readFileHeader += bodyT;
			if (readFileHeader.size() >= LENGTHHEADERFILE && !haveUpdateDB && !range_min) {
				vector<string> lname;
				string partVideo = readFileHeader.substr(0, LENGTHHEADERFILE);
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] file '%s', partVideo: \"%s\"[%i]\n", r.file.c_str(), partVideo.c_str(), partVideo.size());

				splitstring(r.file, DELIM, &lname);
				if (lname.size() == 1) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] Problem with the file: '%s'?, format not correct!\n", r.file.c_str());
					return BodyLength;
				}
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] file '%s', partsFileName: ('%s', '%s')!\n", r.file.c_str(), lname.at(0).c_str(), lname.at(1).c_str());

				if (domaindb.set("START TRANSACTION;") != 0) {
					if (LL > 1) LogFile::ErrorMessage("[DEBUG-readpart] Problem with the file '%s', error whit transaction on mysql:'%s'!\n", r.file.c_str(), domaindb.getError().c_str());
					return BodyLength;
				}
				string peti = "SELECT * FROM haarp WHERE file like concat('" + domaindb.sqlconv(lname.at(0)) + "','"DELIM"',md5('" + domaindb.getRealEscapeString(partVideo) + "'),'"DELIM"%') and domain='" + r.domain + "' FOR UPDATE;";
				if (domaindb.get(peti) != 0) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] Error mysql '%s', query: '%s'!\n", domaindb.getError().c_str(), peti.c_str());
					if (LL > 0) LogFile::ErrorMessage("Error mysql 2 : '%s'\n", domaindb.getError().c_str());
					domaindb.set("ROLLBACK;");
					return BodyLength;
				}
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] mysql query: '%s'!\n", peti.c_str());
				if (!domaindb.get_num_rows()) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] File %s [%lli-%lli] NOT EXISTS ON DB =========================.===========\n", r.file.c_str(), range_min, range_max);
					addUserCache(lusers_db, ip_browser, time(NULL), 0, 0);
					string queryStr = "INSERT INTO haarp (domain, file, size, modified, downloaded, bytes_requested, last_request, file_used, users) VALUES ('" + r.domain + "', concat('" + domaindb.sqlconv(lname.at(0)) + "','"DELIM"',md5('" + domaindb.getRealEscapeString(partVideo) + "'),'"DELIM"','" + domaindb.sqlconv(lname.at(1)) + "'), 0, now(),now(),0,now(), 0, '" + lusercache2str(lusers_db) + "');";

					if (domaindb.set(queryStr) != 0) {
						domaindb.set("ROLLBACK;");
						if (LL > 0) LogFile::ErrorMessage("[DEBUG-readpart] File %s [%lli-%lli] Error on INSERT! (NOT EXISTS FILE ON DB): '%s'\n", r.file.c_str(), range_min, range_max, queryStr.c_str());
						if (LL > 0) LogFile::ErrorMessage("Error mysql 3: '%s'\n", domaindb.getError().c_str());
						return BodyLength;
					}
				} else {
					if (domaindb.set("UPDATE haarp set file=concat('" + domaindb.sqlconv(lname.at(0)) + "','"DELIM"',md5('" + domaindb.getRealEscapeString(partVideo) + "'),'"DELIM"','" + domaindb.sqlconv(lname.at(1)) + "'), modified=now(), file_used=0 WHERE domain='" + r.domain + "' and file like concat('" + domaindb.sqlconv(lname.at(0)) + "','"DELIM"',md5('" + domaindb.getRealEscapeString(partVideo) + "'),'"DELIM"%');") != 0) {
						if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] File %s [%lli-%lli] ERROR MYSQL: '%s'!.\n", r.file.c_str(), range_min, range_max, domaindb.getError().c_str());
						domaindb.set("ROLLBACK;");
						return BodyLength;
					}
				}
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] file '%s'[%lli-%lli], COMMITING .... !\n", r.file.c_str(), range_min, range_max);
				domaindb.set("COMMIT;");
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-readpart] file '%s'[%lli-%lli], COMMITING .... CHAO!!\n", r.file.c_str(), range_min, range_max);
				haveUpdateDB = true;
			}
		}
		//
		else if (r.match) {
			if (!cachefile.is_open()) {
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] Opening file (MISS): '%s'!\n", completefilepath.c_str());
				if (LL > 2) print_list_lranges(lrangeswork, "lrangeswork");
				if (!file_exists(completefilepath)) {
					cachefile.open(string(completefilepath).c_str(), ios::out | ios::binary);
					cachefile.close();
				}
				cachefile.open(string(completefilepath).c_str(), ios::out | ios::binary | ios:: in );
				/**/
				bwrite = 0;
				brange = lrangeswork.begin();
				if (brange->position < 0) {
					brange->position = getPointEnd(lranges);
					bwrite = 1;
				}
				longSeekpFile(cachefile, brange->position);
				acumulate = 0;
				limit = brange->b - brange->a + 1;
				/**/
				if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] Open file (MISS): '%s'!\n", completefilepath.c_str());
			}
			if (BodyLength > 0) {
				long long int pos = 0;
				while (acumulate + BodyLength > limit) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] Dates more that the Limit (acumulate:%lli,bodylength:%i|limit:%lli), bwrite: %i!\n", acumulate,BodyLength,limit,bwrite);
					if (LL > 2) print_list_lranges(lrangeswork, "lrangeswork");
					if (LL > 2) print_range(*brange, "brange");
					long long int btmp = limit - acumulate;
					cachefile.write(bodyT.substr(pos).c_str(), btmp);
					filesended += btmp;
					BodyLength -= btmp;
					pos += btmp;
					acumulate = 0;
					if (bwrite) { //1 - block miss
						np++;
						if (!appendNode(lranges, *brange)) {
							if (LL > 1) LogFile::ErrorMessage("Error, adicionando bloque en cachefile!\n");
						}
					}
					bwrite = 0;
					++brange;
					if (brange == lrangeswork.end()) {
						if (LL > 0) LogFile::ErrorMessage("Error, el archivo descargado contiene más informacion de lo esperado!\n");
						r.match = hit = false;
						cachefile.close();
						if (!exists_transaction_editing_file) liberate_edition();
						return -1;
					}
					if (brange->position < 0) {
						brange->position = getPointEnd(lranges);
						bwrite = 1;
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] Before of seekp!\n");
					longSeekpFile(cachefile, brange->position);
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] After of seekp!\n");
					limit = brange->b - brange->a + 1;
					/**/
					if (!bwrite && is_all_hit(lrangeswork, brange)) {
						miss2hit = 1; /* Cambiar de miss a hit (apartir de aqui todo desde el cache).*/
						if (LL > 2) LogFile::ErrorMessage("Error!, Cambiando MISS to HIT! - 1 -\n");
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] Dates more that the Limit - chao!\n");
				}
				cachefile.write(bodyT.substr(pos).c_str(), BodyLength);
				acumulate += BodyLength;
				filesended += BodyLength;
				if (acumulate == limit) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] before - acumulate == Limit!\n");
					cachefile.flush();
					acumulate = 0;
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] after flush() - acumulate == Limit!\n");
					if (LL > 2) print_list_lranges(lranges, "lranges");
					if (LL > 2) print_range(*brange, "brange");
					if (bwrite) {
						np++;
						if (!appendNode(lranges, *brange)) {
							if (LL > 2) LogFile::ErrorMessage("Error!, Adicionando bloque en cachefile!\n");
						}
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] before ++brange - acumulate == Limit!\n");
					++brange;
					if (brange == lrangeswork.end() && filesended == filesizeneto) {
						return BodyLengthTmp;
					} else
					if (brange == lrangeswork.end() || filesended == filesizeneto) {
						cachefile.close();
						return -1;
					}
					bwrite = 0;
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] after ++brange - acumulate == Limit!\n");
					if (brange->position < 0) {
						// np++;
						brange->position = getPointEnd(lranges);
						bwrite = 1;
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] seekp before - acumulate == Limit!\n");
					longSeekpFile(cachefile, brange->position);
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] seekp after - acumulate == Limit!\n");
					limit = brange->b - brange->a + 1;
					/**/
					if (!bwrite && is_all_hit(lrangeswork, brange)) {
						miss2hit = 1;
						if (LL > 2) LogFile::ErrorMessage("Error!, Cambiando MISS to HIT! - 2 -\n");
					}
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadPartBody] after - acumulate == Limit!\n");
				}
				if ((time(NULL) - timerecord) > 0.8) {
					if (LL > 2) LogFile::ErrorMessage("[DEBUG-ReadBodyPart] in time now - time record!\n");
					cachefile.flush();
					file_setmodif(completefilepath);
					timerecord = time(NULL);
					if (++count_wait > 9 && acumulate && bwrite) {
						count_wait = 0;
						if (!appendSubNode(lranges, *brange, acumulate)) {
							if (LL > 0) LogFile::ErrorMessage("Warning: adding a sub block with lost position!\n");
						}
						brange->a += acumulate;
						brange->position += acumulate;
						limit -= acumulate;
						acumulate = 0;
						
						SubUpdate(filesended - oldfilesended);
						oldfilesended = filesended;
					}
				}
				BodyLength = BodyLengthTmp;
				if (miss2hit)
					cachefile.flush();
			}
		}
	}
	return BodyLength;
}

void ConnectionToHTTP2::CloseUpdate() {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-CU] Closing Connection!\n");
	Update();
	if (!exists_transaction_editing_file) liberate_edition();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-CU] Liberated edition from db\n");
	domaindb.close();
	if (cachefile.is_open()) cachefile.close();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-CU] Cerrando cache file close after!\n");
	if (outfile.is_open()) outfile.close();
	if ((filesended < filesizeneto) && ((r.match && !hit) || (r.match && resuming)))
		file_setmodif(completefilepath, 1);
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-CU] Closed file, domaindb\n");
	downloader.Close();
	ConnectionToHTTP::Close();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG-CU] Closed - BYE!.\n");
}

void ConnectionToHTTP2::Close() {
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Closing Connection!\n");
	domaindb.close();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Closed domaindb!\n");
	if (cachefile.is_open()) cachefile.close();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Cerrando cache file close after!\n");
	if (outfile.is_open()) outfile.close();
	downloader.Close();
	ConnectionToHTTP::Close();
	if (LL > 2) LogFile::ErrorMessage("[DEBUG] Closed - BYE!.\n");
}
