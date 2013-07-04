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


extern int LL; //LogLevel


void ConnectionToHTTP2::getLimitBytes(string &header) {
	if (LL > 0) LogFile::AccessMessage("******************** NEW CONNECTION ********************\n");
	range_min = 0;
	range_max = 0;
	np = 0;
	partial = false;
	vector<string> lines, value, rangeb;
	stringexplode(header, "\r\n", &lines);
	for (unsigned int i = 0; i <= lines.size() - 1; ++i) {
		if ( !i && lines.at(0).find(" 206 ") != string::npos && lines.at(0).find("HTTP") != string::npos ) {
			if (LL > 1) LogFile::AccessMessage("Header with status code 206 found\n");
			partial = true;
		}
		if (lines.at(i).find("Range: bytes=") == 0) {
			stringexplode(lines.at(i), "=", &value);
			if(value.size() <= 1)
				continue;
			partial = true;
			stringexplode(value.at(1), "-", &rangeb);
			if(!rangeb.size())
				continue;
			if(rangeb.size() == 1) {
				range_min = atoi(rangeb.at(0).c_str());
				range_max = -1;
			}
			else {
				range_min = atoi(rangeb.at(0).c_str());
				range_max = atoi(rangeb.at(1).c_str());
			}
			break;
		}
	}
}
int ConnectionToHTTP2::getOnlyContentLength( string header ) {
	vector<string> lines, value;
	stringexplode(header, "\r\n", &lines);
	int result = -1;
	for (int i = 0; i <= (int)lines.size() - 1; i++) {
		if (lines.at(i).find("HTTP") == 0 && lines.at(i).find("200") == string::npos && lines.at(i).find("206") == string::npos)
			return -1;		
		if (lines.at(i).find("Content-Length:") == 0) {
			stringexplode(lines.at(i), ":", &value);
			result = atoi(value.at(1).c_str());
		}
	}
	return result;
}
/* 
 * size_orig_file es el tamaño del archivo en disco.
 * size es el tamaño del archivo real, en el servidor (0: desconocido).
 */
void ConnectionToHTTP2::Cache2( int cl ) {
	
	if( !size_orig_file && range_max <= 0) {
		size_orig_file = cl + range_min;
		if (general)
			domaindb.set("UPDATE haarp SET size=" + itoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
		else
			domaindb.set("UPDATE haarp SET size=" + itoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
	}
	if( range_max <= 0 ) {
		range_max = cl + range_min - 1;
		list_clear(&lrangeswork);
	}
	else {
		if( range_max != cl + range_min - 1 )
		{
			range_max = cl + range_min - 1;
			list_clear(&lrangeswork);
		}
	}
	filesizeneto = cl;
	if( !lrangeswork )
	{
		lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit); // update HIT?.
		if ( !hit ) {
			short wait = WaitLastModification();
			if ( wait == -1 ) 
				r.match = false; /* Go direct to internet */
			if ( !wait && !ReloadData()) {
				return;
			} 
		}
		if( hit ) {
			msghit = "HIT";
			domaindb.set("UPDATE haarp SET requested=requested+1, last_request=now() WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
		}
	}	
}
/*
 * En cabeceras partial, se obtiene lo siguiente: "Content-Range: bytes a-b/c", donde c expresa la longitud total del archivo de verdad.
 * Aqui se pretende actualizar el valor de size en la bd con el valor del tamaño del archivo (c).
 */
void ConnectionToHTTP2::UpdateFileSizeinPartial( string header ) {
	vector<string> lines, value;
	stringexplode(header, "\r\n", &lines);
	for (unsigned int i = 1; i <= lines.size() - 1; ++i) {
		if (lines.at(i).find("Content-Range: bytes") == 0) {
			stringexplode(lines.at(i), "/", &value);
			if(value.size() <= 1)
				continue;
			size_orig_file = atol(value.at(1).c_str());
			domaindb.set("UPDATE haarp SET size=" + itoa(size_orig_file) + " WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';"); 
			return;
		}
	}
}
void ConnectionToHTTP2::Update(){
	string rang_, part_;
	if( r.match && !hit ) {
		if( acumulate && bwrite ) {
			if (cachefile.is_open()) {
				cachefile.flush();
			}
			brange->b = brange->a + acumulate - 1;
			if(!appendNode(&lranges, brange))			
				if (LL > 1) LogFile::ErrorMessage("Error!, block range is NULL and acumulate not is empty!\n");

		}
		if( !hasupdate )
		{
			list2string(lranges,rang_,part_);
			long int f_n = getFileSize(lranges);
			stringstream stmp0,stmp1;
			stmp0<<f_n;
			stmp1<<np;
			if (LL > 1) LogFile::AccessMessage("Update data base with ranges=%s, partition=%s, domain=%s and file=%s\n", rang_.c_str(), part_.c_str(),r.domain.c_str(), r.file.c_str());
			if(domaindb.set("UPDATE haarp set rg='" + rang_ + "', pos='" + part_ + "', filesize='" + stmp0.str() + "', np=np+" + stmp1.str() + " WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") < 0)
				if (LL > 1) LogFile::ErrorMessage("Error updating data base, %s \n",domaindb.getError().c_str());
			hasupdate = true;
		}
		acumulate = 0;
		list_clear(&lranges);
	}
}
void ConnectionToHTTP2::SubUpdate() {
	string rang_, part_;
	list2string(lranges,rang_,part_);
	long int f_n = getFileSize(lranges);
	stringstream stmp0,stmp1;
	stmp0<<f_n;
	stmp1<<np;
	if (LL > 1) LogFile::AccessMessage("Saving part in the database with: ranges='%s', partition='%s', domain='%s' and file=%s\n", rang_.c_str(), part_.c_str(),r.domain.c_str(), r.file.c_str());
	if(domaindb.set("UPDATE haarp set rg='" + rang_ + "', pos='" + part_ + "', filesize='" + stmp0.str() + "', np=np+" + stmp1.str() + " WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';") < 0)
		if (LL > 1) LogFile::ErrorMessage("Error, in sub-updating the data base: '%s' \n",domaindb.getError().c_str());
}
short ConnectionToHTTP2::WaitLastModification() {
	int contador = 0;
	while ( (now() - file_getmodif(completefilepath)) <= 1.8 ) {
		usleep(500000);
		if ( ++contador > 3 ) // + de 5 segundos
		{
			if(LL > 0) LogFile::ErrorMessage("Warning: file '%s' with persistent changes\n", r.file.c_str());
			if(LL > 0) LogFile::AccessMessage("The file on disk is modified, go direct to internet.\n");
			return -1;
		}
	}
	if( contador )
		return 0;
	return 1;
}
bool ConnectionToHTTP2::ReloadData() {
	list_clear(&lranges);
	list_clear(&lrangeswork);
	
	if (domaindb.get("SELECT size, rg, pos FROM haarp WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';") != 0) {
		LogFile::ErrorMessage("Error: '%s'\n", domaindb.getError().c_str());
		r.match = hit = false;
		return false;
	}

	size_orig_file = atol(domaindb.get("size", 1).c_str());
	string ranges = domaindb.get("rg", 1);
	string position = domaindb.get("pos", 1);
	
	if (LL > 0) LogFile::AccessMessage("In RELOAD DB: ranges ('%s') and position ('%s')\n", ranges.c_str(), position.c_str());

	if( !generateList(ranges, position, &lranges) ) { //raro que entre
		if (LL > 0) LogFile::ErrorMessage("Format incorrect: Ranges=%s, Parts=%s\n", ranges.c_str(), position.c_str());
		if (LL > 1) LogFile::AccessMessage("Format incorrect: Reset ranges and positions in the DB\n");
		domaindb.set("UPDATE haarp set rg='', pos='' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';");
	}
	if( size_orig_file ) {
		if(range_max <= 0 )
			range_max = size_orig_file - 1; // update range_max
		if( size_orig_file > range_min + 1 && size_orig_file < range_max + 1 ) {
			range_max = size_orig_file - 1; // truncate
			if (LL > 1) LogFile::AccessMessage("Maximum range, trucate to: %i bytes\n", range_max);
		}
		filesizeneto = range_max - range_min + 1;
		lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit); /* hit, can not change it later */
	}
	else {
		if ( range_max > 0 && ( getExtremeb(lranges) < range_min || range_max <= getExtremeb(lranges) ) ) {
			lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit); /* hit, can not change it later */
		}
		else { // range_max <= 0 || range_min <= exteme < range_max
			hit = false; /* can be changed later (function Cache2) */
		}
	}
	return true;
}
void ConnectionToHTTP2::Cache() {
    msghit = "MISS";
    hasupdate = false;
    unlimit = false;
    miss2hit = false;
    count_wait = 0;
    hit = downloading = r.match = rewrited = resuming = passouheader = general = etag = closed = false;
    limit = 0;
    np = 0;
    lranges = brange = lrangeswork = NULL;
     
    filesizeneto = size_orig_file = filedownloaded = filesended = expiration = 0;
    
    srand(time(0));
    
    if (LL > 0) LogFile::AccessMessage("Url %s%s\n", domain.c_str(), request.c_str());
    string domaintmp = getdomain(domain + request); //youtube.com
    string pluginpath = pluginsdir + domaintmp + ".so";
	
    if (file_exists(pluginpath)) {
        if (LL > 0) LogFile::AccessMessage("Loading plugin %s\n", pluginpath.c_str());
        void* handle = dlopen(pluginpath.c_str(), RTLD_LAZY);
        if (!handle) {
            if (LL > 0) LogFile::ErrorMessage("Cannot open library: %s\n", dlerror());
        }
        typedef resposta(*plugin_t)(string);
        plugin_t plugin = (plugin_t) dlsym(handle, "hgetmatch2");
        if (!plugin) {
            if (LL > 0) LogFile::ErrorMessage("Cannot load symbol: %s\n", dlerror());
            dlclose(handle);
        } else { // Plugin cargado correctamente, ok para atribuir valores a struc
            r = plugin(domain + request);
            if( !partial || (partial && !range_max) ) {
				range_min = r.range_min;
				range_max = r.range_max;
			}
			else {
				if( r.domain == "youtube" ) {
					r.file = "206-" + r.file;
					int postmp;
					if( (postmp = (int)(r.file).rfind(".flv")) != (int)string::npos) {
						(r.file).replace(postmp,4,".webm");
					}
				}
			}
			if (LL > 1) LogFile::AccessMessage("Ranges received: range_min=%d, range_max=%d\n", range_min, range_max);
            if (LL > 0) LogFile::AccessMessage("Resposta Match %d Domain %s File %s\n", r.match, r.domain.c_str(), r.file.c_str());
        }
    }
	
    if (r.match) { // Es la url de un dominio
        subdir = ConvertChar(r.file);
        
        vector<string> list_dir;
        string directory, directorytmp, completefilepathtmp, completepathtmp;
        stringexplode(cachedir,"|",&list_dir);
        
        bool disco_con_espacio = false;
        bool file_in_cache = false;
        
        for(int i=0;i < (int)list_dir.size();i++) {
			directory = trimstr(list_dir.at(i));
			
			completepath = directory + r.domain;
			
			completefilepath = completepath + "/" + subdir + "/" + r.file;
			
			if(file_exists(completefilepath)) {
				file_in_cache = true;
				break;
			}
			if( !disco_con_espacio && disk_use(completepath) <= cache_limit ) {
				disco_con_espacio = true;
				directorytmp = directory;
				completefilepathtmp = completefilepath;
				completepathtmp = completepath;
			}
		}
		if(!file_in_cache && disco_con_espacio) {
			completefilepath = completefilepathtmp;
			completepath = completepathtmp;
			directory = directorytmp;
		}
        if (LL > 0) LogFile::AccessMessage("Cache limit (%.0lf/%d) %s\n", disk_use(completepath), cache_limit, directory.c_str());
        if (LL > 0) LogFile::AccessMessage("File: %s\n", string(completefilepath).c_str());
        if(range_max > 0)
			filesizeneto = range_max - range_min + 1;
        if (r.domain == "rewrite") {
            r.match = false;
            rewrited = true;
            if (LL > 1) LogFile::AccessMessage("Rewrite: %s \n", r.file.c_str());
        } else if (!file_in_cache) { //si no está en cache - MISS!!!!??????
			if (LL > 0) LogFile::AccessMessage("The file is not on disk\n");
			/* MISS */
			lranges = NULL;
			if( range_max > 0 ) {
				lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit);
			}
            if (disco_con_espacio) {
                if (!file_exists(completepath + "/" + subdir + "/")) {
                    mkdir_p(completepath + "/" + subdir + "/");
                }
                domaindb.set("DELETE FROM haarp WHERE domain='" + r.domain + "' and file='" +  domaindb.sqlconv(r.file) + "';");
                domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, requested, last_request) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now());");
                if (LL > 0) LogFile::AccessMessage("MISS: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
            } else {
                hit = r.match = false;
            }
        } else { /* El archivo está en disco.*/
			if (LL > 1) LogFile::AccessMessage("The file is located on disk!\n");

            if (domaindb.get("SELECT size, rg, pos FROM haarp WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';") != 0) {
                LogFile::ErrorMessage("Error select mysql: %s\n", domaindb.getError().c_str());
                r.match = hit = false;
                return;
            }


            if (domaindb.get_num_rows() == 0) // No existe en la db, pero sí en disco, entonces crear nueva entrada en la db.
            {
				/* MISS */
				lranges = NULL;
				if( range_max > 0 ) {
					lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit);
					if ( !WaitLastModification() ) {
						r.match = false; /* Go direct to internet */
					}
				}
				/* END */
                domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, requested, last_request) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now());");
                if (LL > 0) LogFile::AccessMessage("MISS DB: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
            } else { // Existe el archivo en disco y en la base de datos, miss o hit?.
				/* size es el tamaño del archivo, si estubiera entero (como en el mismo servidor)*/
				size_orig_file = atol(domaindb.get("size", 1).c_str());
				
				string ranges = domaindb.get("rg", 1);
				string position = domaindb.get("pos", 1);
				if (LL > 0) LogFile::AccessMessage("In DB: ranges ('%s') and position ('%s')\n", ranges.c_str(), position.c_str());
				
				if( !generateList(ranges, position, &lranges) ) { //raro que entre
					if (LL > 0) LogFile::ErrorMessage("Format incorrect: Ranges=%s, Parts=%s\n", ranges.c_str(), position.c_str());
					if (LL > 1) LogFile::AccessMessage("Format incorrect: Reset ranges and positions in the DB\n");
					domaindb.set("UPDATE haarp set rg='', pos='' WHERE domain='" + r.domain + "' and file='" + domaindb.sqlconv(r.file) + "';");
				}
				if( size_orig_file )
				{
					if(range_max <= 0 )
						range_max = size_orig_file - 1; // update range_max
					if( size_orig_file > range_min + 1 && size_orig_file < range_max + 1 ) {
						range_max = size_orig_file - 1; // truncate
						if (LL > 1) LogFile::AccessMessage("Maximum range, trucate to: %i\n", range_max);
					}
					filesizeneto = range_max - range_min + 1;
					lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit); /* hit, can not change it later */
					if ( !hit ) {
						short wait = WaitLastModification();
						if ( wait == -1 ) 
							r.match = false; /* Go direct to internet */
						if ( !wait && !ReloadData()) {
							return;
						} 
					}
				}
				else {
					if ( range_max > 0 && ( getExtremeb(lranges) < range_min || range_max <= getExtremeb(lranges) ) ) {
						lrangeswork = getRangeWork(&lranges, range_min, range_max, &hit); /* hit, can not change it later */
						if ( !hit ) {
							short wait = WaitLastModification();
							if ( wait == -1 ) 
								r.match = false; /* Go direct to internet */
							if ( !wait && !ReloadData()) {
								return;
							} 
						}
					}
					else { // range_max <= 0 || range_min <= exteme < range_max
						hit = false; /* can be changed later (function Cache2) */
					}
				}
				
				if( hit ) {
					msghit = "HIT";
					domaindb.set("UPDATE haarp SET requested=requested+1, last_request=now() WHERE file='" + domaindb.sqlconv(r.file) + "' and domain='" + r.domain + "';");
					if (LL > 0) LogFile::AccessMessage("HIT: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
				}
			}
            domaindb.clear();
        }
        
    } else if (!rewrited) { // sistema de cache de arquivos ESTÁTICOS GENERAL
        if (Params::GetConfigInt(getFileExtension(getFileName(request)) + "_EXP") > 0 && request.find_first_not_of("()[]?&=;,´`'\"") == string::npos) { // se está configurada essa extensão
			//~ if (LL > 0) LogFile::AccessMessage("(2)**** Entró !rewrited!!\n");
            r.file = getFileName(request);
            r.domain = domaintmp;
            subdir = request.substr(1, request.length() - r.file.length() - 2);
            vector<string> list_dir;
            stringexplode(cachedir,"|",&list_dir);
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
						domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, requested, last_request, static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
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
						domaindb.set("INSERT INTO haarp (domain, file, size, modified, downloaded, requested, last_request,static) VALUES ('" + r.domain + "', '" + domaindb.sqlconv(subdir + "/" + r.file) + "', 0, '1980-01-01 00:00:00',now(),0,now(),1);");
						if (LL > 0) LogFile::AccessMessage("MISS DB: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
					} else if (
							((now() - file_getmodif(completefilepath)) < 10)
							&&
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
							domaindb.set("UPDATE haarp SET requested=requested+1, last_request=now() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
							if (LL > 0) LogFile::AccessMessage("HIT!: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							if (LL > 0) LogFile::AccessMessage("EXPIRED: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							domaindb.set("UPDATE haarp SET downloaded=NOW() WHERE file='" + domaindb.sqlconv(subdir + "/" + r.file) + "' and domain='" + r.domain + "';");
						}
					}
					else if (size_orig_file > 0) { // o arquivo foi baixado parcialmente
						if (disk_use(completepath) <= cache_limit) {
							hit = resuming = downloading = true;
							downloader.ClearVars();
							downloader.SetDomainAndPort(domain, port);
							if (LL > 0) LogFile::AccessMessage("HIT RESUME: Domain: %s File: %s\n", r.domain.c_str(), r.file.c_str());
							msghit = "HIT";
						} else {
							LogFile::ErrorMessage("Cache limit (%.0lf/%d) %s%s\n", disk_use(completepath), cache_limit , cachedir.c_str(), r.domain.c_str());
						}
					}
					domaindb.clear();
				}
            }
        }
    }
}
bool ConnectionToHTTP2::SetDomainAndPort(string domainT, int portT, string requestT) {
    if (!domaindb.connected)
        if (domaindb.open(Params::GetConfigString("MYSQL_HOST"), Params::GetConfigString("MYSQL_USER"), Params::GetConfigString("MYSQL_PASS"), Params::GetConfigString("MYSQL_DB")) != 0) {
            LogFile::ErrorMessage("Error, connection mysql: %s\n", domaindb.getError().c_str());
            Close();
        }
    domain = domainT;
    request = requestT;
    port = portT;
    pluginsdir = Params::GetConfigString("PLUGINSDIR");
    cachedir = Params::GetConfigString("CACHEDIR");
    cache_limit = Params::GetConfigInt("CACHE_LIMIT");

    Cache();

    if (hit)
        return true;
    else
        return ConnectionToHTTP::SetDomainAndPort(domain, port);
}
bool ConnectionToHTTP2::ConnectToServer() {
    if (rewrited) return true;
    if (resuming) {
        if (!downloader.ConnectToServer()) {
            if (LL > 0) LogFile::AccessMessage("downloader not conect\n");
            Close();
        }
    }
    if (hit) return true;
    else 
		return ConnectionToHTTP::ConnectToServer();
}
// header cabecera a enviar al servidor (la misma del browser!). no depende de hit u otras variables parecidas!
// ConnectionClose = "DropBrowser"
// requestT = ToBrowser.GetRequest()
//
bool ConnectionToHTTP2::SendHeader(string header, bool ConnectionClose, string requestT) {
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
    } else
	{
		bool tmp = ConnectionToHTTP::SendHeader(header, ConnectionClose); //la misma que envia el browser aqui
		return tmp;
	}		
}
string ConnectionToHTTP2::GetIP() {
    if (hit || rewrited) return "0.0.0.0";
    else 
		return ConnectionToHTTP::GetIP();
}
bool ConnectionToHTTP2::ReadHeaderFromServer(string &headerT) {
	bool result = ConnectionToHTTP::ReadHeader(headerT);
	if(result) {
		if( r.domain == "youtube" && getFileExtension(r.file) == "FLV" ) {	
			size_t pos;
			char line[90];
			//if (LL > 0) LogFile::AccessMessage("BEFORE-HEADER: %s\n", headerT.c_str());
			if( (pos = headerT.find("Content-Type:")) != string::npos && (headerT[pos - 1] == '\n' || headerT[pos - 1] == '\r') ) {
				sscanf((headerT.substr(pos + 14)).c_str(), "%s", line);
				headerT.replace(pos,strlen(line) + 15,"Content-Type: video/x-flv");
			}
			//if (LL > 0) LogFile::AccessMessage("AFTER-HEADER: %s\n", headerT.c_str());
		}
	}
	return result;
}
/* headerT: Edita la cabezera que será enviada al navegador.
 * */
bool ConnectionToHTTP2::ReadHeader(string &headerT) {
    bool result = false;
    if( !size_orig_file && r.match && !hit ) //truncar, partial, !knowhitmiss
    {
		result = ConnectionToHTTP2::ReadHeaderFromServer(headerT);
		if ( partial )
		{
			UpdateFileSizeinPartial(headerT);
			if( range_max <= 0 )
				range_max = size_orig_file - 1;
		}
		int cl = getOnlyContentLength(headerT);
		if(cl < 0)
		{
			return result;
		}
		Cache2(cl);//generate lrangeswork!
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
        return true;
    } else if (hit) {
		
        stringstream tmp;
        tmp.str("");
        if ( !partial )
            tmp << "HTTP/1.0 200 OK\r\n";
        else
            tmp << "HTTP/1.0 206 Partial\r\n";
        //tmp << "Content-Disposition: attachment; filename=\"" + r.file + "\"\r\n";
        if (getFileExtension(r.file) == "SWF")
            tmp << "Content-Type: application/x-shockwave-flash\r\n";
        else if (getFileExtension(r.file) == "FLV")
            tmp << "Content-Type: video/x-flv\r\n";
        else if (getFileExtension(r.file) == "MP4")
            tmp << "Content-Type: video/mp4\r\n";
        else if (getFileExtension(r.file) == "MP4A" && r.domain == "youtube")
            tmp << "Content-Type: audio/mp4\r\n";
		else if (getFileExtension(r.file) == "WEBM")
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
        headerT = tmp.str();
        return true;

    } else {
		if(result)
			return true;
		bool re = ConnectionToHTTP2::ReadHeaderFromServer(headerT);
		return re;
	}
	
}
bool ConnectionToHTTP2::AnalyseHeader(string &linesT) {
    if (hit) return true;
    else return ConnectionToHTTP::AnalyseHeader(linesT);
}
bool ConnectionToHTTP2::IsItKeepAlive() {
    if (hit) return true;
    else return ConnectionToHTTP::IsItKeepAlive();
}
int64_t ConnectionToHTTP2::GetContentLength() {
	int64_t tmp;
    if (hit)
        tmp = filesizeneto;
    else {
        int64_t ContentLengthReference;
        ContentLengthReference = ConnectionToHTTP::GetContentLength(); //del server..
        if (r.match) {
           if ((
					ContentLengthReference < Params::GetConfigInt(getFileExtension(r.file) + "_MIN") || 
					(ContentLengthReference > Params::GetConfigInt(getFileExtension(r.file) + "_MAX") && Params::GetConfigInt(getFileExtension(r.file) + "_MAX") > 0)					
				) || (
					ContentLengthReference < Params::GetConfigInt(UpperCase(r.domain) + "_MIN") || 
					(ContentLengthReference > Params::GetConfigInt(UpperCase(r.domain) + "_MAX") && 
						Params::GetConfigInt(UpperCase(r.domain) + "_MAX") > 0
					)
				)
			)  
	    {
                r.match = general = false;
                if (LL > 0) LogFile::AccessMessage("MAXMIN CANCEL: Domain: %s File: %s Size: "LLD"\n", r.domain.c_str(), r.file.c_str(), filesizeneto);
            }
        }
        tmp = ContentLengthReference;
    }
    return tmp;
}
bool ConnectionToHTTP2::IsItChunked() {
    bool tmp;
    if (hit) return false;
    else tmp =  ConnectionToHTTP::IsItChunked();
	return tmp;
}
string ConnectionToHTTP2::PrepareHeaderForBrowser() {
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
    int retorno;
    if (hit) retorno = 200;
    else if (!hit && r.match) {
        retorno = ConnectionToHTTP::GetResponse();
        if (retorno != 200 && retorno != 206 ) {
            r.match = hit = resuming = false;
        }
    } else retorno = ConnectionToHTTP::GetResponse();
    return retorno;
}
bool ConnectionToHTTP2::CheckForData(int timeout) {
    if (hit) return false;
    else return ConnectionToHTTP::CheckForData(timeout);
}

//filedownloaded - Cantidad de bytes descargados.
ssize_t ConnectionToHTTP2::ReadBodyPart(string &bodyT, bool Chunked) {
    if (rewrited) {
        bodyT.append("\r\n", 2);
        return 2;
    }
    ssize_t BodyLength = 0;
    
    if (resuming) {
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
        memset(memblock, '\0', sizeof (memblock));
        bodyT = "";
        while (BodyLength == 0) {
            if (!cachefile.is_open()) {
				bwrite = 0;
                cachefile.open(string(completefilepath).c_str(), ios::in | ios::binary);
                /**/
				brange = lrangeswork;
				acumulate = 0;
				if( brange->p < 0 )
				{
					BodyLength = -1;
					cachefile.close();
					break;
				}
				cachefile.seekg(brange->p, ios::beg);
				limit = brange->b - brange->a + 1;
				/**/
                timeout = timerecord = time(NULL);
            }
			if(acumulate + tbuffer > limit) // acumulate es siempre menor que limit!
				tbuffer = limit - acumulate;
			
			cachefile.tellg();
			cachefile.read(memblock, tbuffer);
			BodyLength = cachefile.gcount();// Acomoda
			if (BodyLength > 0) {
				filesended += BodyLength;
				acumulate += BodyLength;
				bodyT.append(memblock, BodyLength);
				if(acumulate == limit)
				{
					brange = brange->next;
					acumulate = 0;
					if(brange) {
						cachefile.seekg(brange->p, ios::beg);
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
            if( brange == NULL && filesended == filesizeneto )
            {
				cachefile.close();
                break;
			}
			else			
				if( brange == NULL || filesended == filesizeneto )
				{
					BodyLength = -1;
					cachefile.close();
					break;
				}
        }
    } else { //miss
        BodyLength = 0;
        ssize_t BodyLengthTmp = 0;
        BodyLength = ConnectionToHTTP::ReadBodyPart(bodyT, Chunked);
        BodyLengthTmp = BodyLength;
        if (r.match) {
            if ( !cachefile.is_open() ) {
				if ( !file_exists(completefilepath) ) {
					cachefile.open(string(completefilepath).c_str(), ios::out | ios::binary);
					cachefile.close();
				}
                cachefile.open(string(completefilepath).c_str(), ios::out | ios::binary | ios::in);
                /**/
                bwrite = 0;
				brange = lrangeswork;
				if( brange->p < 0 ) {
					brange->p = getPointEnd(lranges);
					bwrite = 1;
				}
				cachefile.seekp(brange->p, ios::beg);
				acumulate = 0;
				limit = brange->b - brange->a + 1;
                /**/                
            }
            if ( BodyLength > 0 ) {
				int pos = 0;
				while( acumulate + BodyLength > limit ) {
					int btmp = limit - acumulate;
					cachefile.write(bodyT.substr(pos).c_str(), btmp);
					filesended += btmp;
					BodyLength -=  btmp;
					pos +=  btmp;
					acumulate = 0;
					if( bwrite ) { //1 - block miss
						np++;
						if(!appendNode( &lranges, brange )) {
							if (LL > 1) LogFile::ErrorMessage("Error, adicionando bloque en cachefile!\n");
						}
					}
					bwrite = 0;
					brange = brange->next;
					if( !brange ) {
						if (LL > 0) LogFile::ErrorMessage("Error, el archivo descargado contiene más informacion de losesperado!\n");
						cachefile.close();
						return -1;
					}
					if( brange->p < 0 ) {
						brange->p = getPointEnd(lranges);
						bwrite = 1;
					}
					cachefile.seekp(brange->p, ios::beg);
					limit = brange->b - brange->a + 1;
					/**/
					if( !bwrite && is_all_hit(brange) ) {
						miss2hit = 1; /* Cambiar de miss a hit (apartir de aqui todo desde el cache).*/
					}
				}
                cachefile.write( bodyT.substr(pos).c_str(), BodyLength );
                acumulate += BodyLength;
                filesended += BodyLength;
                if( acumulate == limit ) {
					cachefile.flush();
					acumulate = 0;
					if( bwrite ) {
						np++;
						if(!appendNode( &lranges, brange )) {
							if (LL > 0) LogFile::ErrorMessage("Error!, Adicionando bloque en cachefile!\n");
						}
					}
					brange = brange->next;
					if( brange == NULL && filesended == filesizeneto )
					{
						return BodyLengthTmp;
					}
					else
						if( brange == NULL || filesended == filesizeneto )
						{
							cachefile.close();
							return -1;
						}
					bwrite = 0;
					if( brange->p < 0 ){
						// np++;
						brange->p = getPointEnd(lranges);
						bwrite = 1;
					}
					cachefile.seekp(brange->p,ios::beg);
					limit = brange->b - brange->a + 1;
					/**/
					if( !bwrite && is_all_hit(brange) ) {
						miss2hit = 1;
					}
				}
                if ((time(NULL) - timerecord) > 0.8) {
                    cachefile.flush();
                    file_setmodif(completefilepath);
                    timerecord = time(NULL);
                    if( ++count_wait > 9 && acumulate && bwrite )
					{
						count_wait = 0;
						appendSubNode( &lranges, brange, acumulate );
						brange->a += acumulate;
						brange->p += acumulate;
						limit -= acumulate;
						acumulate = 0;
						SubUpdate();
					}
                }
                BodyLength = BodyLengthTmp;
                if(miss2hit)
					cachefile.flush();
            }
        }
    }
    return BodyLength;
}

void ConnectionToHTTP2::Close() {
    if (LL > 1) LogFile::AccessMessage("Closing Connection!\n");
    Update();
    domaindb.close();
    if (cachefile.is_open()) cachefile.close();
    if (outfile.is_open()) outfile.close();
    if ((filesended < filesizeneto) && ((r.match && !hit) || (r.match && resuming)))
        file_setmodif(completefilepath, 1);
    downloader.Close();
    ConnectionToHTTP::Close();
}

