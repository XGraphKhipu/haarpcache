Introduction
------------
Proxy Haarpcache es un tenedor de Thundercache.

Lo resaltante en el sistema haarpcache, es que este permite almacenar los archivos en forma inteligente. 

Es decir almacena cada pedaso de video o archivo juntandolo todo en un único recurso en el disco, sin importar que estas partes hayan venido incompletas o en desorden. Además esto hace que 
cuando se pida de nuevo el mismo archivo; se despachen las partes que entan en el disco y las que no se encuentran en el, se toman desde la red, pero al final las partes faltantes del archivo que se descargan se siguen almacenando en el mismo archivo; completando así el achivo original. 
 

Building
--------
* Type _./configure_ and then _make_
* To clean, type _make clean_
* To remove, type _make uninstall_


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

Manolo Canales C.

kei.haarpcache@gmail.com

http://perufw.wordpress.com
