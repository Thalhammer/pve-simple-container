#!/bin/bash
DEPS=`readelf -d $1 2>/dev/null | grep NEEDED | awk '{print $5}' | sed "s/\[//g" | sed "s/\]//g" | sort -u | while read n;do dpkg-query -S $n; done | sed 's/^\([^:]\+\):.*$/\1/' | sort | uniq`
dpkg -l | grep ii | awk '{print $2}' | cut -d: -f1,1 | sort -u > /tmp/installed$$.txt
while IFS= read -r line
do
	if grep -Fxq "$line" /tmp/installed$$.txt
	then
		echo $line;
	fi
done <<< "$DEPS"
rm -f /tmp/installed$$.txt