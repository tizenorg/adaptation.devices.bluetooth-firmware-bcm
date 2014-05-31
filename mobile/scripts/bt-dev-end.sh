#!/bin/sh

#
# Script for stopping Broadcom UART Bluetooth stack
#

# Device down
/usr/sbin/hciconfig hci0 down

# OMAP4
REVISION_NUM=`grep Revision /proc/cpuinfo | awk "{print \\$3}"`
if [ $REVISION_NUM == "0006" ]; then
	rmmod bt_drv.ko
	rmmod st_drv.ko
	sleep 1
	killall uim_rfkill
	exit 0
fi

# Turn off Bluetooth Chip
rfkill block bluetooth

killall hciattach

#if [ -e /sys/class/gpio/gpio17/value ]
#then
#	# Reset BT chip
#	echo 0 > /sys/class/gpio/gpio17/value
#	sleep 0.1
#	echo 1 > /sys/class/gpio/gpio17/value
#fi

