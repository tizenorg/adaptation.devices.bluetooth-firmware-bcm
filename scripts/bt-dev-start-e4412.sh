#!/bin/sh

#
# Script for registering Broadcom UART BT device
#
BT_UART_DEVICE=/dev/ttySAC0
BT_CHIP_TYPE=bcm2035
BCM_TOOL=/usr/bin/bcmtool_4330b1

BT_ADDR=/csa/bluetooth/.bd_addr

BT_PLATFORM_DEFAULT_HCI_NAME="TIZEN-Mobile"
UART_SPEED=3000000

# defult firmware
# SEMCO external LNA, I2S slave
BCM_FIRMWARE=BCM4334B0_002.001.013.0079.0081.hcd

HOST_NAME=`grep Hardware /proc/cpuinfo | awk "{print \\$3}"`
REVISION_NUM=`grep Revision /proc/cpuinfo | awk "{print \\$3}"`
REVISION_HIGH=`echo $REVISION_NUM| cut -c1-2`
REVISION_LOW=`echo $REVISION_NUM| cut -c3-`

echo $BCM_FIRMWARE

if [ ! -e "$BT_UART_DEVICE" ]
then
	mknod $BT_UART_DEVICE c 204 64
fi

# Set BT address: This will internally check for the file presence
/usr/bin/setbd

#if the setbd return non 0, which means incorrect bd address file, then exit
if [ $? -ne 0 ]
then
	exit 1
fi

rfkill unblock bluetooth

echo "Check for Bluetooth device status"
if (/usr/sbin/hciconfig | grep hci); then
	echo "Bluetooth device is UP"
	/usr/sbin/hciconfig hci0 up
else
	echo "Bluetooth device is DOWN"
	echo "Registering Bluetooth device"

	$BCM_TOOL $BT_UART_DEVICE -FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED -ADDR=$BT_ADDR -SETSCO=0,0,0,0,0,0,0,3,3,0 -LP > /dev/null 2>&1 &
	bcmtool_pid=$!
	#Check next 5 seconds for bcmtool success
	for (( i=1; i<=50; i++))
	do
		sleep 0.1
		kill -0 $bcmtool_pid
		bcmtool_alive=$?

		if [ $i -eq 50 ]
		then
			echo "time expired happen $i"
			kill -TERM $bcmtool_pid
			rfkill block bluetooth
			cp /var/log/messages /var/lib/bluetooth/
			exit 1
		fi

		if [ $bcmtool_alive -eq 0 ]
		then
			echo "Continue....$i"
			continue
		else
			echo "Break.......$i"
			break
		fi
	done

	# Attaching Broadcom device
	if (/usr/sbin/hciattach $BT_UART_DEVICE -s $UART_SPEED $BT_CHIP_TYPE $UART_SPEED flow); then
		sleep 0.1
		/usr/sbin/hciconfig hci0 name $BT_PLATFORM_DEFAULT_HCI_NAME
		/usr/sbin/hciconfig hci0 sspmode 1
		echo "HCIATTACH success"
	else
		echo "HCIATTACH failed"
		rfkill block bluetooth
		cp /var/log/messages /var/lib/bluetooth/
	fi
fi

#/usr/sbin/hciconfig hci0 down

