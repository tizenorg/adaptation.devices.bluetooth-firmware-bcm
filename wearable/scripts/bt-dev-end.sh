#!/bin/sh

#
# Script for stopping Broadcom UART Bluetooth stack
#

# Device down
/usr/sbin/hciconfig hci0 down

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

