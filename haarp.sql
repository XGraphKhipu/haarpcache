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
  `rg` text character set latin1 default NULL,
  `pos` text character set latin1 default NULL,
  `filesize` int(11) DEFAULT '0',
  `np` int(11) DEFAULT '0',
  `deleted` tinyint(1) NOT NULL default '0',
  `static` tinyint(1) NOT NULL default '0',
  `users` varchar(1000) NOT NULL default '',  
  `file_used` tinyint(1) NOT NULL default '0',
    PRIMARY KEY  (`id`),
    UNIQUE KEY `file_domain` (`file`,`domain`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

delimiter $$
DROP PROCEDURE IF EXISTS UpdateColum $$
CREATE PROCEDURE UpdateColum()
BEGIN
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'file_used') THEN
	ALTER TABLE haarp ADD COLUMN file_used tinyint(1) NOT NULL DEFAULT '0';
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'users') THEN
        ALTER TABLE haarp ADD COLUMN users varchar(1000) NOT NULL DEFAULT '';
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'bytes_requested') THEN
        ALTER TABLE haarp CHANGE requested bytes_requested bigint unsigned NOT NULL DEFAULT 0;
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'prob') THEN
        ALTER TABLE haarp ADD prob double NOT NULL DEFAULT 0 AFTER np;
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'expires') THEN
        ALTER TABLE haarp ADD expires datetime NOT NULL DEFAULT "2050-10-10 00:00:00" AFTER np;
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'rg' AND DATA_TYPE='text') THEN
        ALTER TABLE haarp MODIFY COLUMN rg text character set latin1 default null;
end if;
IF NOT EXISTS (SELECT * from information_schema.COLUMNS where TABLE_SCHEMA = 'haarp' AND TABLE_NAME = 'haarp' AND COLUMN_NAME = 'pos' AND DATA_TYPE='text') THEN
        ALTER TABLE haarp MODIFY COLUMN pos text character set latin1 default NULL;
end if;

END
$$
delimiter ;

CALL UpdateColum(); 
