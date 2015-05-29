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
	mysql -u root -p < haarp.sql


* Configure your /etc/haarp/haarp.conf:

		CACHEDIR <dir>	
		MYSQL_USER <user_mysql>
		MYSQL_PASS <pass_mysql>


* Copy & page this, in the file of configuration: squid.conf (install squid)

		acl haarp_lst url_regex -i "/etc/haarp/haarp.lst"
		cache deny haarp_lst
		cache_peer 127.0.0.1 parent 8080 0 proxy-only no-digest
		dead_peer_timeout 2 seconds
		cache_peer_access 127.0.0.1 allow haarp_lst
		cache_peer_access 127.0.0.1 deny all


You can change the address 127.0.0.1 for the IP of you server haarp.

* On crontab add the line:
	
		50 12     * * *   root    /etc/init.d/haarpclean

 
List of Plugins
--------------

* Currently Haarp supports:


	Youtube (https not supported),
	DoubleClick (Block of ADS),
	Googlevideo,
	Dailymotion,
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
	5min,
	Tumblr,
	Facebook (https not supported);
	
	4shared,
	Mediafire,
	Etrustdownloads,
	Ziddu;
	
	Issuu,
	Ytimg,
	Submanga,
	Blogspot;
	
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
	YouPorn,
	Tube8;
	
	Juegosdiarios,
	Juegosjuegos;

	SpeedTest.
	
Configure your file /etc/haarp/haarp.lst to disable or enable the plugins.	

	
Changelog
---------

__Version 1.2.1__
_Thanks to Samuel Espinoza, Oscar Vaquero y Fernando Maniglia._

2015/05/29

* Fix concurrent caching of files.
* Update haarpclean script.
* Update some plugins: Youtube, Dailymotion, 4shared, Issuu, Avgate, Submanga, YourPorn, PornHub, Xvideos, PornTube, Tube8. 
* Files of youtube are cached with the watch ID from the URL.

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
* Multidisc.
* Blocking advertising to Youtube.
* Cache of videos in html5 for Youtube.

Fork created by

Manolo Canales

kei.haarpcache@gmail.com

http://perufw.wordpress.com

<br/>
<div align="center">
<p><h6>If you find this code useful, or wish to fund further development,
you can use PayPal to donate to the HaarpCache project:</h6>
<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&amp;hosted_button_id=QYCCSYYGW52QU"><img src="https://www.paypalobjects.com/en_US/i/btn/btn_donateCC_LG.gif"></a>
</div>
    
  </body>
</html>
