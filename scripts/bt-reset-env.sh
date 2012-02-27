#!/bin/sh

# BT Stack and device stop
/usr/etc/bluetooth/bt-stack-down.sh

killall -9 hciattach

# Remove BT files and setting
rm -rf /opt/data/bluetooth/.bt_paired
rm -rf /var/lib/bluetooth/*

KDB=`mount | grep libsqlfs_mount | awk '{print $3}'`
if [ -d $KDB/bluetooth ]
then
	rm -rf $KDB/bluetooth/*
fi
if [ -d $KDB/user/bluetooth ]
then
	rm -rf $KDB/user/bluetooth/*
fi

# Remove BT shared memory
list=`ipcs -m | awk '$1==0x0001000 {print $2}'`
for i in $list
do
	ipcrm -m $i
done
ipcs -m | grep "0x00001000" | awk '{ print $2 }'

