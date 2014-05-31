#!/bin/sh

#
# Script for registering Broadcom UART BT device
#
BT_UART_DEVICE=/dev/ttySAC0
BT_CHIP_TYPE=bcm2035
BCM_TOOL=/usr/bin/bcmtool_4330b1

BT_ADDR=/csa/bluetooth/.bd_addr

UART_SPEED=3000000
TIMEOUT=24

# defult firmware
# SEMCO external LNA, I2S slave, fix high sleep current
BCM_FIRMWARE=BCM20710A1_001.002.014.0059.0060.hcd

#B2 - REV_01 - 4334W SEMCO B58 module with iPA(internal PA)
#B2 - REV_02 - 4334W SEMCO B58 module with ePA(external PA)
BCM_B2_FIRMWARE=BCM4334B0_002.001.013.1675.1676_B2_ORC.hcd
BCM_B2_SEC_FIRMWARE_REV_01=BCM4334W_Rinato_TestOnly.hcd
BCM_B2_SEC_FIRMWARE_REV_02=BCM4334W_001.002.003.0997.1027_B58_ePA.hcd

HOST_NAME=`grep Hardware /proc/cpuinfo | awk "{print \\$3}"`
REVISION_NUM=`grep Revision /proc/cpuinfo | awk "{print \\$3}"`

if [ "$HOST_NAME" == "B2" ]; then
	if [ "$REVISION_NUM" == "0000" ]; then
		BCM_FIRMWARE=$BCM_B2_FIRMWARE
	else
		if [ "$REVISION_NUM" == "0001" ]; then
			BCM_FIRMWARE=$BCM_B2_SEC_FIRMWARE_REV_01
		else
			BCM_FIRMWARE=$BCM_B2_SEC_FIRMWARE_REV_02
		fi
	fi
fi

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

	# In tizenW hardware first time bcmtool download may not success, hence we need to try more times
	MAXBCMTOOLTRY=5
	flag=0
	for (( c=1; c<=$MAXBCMTOOLTRY; c++))
	do
		echo "******* Bcmtool download attempt $c ********"

	$BCM_TOOL $BT_UART_DEVICE -FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED -ADDR=$BT_ADDR -SETSCO=0,0,0,0,0,0,0,3,3,0 -LP &
		bcmtool_pid=$!
		#Check next timeout seconds for bcmtool success
		for (( i=1; i<=$TIMEOUT; i++))
		do
			sleep 0.1
			kill -0 $bcmtool_pid
			bcmtool_alive=$?

			if [ $i -eq $TIMEOUT ]
			then
				echo "time expired happen $i"
				kill -TERM $bcmtool_pid
				break
#				rfkill block bluetooth
#				exit 1
			fi

			if [ $bcmtool_alive -eq 0 ]
			then
				echo "Continue....$i"
				continue
			else
				echo "Break.......$i"
				flag=1
				break
			fi
		done

		if [ $flag -eq 1 ]
		then
			echo "Break bcmtool download loop on $c attempt"
			break
		else
			sleep 1
			echo "sleep done"
		fi


		if [ $c -eq $MAXBCMTOOLTRY ]
		then
			echo "***** No Chance to activate, count=$c ******"
			rfkill block bluetooth
			exit 1
		fi

	done

	echo "Try for hciattach"

	# Attaching Broadcom device
	if (/usr/sbin/hciattach $BT_UART_DEVICE -s $UART_SPEED $BT_CHIP_TYPE $UART_SPEED flow); then
		sleep 0.1
		echo "HCIATTACH success"
	else
		echo "HCIATTACH failed"
		rfkill block bluetooth
		cp /var/log/messages /var/lib/bluetooth/
	fi
fi

#/usr/sbin/hciconfig hci0 down

