#!/bin/sh

app="dz"
system=`uname -o`
pid=`ps aux | grep ${app} | grep -v grep | awk '{ print $2 }'`
echo $pid
if [ "$pid" != "" ];then
	kill $pid
fi

GDK_BACKEND=x11 ../build-dz-Desktop-Debug/${app} &
