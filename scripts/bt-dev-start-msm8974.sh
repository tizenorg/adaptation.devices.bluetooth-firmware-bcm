#!/bin/sh

#
# Script for registering Broadcom UART BT device
#
BT_UART_DEVICE=/dev/ttyHS0
BT_CHIP_TYPE=bcm2035
BCM_TOOL=/usr/bin/bcmtool

BT_ADDR=/csa/bluetooth/.bd_addr

UART_SPEED=3000000

# defult firmware
# SEMCO external LNA, I2S slave
# For host name REDWOOD
BCM_4335_FIRMWARE=BCM4335B0_002.001.006.0233.0234_ORC_RedWood.hcd
BCM_4339_FIRMWARE=BCM4339_003.001.009.0030.0122_ORC_RedWood.hcd

HOST_NAME=`grep Hardware /proc/cpuinfo | awk "{print \\$3}"`
REVISION=`grep Revision /proc/cpuinfo | awk "{print \\$3}"`

BCM_FIRMWARE=$BCM_4339_FIRMWARE

if [ "$HOST_NAME" == "RedwoodLTE_MSM_EUR" ]; then
	if [ "$REVISION" == "0000" ] || [ "$REVISION" == "0001" ] || [ "$REVISION" == "0004" ]; then
		BCM_FIRMWARE=$BCM_4335_FIRMWARE
	fi
fi

echo $BCM_FIRMWARE

# Set BT address: This will internally check for the file presence
/usr/bin/setbd

#if the setbd return non 0, which means incorrect bd address file, then exit
if [ $? -ne 0 ]
then
	exit 1
fi

rfkill unblock bluetooth

echo "Check for Bluetooth device status"
if (/usr/bin/hciconfig | grep hci); then
	echo "Bluetooth device is UP"
	/usr/bin/hciconfig hci0 up
else
	echo "Bluetooth device is DOWN"
	echo "Registering Bluetooth device"

	$BCM_TOOL $BT_UART_DEVICE -DEBUG -CSTOPB -FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED -ADDR=$BT_ADDR -SETSCO=0,0,0,0,0,0,0,3,3,0 -LP

	# Attaching Broadcom device
	if (/usr/bin/hciattach $BT_UART_DEVICE -s $UART_SPEED $BT_CHIP_TYPE $UART_SPEED flow); then
		sleep 0.1
		echo "HCIATTACH success"
	else
		echo "HCIATTACH failed"
		rfkill block bluetooth
		cp /var/log/messages /var/lib/bluetooth/
	fi
fi

#/usr/bin/hciconfig hci0 down

