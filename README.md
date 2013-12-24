Introduction
------------
Proxy Haarpcache es un tenedor de Thundercache.

Lo resaltante en el sistema haarpcache, es que este permite almacenar los archivos en forma inteligente. 

Es decir almacena cada pedazo del video o archivo juntandolo todo en un único recurso en el disco, sin importar que estas partes hayan venido incompletas o en desorden. Además esto hace que 
cuando se pida de nuevo el mismo archivo; se despachen las partes que están en el disco y las que no se encuentran en el, se toman desde la red, pero al final las partes faltantes del archivo que se descargan se siguen almacenando en el mismo archivo; completando así el archivo original. 
 

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
