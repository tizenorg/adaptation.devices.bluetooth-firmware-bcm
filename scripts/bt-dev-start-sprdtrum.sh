#!/bin/sh

GREP="/bin/grep"
MKNOD="/bin/mknod"
AWK="/usr/bin/awk"
RFKILL="/usr/sbin/rfkill"
CP="/bin/cp"
SLEEP="/bin/sleep"

#
# Script for registering Broadcom UART BT device
#
BT_UART_DEVICE=/dev/ttyS0
BT_CHIP_TYPE=bcm2035
BCM_TOOL=/usr/bin/bcmtool

BCM_TOOL_DBG_LOG=/var/lib/bluetooth/bcmtool_log

# If you want to enable bcmtool debug log, please uncomment below #
#ENABLE_BCMTOOL_DEBUG="-DEBUG"

HCI_CONFIG=/usr/bin/hciconfig
HCI_ATTACH=/usr/bin/hciattach

BT_ADDR=/csa/bluetooth/.bd_addr

SYSLOG_PATH=/var/log/messages

UART_SPEED=3000000

#Firmware Loading timeout:  Unit * 100ms
# Example : 60 is 6.0 sec
TIMEOUT=60

BCM_4343A0="BCM4343A0_001.001.034.0058.0215_ORC_Kiran.hcd"

echo "Check for Bluetooth device status"
if (${HCI_CONFIG} | grep hci); then
	echo "Bluetooth device is UP"
	${HCI_CONFIG} hci0 up
	exit 1
fi

#Select Firmware to check chip info
BCM_FIRMWARE=${BCM_4343A0}

${RFKILL} unblock bluetooth

echo "BCM_FIRMWARE: $BCM_FIRMWARE"

# Set BT address: This will internally check for the file presence
/usr/bin/setbd

#if the setbd return non 0, which means incorrect bd address file, then exit
if [ $? -ne 0 ]
then
	exit 1
fi

echo "Registering Bluetooth device"

$BCM_TOOL $BT_UART_DEVICE $ENABLE_BCMTOOL_DEBUG -CSTOPB \
	-FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED \
	-ADDR=$BT_ADDR -LP -SCO >$BCM_TOOL_DBG_LOG  2>&1 &
bcmtool_pid=$!

#Check next timeout seconds for bcmtool success
for (( i=1; i<=$TIMEOUT; i++))
do
        ${SLEEP} 0.1
        kill -0 $bcmtool_pid
        bcmtool_alive=$?

        if [ $i -eq $TIMEOUT ]
        then
                echo "time expired happen $i"
                kill -TERM $bcmtool_pid
                ${RFKILL} block bluetooth
                ${CP} $SYSLOG_PATH /var/lib/bluetooth/
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
if (${HCI_ATTACH} $BT_UART_DEVICE -s $UART_SPEED $BT_CHIP_TYPE $UART_SPEED flow); then
	${SLEEP} 0.1
	echo "HCIATTACH success"
else
	echo "HCIATTACH failed"
	${RFKILL} block bluetooth
	${CP} $SYSLOG_PATH /var/lib/bluetooth/
fi
