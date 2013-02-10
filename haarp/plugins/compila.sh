#!/bin/bash
for i in *.cpp ; do
  i2=`echo $i|sed -e "s/cpp/so/g"`
  echo "Compilando $i"
  g++ -I. -fPIC -shared -o $i2 $i 
done
