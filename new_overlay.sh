#!/bin/bash
apt download $1
mv $1*.deb $1.deb
info=`apt show $1 2>/dev/null`
desc=`echo "$info" | grep Description | sed 's/.*: //'`
version=`echo "$info" | grep Version | sed 's/.*: //'`
depends=`echo "$info" | grep Depends | sed 's/.*: //' | sed -e's/ (/\":\"/g' | sed -e 's/)/"/g' | sed -e 's/, /,\n        "/g'`
IFS=","
echo "{" > $1.json
echo "    \"description\":\"$desc\"," >> $1.json
echo "    \"version\":\"$version\"," >> $1.json
echo "    \"depends\": {" >> $1.json
echo "        \"$depends" >> $1.json
echo "    }" >> $1.json
echo "}" >> $1.json

dpkg-deb -R $1.deb deb
rm $1.deb
cd deb
tree
bash
tar --owner=0 --group=0 -cvzf ../$1.tar.gz *
cd ..
rm -r deb