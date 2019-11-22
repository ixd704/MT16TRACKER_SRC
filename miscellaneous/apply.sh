#!/bin/sh


MY_DIR=`pwd`
TEMP_DIR=/tmp/t

source ../environment.sh


MERGE_DIR=$LTIB_DIR/merge

rm -rf $MERGE_DIR


chmod 555 merge/etc/rc.d/rcS
chmod 555 merge/etc/rc.d/autostart.d/*

cp -a merge $LTIB_DIR
if [ $? -ne 0 ]; then
	echo unable to copy Merge DIR!
	exit -1	
fi

#Build SD force detection Driver
cd drivers/sd_force_detection

make
if [ $? -ne 0 ]; then
	echo unable to Build SD Force detection driver
	exit -1	
fi

mkdir -p $MERGE_DIR/lib/modules/$KERNEL_VERSION/kernel/drivers/misc/
cp -v sddetect.ko $MERGE_DIR/lib/modules/$KERNEL_VERSION/kernel/drivers/misc/
if [ $? -ne 0 ]; then
	echo unable to copy SD Force detection driver
	exit -1	
fi
make clean


mkdir -p $MERGE_DIR/lib/modules/$KERNEL_VERSION/kernel/drivers/net/wifi/
cp -v /home/pavel/RTL8188EUS_RTL8189ES_linux_v4.1.3_6099.20121214/driver/rtl8188EUS_rtl8189ES_linux_v4.1.3_6099.20121214/8189es.ko $MERGE_DIR/lib/modules/$KERNEL_VERSION/kernel/drivers/net/wifi/
if [ $? -ne 0 ]; then
	echo unable to copy WIFI driver
	exit -1	
fi










