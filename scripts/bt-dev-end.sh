#!/bin/sh

#
# Script for stopping Broadcom UART Bluetooth stack
#

PGREP="/usr/bin/pgrep"

# Device down
/usr/bin/hciconfig hci0 down

# Turn off Bluetooth Chip
/usr/sbin/rfkill block bluetooth

#/usr/bin/killall hciattach
# Do NOT use killall due to smack
hciattach_pid=`${PGREP} hciattach`
kill $hciattach_pid

#if [ -e /sys/class/gpio/gpio17/value ]
#then
#	# Reset BT chip
#	echo 0 > /sys/class/gpio/gpio17/value
#	sleep 0.1
#	echo 1 > /sys/class/gpio/gpio17/value
#fi

