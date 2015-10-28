CREATE DATABASE if not exists haarp;
USE haarp;

CREATE TABLE IF NOT EXISTS `haarp` (
  `id` bigint NOT NULL auto_increment,
  `file` varchar(767) NOT NULL,
  `domain` varchar(255) NOT NULL,
  `size` int(10) unsigned NOT NULL default '0',
  `modified` datetime NOT NULL,
  `downloaded` datetime NOT NULL,
  `bytes_requested` bigint unsigned NOT NULL default 0,
  `last_request` datetime NOT NULL,
  `rg` varchar(5000) NOT NULL DEFAULT '',
  `pos` varchar(2500) NOT NULL DEFAULT '',
  `filesize` int(11) DEFAULT '0',
  `np` int(11) DEFAULT '0',
  `expires` datetime NOT NULL DEFAULT '2050-10-10 00:00:00',
  `prob` double NOT NULL DEFAULT '0',
  `deleted` tinyint(1) NOT NULL default '0',
  `static` tinyint(1) NOT NULL default '0',
  `users` varchar(1000) NOT NULL default '',  
  `file_used` tinyint(1) NOT NULL default '0',
    PRIMARY KEY  (`id`),
    UNIQUE KEY `file_domain` (`file`,`domain`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


grant all privileges on haarp.* to haarp@localhost identified by 'haarpcache';

