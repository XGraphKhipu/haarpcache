#!/bin/bash
# ---------------------------------------------------------------------------
#  Copyright 2015, Kei Kurono <kei.haarpcache@gmail.com>
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#  
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#  


echo -e "\e[5m__        __   _                          
\ \      / /__| | ___ ___  _ __ ___   ___ 
 \ \ /\ / / _ \ |/ __/ _ \| '_ \` _ \ / _ \\
  \ V  V /  __/ | (_| (_) | | | | | |  __/
   \_/\_/ \___|_|\___\___/|_| |_| |_|\___|
 _ 
| |_ ___  
| __/ _ \ 
| || (_) |
 \__\___/
 _   _                         ____           _          
| | | | __ _  __ _ _ __ _ __  / ___|__ _  ___| |__   ___ 
| |_| |/ _\` |/ _\` | '__| '_ \| |   / _\` |/ __| '_ \ / _ \\
|  _  | (_| | (_| | |  | |_) | |__| (_| | (__| | | |  __/
|_| |_|\__,_|\__,_|_|  | .__/ \____\__,_|\___|_| |_|\___|
                       |_|                               \e[25m"
echo  "Buscando las tarjetas de red ...."
sleep 0.8
function getInterfacesIP() {
	echo "$(ip -o -4 add | egrep -A0 "(eth[0-9]*|enp[0-9]*s[0-9]*)"  | awk '{print $2, $4}' | cut -d '/' -f1)"
}
function mask2cidr() {
    nbits=0
    IFS=.
    for dec in $1 ; do
        case $dec in
            255) let nbits+=8;;
            254) let nbits+=7;;
            252) let nbits+=6;;
            248) let nbits+=5;;
            240) let nbits+=4;;
            224) let nbits+=3;;
            192) let nbits+=2;;
            128) let nbits+=1;;
            0);;
            *) echo "Error: $dec is not recognised"; exit 1
        esac
    done
    echo "$nbits"
}

RED='\e[31m';
NOR='\e[0m';
#~ 
interIP=($(getInterfacesIP))
#~ 
name=""
lenInter=${#interIP[@]};
if [[ $lenInter -eq 0 ]]; then
	echo -e "${RED}Interfaces no encontrados!, configure sus interfaces!.${NOR}";
	exit
fi
index=-2;
if [ $[lenInter%2] -eq 1 ]; then
	lenInter=$[lenInter-1];
fi
while [[ $name != "yes" ]]; do
	index=$[$[index+2]%lenInter];
	echo -n -e "${RED}Su red LAN se encuentra en ${interIP[$index]} -${interIP[$[index+1]]}-${NOR} (yes/no)?: ";
	read name;
done
sleep 0.5
#~ 
ETHLAN=${interIP[$index]};
IPLAN=${interIP[$[index+1]]};
#~ 
NETMASK_LAN="127.0.0.0"
CIDR_LAN="24"
#~ 
network_mask=($(netstat -nr | grep -E " $ETHLAN($| )" | grep -v "^[0.0.0.0]" | awk '{print $1, $3}'))
if [[ "${#network_mask[@]}" != "2" ]]; then
	echo ""
	echo -e "${RED}ADVERTENCIA: network y netmask no configurados para $ETHLAN (${interIP[$[index+1]]})";
	echo -n -e "${NOR}Pulse enter para continuar ... ";
	read tmp
else 
	NETMASK_LAN=${network_mask[0]}
	CIDR_LAN=$(mask2cidr ${network_mask[1]})
fi
#
clear
apt-get update 
apt-get upgrade
apt-get install -y aptitude
aptitude install -y build-essential mysql-server mysql-client php5 apache2 php5-mysql libblkid-dev libcurl4-gnutls-dev libmysqlclient15-dev libapache2-mod-auth-mysql libapache2-mod-php5 sharutils curl autoconf squid3 git g++-4.4 bind9 dnsutils

mv /etc/squid3/squid.conf "/etc/squid3/squid.conf.backup_$(date +%Y%m%d)"
touch /etc/squid3/squid.conf
echo "#===========================================================#
#          Generate by the installer of HaarpCache          #
#===========================================================#
http_port 3128 intercept
visible_hostname haarp.cache

dns_nameservers 8.8.8.8 8.8.4.4
dns_retransmit_interval 5 seconds
dns_timeout 2 minutes

acl localnet src $NETMASK_LAN/$CIDR_LAN # Your LAN
acl localnet src fc00::/7       # RFC 4193 local private network range
acl localnet src fe80::/10      # RFC 4291 link-local (directly plugged) machines

acl SSL_ports port 443
acl Safe_ports port 80          # http
acl Safe_ports port 21          # ftp
acl Safe_ports port 443         # https
acl Safe_ports port 70          # gopher
acl Safe_ports port 210         # wais
acl Safe_ports port 1025-65535  # unregistered ports
acl Safe_ports port 280         # http-mgmt
acl Safe_ports port 488         # gss-http
acl Safe_ports port 591         # filemaker
acl Safe_ports port 777         # multiling http
acl CONNECT method CONNECT

http_access deny !Safe_ports
http_access deny CONNECT !SSL_ports

http_access allow manager localhost
http_access deny manager 
http_access allow localnet
http_access allow localhost
#################################
acl google url_regex -i (googlevideo.com|www.youtube.com)
acl iphone browser -i regexp (iPhone|iPad)
acl BB browser -i regexp (BlackBerry|PlayBook)
acl Winphone browser -i regexp (Windows.*Phone|Trident|IEMobile)
acl Android browser -i regexp Android
request_header_access User-Agent deny google !iphone !BB !Winphone !Android
request_header_replace User-Agent Mozilla/5.0 (compatible; Googlebot/2.1; +http://www.google.com/bot.html)
#################################
quick_abort_min 0 KB
quick_abort_max 0 KB
quick_abort_pct 100
coredump_dir /var/spool/squid3
logfile_rotate 5
access_log /var/log/squid3/access.log
access_log /var/log/squid3/error.log
cache_store_log none" > /etc/squid3/squid.conf
/etc/init.d/squid3 restart
clear
echo -e "${RED}*///////////////======= Instalando HaarpCache =======///////////////*${NOR}"
cd /usr/src/
git clone https://github.com/keikurono/haarpcache.git
cd haarpcache
./configure CXX=g++-4.4
make
make install
echo -e "${RED}Ingrese la contraseña root de Mysql${NOR}"
while true; do
	mysql -u root -p < haarp2.sql
	if [ $? -eq 0 ]; then
		break;
	fi
done
mkdir /var/www/html/ 2>/dev/null
yes | cp -u etc/haarp/haarp.php /var/www/  2>/dev/null
yes | cp -u etc/haarp/haarp.php /var/www/html/ 2>/dev/null
cd /etc/init.d
update-rc.d haarp defaults 98
echo "#------------- HaarpCache ------------------------
acl haarp_lst url_regex -i \"/etc/haarp/haarp.lst\"
cache deny haarp_lst
cache_peer 127.0.0.1 parent 8080 0 proxy-only no-digest
dead_peer_timeout 2 seconds
cache_peer_access 127.0.0.1 allow haarp_lst
cache_peer_access 127.0.0.1 deny all" >> /etc/squid3/squid.conf
#
echo "# HaarpClean: 
0 */1     * * *   root    /etc/init.d/haarpclean" >> /etc/crontab
clear
# restart servers
/etc/init.d/haarp restart
/etc/init.d/apache2 restart
squid3 -k reconfigure 
# DNS configuration
mv /etc/resolv.conf "/etc/resolv.conf.backup_$(date +%Y%m%d)"
touch /etc/resolv.conf
echo "nameserver 8.8.8.8
nameserver 8.8.4.4" >> /etc/resolv.conf
mv /etc/bind/named.conf.options "/etc/bind/named.conf.backup_$(date +%Y%m%d)"
touch /etc/bind/named.conf.options
echo "options {
        directory \"/var/cache/bind\";
        forwarders {
                8.8.8.8;
                8.8.4.4;
        };
        auth-nxdomain no;
        listen-on-v6 { any; };
};" >> /etc/bind/named.conf.options

# Install Haarp-Viewer v1.x
aptitude install unzip
cd /var/www/html/
wget http://extjs.cachefly.net/ext-3.4.0.zip 
unzip  ext-3.4.0.zip
ln -s ext-3.4.0 ext
ln -s html/ext-3.4.0 ../ext

# Install LibCGI
cd /usr/src
aptitude install autoconf git
git clone git://github.com/keikurono/libcgi.git
cd libcgi
./autogen.sh
./configure --prefix=/usr
make
make install    

# Install HaarpViewer
cd /usr/src/
git clone git://github.com/keikurono/haarp-viewer.git
cd haarp-viewer
cd src/
make

cd ..
rm -f /usr/lib/cgi-bin/report.cgi /usr/lib/cgi-bin/haarp.cgi 2>/dev/null
rm -f /var/www/cgi-bin/report.cgi /var/www/cgi-bin/haarp.cgi 2>/dev/null
cp src/*.cgi /usr/lib/cgi-bin/ 2>/dev/null
cp src/*.cgi /var/www/cgi-bin/ 2>/dev/null
yes | cp hc.html /var/www/html/ 2>/dev/null
yes | cp hc.html /var/www/  2>/dev/null
yes | cp -r images /var/www/html/ 2>/dev/null
yes | cp -r images /var/www/ 2>/dev/null
touch /var/log/haarp/webaccess.log
chown www-data:www-data /var/log/haarp/webaccess.log
echo "ACCESSWEBLOG /var/log/haarp/webaccess.log
FORWARDED_IP true" >> /etc/haarp/haarp.conf
# --- Apache to port 88 ---
sed -i 's/Listen.*[0-9]*/Listen 88/g' /etc/apache2/ports.conf
sed -i 's/NameVirtualHost.*:[0-9]*/NameVirtualHost *:88/g' /etc/apache2/ports.conf
sed -i 's/VirtualHost.*:[0-9]*>/VirtualHost *:88>/g'  /etc/apache2/sites-enabled/000-default.conf
# -- restart service apache2 --
a2enmod cgi 2>/dev/null
service apache2 restart 2>/dev/null
# -- create script rc.local --
mv /etc/rc.local "/etc/rc.local.backup_$(date +%Y%m%d)"
touch /etc/rc.local
chmod +x /etc/rc.local
# --- Firewall Conf ---
clear
iptables -t nat -F
iptables -t mangle -F
iptables -F
echo "#!/bin/bash
1 > /proc/sys/net/ipv4/ip_forward
iptables -A FORWARD -i $ETHLAN -p udp -m udp --dport 80  -j REJECT --reject-with icmp-port-unreachable
iptables -A FORWARD -i $ETHLAN -p udp -m udp --dport 443 -j REJECT --reject-with icmp-port-unreachable
iptables -A PREROUTING -t nat -i $ETHLAN -p tcp -m tcp --dport 80  -j REDIRECT --to-ports 3128" > /etc/rc.local

echo 1 > /proc/sys/net/ipv4/ip_forward
iptables -A FORWARD -i $ETHLAN -p udp -m udp --dport 80  -j REJECT --reject-with icmp-port-unreachable
iptables -A FORWARD -i $ETHLAN -p udp -m udp --dport 443 -j REJECT --reject-with icmp-port-unreachable
iptables -A PREROUTING -t nat -i $ETHLAN -p tcp -m tcp --dport 80  -j REDIRECT --to-ports 3128
# Masquerade of others interfaces - WAN?.
index=0;
while [[ $index < $lenInter ]]; do
	if [ "${interIP[$index]}" != "$ETHLAN" ]; then
		ethWan=${interIP[$index]};
		echo "iptables -A POSTROUTING -t nat -o $ethWan -j MASQUERADE
iptables -A INPUT -i $ethWan -p tcp -m tcp --dport 8080 -m state --state NEW -j DROP" >> /etc/rc.local
		iptables -A POSTROUTING -t nat -o $ethWan -j MASQUERADE
		iptables -A INPUT -i $ethWan -p tcp -m tcp --dport 8080 -m state --state NEW -j DROP
	fi 
	let index+=2;
done
echo "exit 0" >> /etc/rc.local
#~ 
echo -e "        /*       _\|/_
                 (o o)
         +----oOO-{_}-OOo------------------------------------------------------------+
         |                                                                           |
         | Finish?, access to HaarpViewer: \e[4mhttp:///$IPLAN:88/cgi-bin/haarp.cgi\e[24m 
         +--------------------------------------------------------------------------*/"


