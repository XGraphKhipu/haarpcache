/*
 * (c) Copyright 2013 Manolo Canales <kei.haarpcache@gmail.com>
 * Some Rights Reserved.
 *
 * @autor Manolo Canales <kei.haarpcache@gmail.com>
 */

#ifndef HAARP_CLEAN_H
#define HAARP_CLEAN_H
#include <iostream>
#include <string>

using namespace std;

string sqlconv2(string sql);
void SearchReplace2(string &source, string search, string replace);

#endif
