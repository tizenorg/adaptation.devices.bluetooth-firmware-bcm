#!/bin/sh

#
# Script for executing Bluetooth stack
#

# Register BT Device
/usr/etc/bluetooth/bt-dev-start.sh

if !(/usr/sbin/hciconfig | grep hci); then
	echo "Registering BT device is failed."
	exit 1
fi

# Execute BlueZ BT stack
echo "Run bluetoothd"
/usr/sbin/bluetoothd -d
/usr/lib/obex/obexd -d --noplugin=syncevolution,pcsuite --symlinks -r /opt/share/bt-ftp
/usr/bin/bluetooth-share &
sleep 2

exit 0

# Check result
#if (dbus-send --system --print-reply --dest=org.bluez / org.bluez.Manager.DefaultAdapter | grep hci); then
#	exit 0
#else
#	echo "Running BT stack is failed."
#	exit 1
#fi

