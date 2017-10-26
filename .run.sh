#!/bin/sh

app="dz"
spp="vwsimulator"
system=`uname -o`
pid=`ps aux | grep ${app} | grep -v grep | awk '{ print $2 }'`
if [ "$pid" != "" ];then
	kill $pid
fi
has=`lsmod | grep tty0tty | wc -l`
if [ "${has}" == "0" ] && [ -d "~/Project/tty0tty" ];then
	sudo insmod ~/Project/tty0tty/module/tty0tty.ko
	sudo chmod a+r /dev/tnt*
	sudo chmod a+w /dev/tnt*
	sudo chown gino:gino /dev/tnt*
fi
rm -rf vw_knob_test_*

pid=`ps aux | grep ${spp} | grep -v grep | awk '{ print $2 }'`
if [ "$pid" == "" ] && [ -f ~/${spp} ];then
	~/${spp} &
fi

sudo chown gino:gino /dev/tnt*
sudo chown gino:gino /dev/ttyUSB*
GDK_BACKEND=x11 ../build-dz-Desktop-Debug/${app} &
