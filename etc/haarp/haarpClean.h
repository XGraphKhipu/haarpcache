#ifndef HAARP_CLEAN_H
#define HAARP_CLEAN_H
#include <iostream>
#include <string>

using namespace std;

string sqlconv2(string sql);
void SearchReplace2(string &source, string search, string replace);

#endif
