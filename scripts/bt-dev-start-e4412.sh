#!/bin/sh

# Script for registering Broadcom UART BT device
BT_UART_DEVICE=/dev/ttySAC0
BT_CHIP_TYPE=bcm2035
BCM_TOOL=/usr/bin/bcmtool_4330b1

BT_PLATFORM_DEFAULT_HCI_NAME="TIZEN-Mobile"
UART_SPEED=3000000

# defult firmware
# SEMCO external LNA, I2S slave
BCM_FIRMWARE=BCM4334B0_002.001.013.0079.0081.hcd

REVISION_NUM=`grep Revision /proc/cpuinfo | awk "{print \\$3}"`
REVISION_HIGH=`echo $REVISION_NUM| cut -c1-2`
REVISION_LOW=`echo $REVISION_NUM| cut -c3-`

#HARDWARE=`grep Hardware /proc/cpuinfo | awk "{print \\$3}"`

echo $BCM_FIRMWARE

if [ ! -e "$BT_UART_DEVICE" ]
then
	mknod $BT_UART_DEVICE c 204 64
fi

if [ ! -e /csa/bluetooth/.bd_addr ]
then
	# Set BT address
	/usr/bin/setbd
fi

rfkill unblock bluetooth

echo "Check for Bluetooth device status"
if (/usr/sbin/hciconfig | grep hci); then
	echo "Bluetooth device is UP"
	/usr/sbin/hciconfig hci0 up
else
	echo "Bluetooth device is DOWN"
	echo "Registering Bluetooth device"

	$BCM_TOOL $BT_UART_DEVICE -FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED -ADDR=/csa/bluetooth/.bd_addr -SETSCO=0,0,0,0,0,0,0,3,3,0 -LP > /dev/null 2>&1

	# Attaching Broadcom device
	if (/usr/sbin/hciattach $BT_UART_DEVICE -s $UART_SPEED $BT_CHIP_TYPE $UART_SPEED flow); then
		sleep 0.1
		/usr/sbin/hciconfig hci0 up
		/usr/sbin/hciconfig hci0 name $BT_PLATFORM_DEFAULT_HCI_NAME
		/usr/sbin/hciconfig hci0 sspmode 1
		echo "HCIATTACH success"
	else
		echo "HCIATTACH failed"
		rfkill block bluetooth
	fi
fi

#/usr/sbin/hciconfig hci0 down

