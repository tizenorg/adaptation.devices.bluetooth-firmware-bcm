#!/bin/sh

GREP="/bin/grep"
MKNOD="/bin/mknod"
AWK="/usr/bin/awk"
RFKILL="/usr/sbin/rfkill"
CP="/bin/cp"

#
# Script for registering Broadcom UART BT device
#
BT_UART_DEVICE=/dev/ttyHS0
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
# Example : 34 is 3.4 sec
TIMEOUT=34

BCM_4334_SEMCO="BCM4334W0_001.002.003.0014.0017_Ponte_Solo_Semco_B58_13.5dBm.hcd"
BCM_4334_MURATA="BCM43342A1_001.002.003.1006.0000_Rintao_G3_ePA.hcd"
BCM_4343_SEMCO="BCM4343A0_001.001.034.0048.0145_ORC_Ponte_Solo-3G.hcd"
BCM_4343_A1_SEMCO="BCM4343A1_001.002.009.0009.0012_ORC_Ponte_Solo-3G.hcd"

#REVISION_NUM=`${GREP} Revision /proc/cpuinfo | ${AWK} "{print \\$3}"`

echo "Check for Bluetooth device status"
if (${HCI_CONFIG} | grep hci); then
	echo "Bluetooth device is UP"
	${HCI_CONFIG} hci0 up
	exit 1
fi

#Get RFKILL info (ex. bcm4343w X semco)
BCM_CHIP_NAME=`${RFKILL} list bluetooth | ${AWK} -F'[: ]' '/^0/{print $3}'`
BCM_CHIP_REV=`${RFKILL} list bluetooth | ${AWK} -F'[: ]' '/^0/{print $4}'`
BCM_CHIP_PKG=`${RFKILL} list bluetooth | ${AWK} -F'[: ]' '/^0/{print $5}'`

#Select Firmware to check chip info
BCM_FIRMWARE=${BCM_4343_SEMCO}

${RFKILL} unblock bluetooth

if [ "$BCM_CHIP_NAME" == "bcm4334w" ]; then
	if [ "$BCM_CHIP_PKG" == "semco" ]; then
		BCM_FIRMWARE=${BCM_4334_SEMCO}
	elif [ "$BCM_CHIP_PKG" == "murata" ]; then
		BCM_FIRMWARE=${BCM_4334_MURATA}
	fi
elif [ "$BCM_CHIP_NAME" == "bcm4343w" ]; then
	if [ "$BCM_CHIP_REV" == "a0" ]; then
		BCM_FIRMWARE=${BCM_4343_SEMCO}
	elif [ "$BCM_CHIP_REV" == "a0_a1" ]; then
		BT_HW_CHIP_NAME=`$BCM_TOOL $BT_UART_DEVICE -GETNAME 2>&1| ${AWK} '/^Chip/{print $4}'`

		if [ $BT_HW_CHIP_NAME == "BCM43430A1" ]; then
			BCM_FIRMWARE=${BCM_4343_A1_SEMCO}
		fi
	fi
fi

echo "BCM_CHIP_NAME: $BCM_CHIP_NAME, BCM_CHIP_REV: $BCM_CHIP_REV, BCM_CHIP_PKG: $BCM_CHIP_PKG"
echo "BCM_FIRMWARE: $BCM_FIRMWARE"

# Set BT address: This will internally check for the file presence
/usr/bin/setbd

#if the setbd return non 0, which means incorrect bd address file, then exit
if [ $? -ne 0 ]
then
	exit 1
fi

echo "Registering Bluetooth device"

$BCM_TOOL $BT_UART_DEVICE -TYPE=${BCM_CHIP_NAME} $ENABLE_BCMTOOL_DEBUG \
	-FILE=/usr/etc/bluetooth/$BCM_FIRMWARE -BAUD=$UART_SPEED \
	-ADDR=$BT_ADDR -PCM_SETTING -LP  >$BCM_TOOL_DBG_LOG  2>&1 &
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
                rfkill block bluetooth
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
	/bin/sleep 0.1
	echo "HCIATTACH success"
else
	echo "HCIATTACH failed"
	${RFKILL} block bluetooth
	${CP} $SYSLOG_PATH /var/lib/bluetooth/
fi
