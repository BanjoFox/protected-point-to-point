#!/bin/sh
#
# This script must be run as root
#

myID=`id -u`
if [ "$myID" != "0" ]; then
  echo "This script must be run by root"
  exit 1
fi

adb/adb-linux push prepary.sh /data/local/tmp/.
if [ "$?" != "0" ]; then
	echo "Cannot push files to the Android device."
	echo "Make sure it is connected with the command:"
	echo "  adb/adb-linux devices"
	echo ''
	echo "If the device is attached on a USB port, then run the"
	echo "following commands as root, until the device is available:"
	echo "  adb/adb-linux kill-server"
	echo "  adb/adb-linux start-server"
	echo "  adb/adb-linux devices"
	echo ''
	echo "If the device is using ADB wireless, make sure the app is running"
	echo "on the device and run the following command:"
	echo "  adb/adb-linux connect <device_ip_address>"
	echo "  adb/adb-linux devices"
	echo ''
	exit 1
fi
adb/adb-linux push p3run /data/local/tmp/.
adb/adb-linux push p3stop /data/local/tmp/.
adb/adb-linux push p3secondary.conf /data/local/tmp/.
adb/adb-linux push p3secondary.ko /data/local/tmp/.
adb/adb-linux push p3syssecondary /data/local/tmp/.

echo "The Velocite Systems P3 files have been installed"
echo "To test the installation, run the following commands:"
echo "  adb/adb-linux shell"
echo "  su"
echo "  cd /data/local/tmp"
echo "  ls -l"
echo ''
echo "  NOTE:  Verify that the following files have execute permission (-rwxr-xr-x):"
echo "    p3run, p3stop, p3prepare.sh"
echo ''
echo "  ./p3run"
echo "  ./p3stop"
echo "  cat p3.log"
echo ''
echo "  NOTE: The log file should have some networking messages.  If the"
echo "     P3 primary is not active, these will be errors.  Otherwise,"
echo "     there should be a message indicating a successful connection."
echo ''

