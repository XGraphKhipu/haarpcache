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

Copy & page this, in the file of configuration: squid.conf (install squid)
acl haarp\_lst\ url\_regex -i "/etc/haarp/haarp.lst"
cache deny haarp\_lst
cache\_peer 127.0.0.1 parent 8080 0 proxy-only no-digest
dead\_peer\_timeout 2 seconds
cache\_peer\_access 127.0.0.1 allow haarp\_lst
cache\_peer\_access 127.0.0.1 deny all

You can change the address 127.0.0.1 for the IP of you server haarp.

*To clean: make clean

Changelog
---------
__Vesion 1.1__

2013/02/09

* Cache dinamico e inteligente.
* Plugins editados, para funcionamiento inteligente.
* Multi disco
* Bloqueo de publicidad en www.youtube.com.
* Cache de Youtube en html5.

Fork creado por

Manolo Canales

kei.haarpcache@gmail.com

http://perufw.wordpress.com
