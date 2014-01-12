Introduction
------------

Proxy HaarpCache is a fork of ThunderCache.

HaarpCache is a robust static and dynamic cache that provides support to the DASH technology.

Installing
--------

	cd /tmp/
	git clone https://github.com/keikurono/haarpcache.git
	cd haarpcache
	./configure CXX=g++-4.4 (recommended install g++-4.4)
	make
	make install


* Copy & page this, in the file of configuration: squid.conf (install squid)

		acl haarp_lst url_regex -i "/etc/haarp/haarp.lst"
		cache deny haarp_lst
		cache_peer 127.0.0.1 parent 8080 0 proxy-only no-digest
		dead_peer_timeout 2 seconds
		cache_peer_access 127.0.0.1 allow haarp_lst
		cache_peer_access 127.0.0.1 deny all

You can change the address 127.0.0.1 for the IP of you server haarp.

* To clean: make clean

 
List of Plugins
--------------

* Currently Haarp supports:


	Youtube,
	DoubleClick (Action of Blocked of the ADS),
	Googlevideo,
	AOL,
	Vimeo,
	Metacafe,
	Bitgravity,
	Globo,
	Terra,
	Viddler,
	Justin,
	Edgecastcdn,
	Youku,
	Wrzuta,
	Ytimg,
	Tumblr,
	Facebook;
	
	
	4shared,
	Mediafire,
	Etrustdownloads,
	Ziddu;
	
	Avast,
	Avgate,
	Eset,
	Mcafee;
	
	Windowsupdate,
	Adobe;
	
	Porntube,
	Xvideos,	
	Pornhub,
	Redtube,
	Tube8;
	
	juegosdiarios,
	juegosjuegos.
	
	
Configure your file /etc/haarp/haarp.lst to disable or enable plugins.	

	
Changelog
---------

__Version 1.2__

2013/12/24

* Improved ad-blocking plugin for Youtube.
* Update plugins: globo.com, youtube.com.
* Differentiation of the header for the chrome browser.
* Update of haarpclean script for the cleaning the cache.
* Synchronous to the storage of the cache in an only file.

__Version 1.1__

2013/02/09

* Cache dynamic and intelligent.
* Update of plugins for the smart operation.
* Multi disc
* Blocking advertising to Youtube.
* Cache of videos in html5 for Youtube.

Fork created by

Manolo Canales

kei.haarpcache@gmail.com

http://perufw.wordpress.com
