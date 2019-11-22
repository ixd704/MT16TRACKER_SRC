#!/bin/sh


MY_DIR=`pwd`
TEMP_DIR=/tmp/t
KERNEL_PATCH_ARCHIVE=/opt/freescale/pkgs/linux-2.6.31-imx_09.12.01.bz2
echo $MY_DIR

rm -rf $TEMP_DIR

mkdir -p $TEMP_DIR

cd $TEMP_DIR
if [ $? -ne 0 ]; then
	echo unable to goto TEMP DIR
	exit -1	
fi

echo Extracting Kernel Patches
tar jxf $KERNEL_PATCH_ARCHIVE
if [ $? -ne 0 ]; then
	echo unable to extract Kernel patches!
	exit -1	
fi

echo Merging Kernel patches
rm -f patches/1???-*
rm -f patches/1???_*
cp $MY_DIR/patches/*.patch patches
if [ $? -ne 0 ]; then
	echo Patches merge error
	exit -1	
fi

tar jcf $KERNEL_PATCH_ARCHIVE patches/
if [ $? -ne 0 ]; then
	echo Unable to create Kernel Patch
	exit -1	
fi


md5sum $KERNEL_PATCH_ARCHIVE > $KERNEL_PATCH_ARCHIVE.md5



