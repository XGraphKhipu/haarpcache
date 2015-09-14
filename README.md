# HaarpCache

## Maintainer
- [Kei Kurono] (https://perufw.wordpress.com) <<kei.haarpcache@gmail.com>>

## Table of contents
- [Introduction](#introduction)
- [Installing](#installing)
- [Reinstall or Update](#reinstall-or-update)
- [Monitoring Logs](#monitoring-logs)
- [List of Plugins](#list-of-plugins)
- [Changelog](#changelog)

## Introduction

Proxy HaarpCache is a fork of ThunderCache.

HaarpCache is a robust static and dynamic cache that provides support to the DASH technology.

## Installing

	su
	apt-get update 
	apt-get upgrade
	apt-get install build-essential mysql-server mysql-client php5 apache2 php5-mysql \
	libblkid-dev libcurl4-gnutls-dev libmysqlclient15-dev libapache2-mod-auth-mysql \
	libapache2-mod-php5 sharutils curl autoconf squid3
	cd /usr/src/
	git clone https://github.com/keikurono/haarpcache.git
	cd haarpcache
	./configure
	make
	make install
	mysql -u root -p < haarp.sql
	cd /etc/init.d
	update-rc.d haarp defaults 98
	
	
* Create a user for database of haarpcache:
	
		mysql -u root -p
		grant all privileges on haarp.* to haarp_user@localhost identified by 'haarpcache_password';
		
	Change the username ('haarp_user') and password ('haarpcache_password') for which you want.
	

* Configure your /etc/haarp/haarp.conf
		
		CACHEDIR <dir_1>|<dir_2>|<dir_3> ...	
		MYSQL_USER <haarp_user>
		MYSQL_PASS <haarpcache_password>

	`<dir_1>`, `<dir_2>`,... are the directories where cache files will be stored, for default '/haarp/'.
	`<haarp_user>` and `<haarpcache_password>` are the user and password for the database of haarpcache defined above.

* Copy and paste the line above at the end of the file 'squid.conf' (or 'squid3.conf' which is in operation):

		acl haarp_lst url_regex -i "/etc/haarp/haarp.lst"
		cache deny haarp_lst
		cache_peer 127.0.0.1 parent 8080 0 proxy-only no-digest
		dead_peer_timeout 2 seconds
		cache_peer_access 127.0.0.1 allow haarp_lst
		cache_peer_access 127.0.0.1 deny all
		
* Below the line of the `http_port` configuration, add: 

		acl google url_regex -i (googlevideo.com|www.youtube.com)
		acl iphone browser -i regexp (iPhone|iPad)
		acl BB browser -i regexp (BlackBerry|PlayBook)
		acl Winphone browser -i regexp (Windows.*Phone|Trident|IEMobile)
		acl Android browser -i regexp Android
		request_header_access User-Agent deny google !iphone !BB !Winphone !Android
		request_header_replace User-Agent Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)
	with this we avoid the redirects `HTTP` to `HTTPS` in `youtube.com`. Only works for PC's, and when you enter to `http://www.youtube.com` from the address bar (of your browser) or from `www.google.com`.

* On squid.conf Comment the line:
		
		#hierarchy_stoplist cgi-bin ?

* On crontab add the line:
	
		0 */1     * * *   root    /etc/init.d/haarpclean

* View reports of your cache system:

		cp /usr/src/haarpcache/etc/haarp/haarp.php /var/www/
	
	Change the line 24 of the file haarp.php with your user and password for your database haarpcache.
	
		 if (!( $db = new PDO('mysql:host=localhost;dbname=haarp', 'haarp_user','haarpcache_password') ) )

* Your viewer:

		http://<IP_Your_Server>/haarp.php

* Security:
	
		iptables -I INPUT 1 -i <eth_wan_1> -m tcp -p tcp --dport 8080 -m state --state NEW -j DROP
		iptables -I INPUT 1 -i <eth_wan_2> -m tcp -p tcp --dport 8080 -m state --state NEW -j DROP
		...
	
	The port 8080 is of your haarpcache server. You can change this editing the file /etc/haarp/haarp.conf. 

* Avoid the `QUIC` protocol connections on `www.youtube.com`

		iptables -A FORWARD -i <eth_lan> -p udp -m udp --dport 80 -j REJECT --reject-with icmp-port-unreachable
		iptables -A FORWARD -i <eth_lan> -p udp -m udp --dport 443 -j REJECT --reject-with icmp-port-unreachable
		
	add the following line in `squid.conf`:
	
		# Disable alternate protocols
		reply_header_access Alternate-Protocol deny all
		
* All iptables rules should be placed in a *.sh file to be executed whenever the computer is started.

* Finally:
 	
		/etc/init.d/haarp restart
		squid -k reconfigure
		
* Handling QoS can be done by making a marking of packages based on the search for the string "X-Cache: HIT from HAARP" or "HAARP: HIT from" in the packet header. Thus a possible handling may carried out as follows:

		IF_LAN=eth0
		MAX_DOWN=1300kbps
		MIN_CACHE_DOWN=1000kbps
		MAX_CACHE_DOWN=1100kbps
		 
		iptables -A OUTPUT -t mangle -o $IF_LAN -p tcp -m string --string "X-Cache: HIT from Haarp" --algo kmp -j MARK --set-mark 666
		 
		tc qdisc add dev $IF_LAN root handle 1:0 htb default 10 r2q 15
		tc class add dev $IF_LAN parent 1:0 classid 1:1 htb rate $MAX_DOWN ceil $MAX_DOWN
		tc class add dev $IF_LAN parent 1:1 classid 1:66 htb rate $MIN_CACHE_DOWN ceil $MAX_CACHE_DOWN
		tc qdisc add dev $IF_LAN parent 1:66 handle 66:0 sfq perturb 30
		tc filter add dev $IF_LAN protocol ip parent 1:0 handle 666 fw classid 1:66
		

## Reinstall or Update

For a common update:
	
		su
		cd /usr/src/
		git clone https://github.com/keikurono/haarpcache.git
		cd haarpcache
		./configure
		make
		make install
		mysql -u root -p < haarp.sql
		cp -b etc/haarp/haarp.lst /etc/haarp/haarp.lst
		/etc/init.d/haarp restart
		squid -k reconfigure
	
## Monitoring Logs

Depending on the location of the logs defined in the file `/etc/haarp/haarp.conf`:

		tail -f /var/log/haarp/access.log
		tail -f /var/log/haarp/error.log

Level information in the logs: change the `LOGLEVEL` on `/etc/haarp/haarp.conf`.

## List of Plugins

* Currently Haarp supports:


	Youtube (https not supported),
	Netflix (Video and Images),
	DoubleClick (Block of ADS),
	Googlevideo,
	Dailymotion,
	AOL,
	Vimeo,
	Metacafe,
	Modovideo,
	Bitgravity,
	Globo,
	Terra,
	Viddler,
	Justin,
	Edgecastcdn,
	Youku,
	Wrzuta,
	5min,
	Viddler,
	Tumblr,
	Facebook (https not supported);
	
	Bomusica,
	Bullafina,
	Coqui,
	Cutmu,
	Domusica,
	Domusicas,
	Dungeonbusters,
	Goobeo,
	Gooveo,
	Miniclip,
	Molenillo,
	Mp3-Buscador,
	Musicaveo,
	Ning,
	Nofeehost,
	Sonicomusica,
	Turner,
	Vevos;
	
	4shared,
	Mediafire,
	Etrustdownloads,
	Ziddu;
	
	Akamaihd,
	Amazonaws,
	Blogspot,
	Imageshack,
	Issuu,
	Mercadolibre,
	Submanga,
	Ytimg;
	
	Avast,
	Avgate,
	Eset,
	Mcafee,
	Avira,
	Bitdefender;

	Windowsupdate,
	Adobe;
	
	Porntube,
	Xvideos,	
	Pornhub,
	Redtube,
	YouPorn,
	Serviporno,
	Tube8;
	
	Appspot,
	Cloudfront,
	Juegosdiarios,
	Juegosjuegos,
	Netpocket,
	Nordeus,
	Popcap,
	Socialpointgames,
	Telaxo,
	Tetrisfb,
	Vostucdn,
	Wooga,
	Zgncdn;
	
	Disneylatino,
	Friv,
	Geewa,
	Hulkshare,
	Kixeye,
	Llnwd,
	Maguinamotors,
	Manabar;
	
	SpeedTest (some servers);
	

Configure your file /etc/haarp/haarp.lst to disable or enable the plugins.	

	
## Changelog

__Version 1.5__

2015/09/14

* Storing the address IP of the user who uses the cache.
* Adding user history for each file in cache.
* Update the regexp for some plugins in haarp.lst

__Version 1.4.1__
_Thanks to Hernan Dario Nacimiento._

2015/07/31

* Update the plugins for edgecastcdn, socialpointgames and akamaihd.
* Update plugin of Youtube: not cached of videos on live.
* Using the parameter ‘clen’ of the url from youtube as file size.
* Update the HaarpClean program.
* Fixed the problem of bad replacement string in the SQL requests.
* Update the file of configuration for logrotate.
* Fix problem: ‘general protection ip:7f39b6cdd44e sp:7ffc278771f0 error:0 in libmysqlclient.so.18.0.0‘.

__Version 1.4__

2015/07/07

* Adding 50 new plugins.
* Improved support for DASH technology.
* Update the file of configuration for logrotate.
* Fix problem: 'segfault at 6 ip 00007f850c2cef36 sp 00007fff19f22d00 error 4 in libmysqlclient.so.18.1.0'.
* Fix problem: 'Error in /usr/local/sbin/haarp: free(): invalid pointer: 0x00007fb5e166d618'.

__Version 1.3__

2015/06/13

* Fix Problem with concurrent caching of files.
* Update the plugin Youtube. 

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
