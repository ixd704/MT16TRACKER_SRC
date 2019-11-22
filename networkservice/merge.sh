#!/bin/bash

MY_DIR=`pwd`
PKG_ARCHIVE=networkService_latest.tgz
TEMP_DIR=/tmp/t

source ../environment.sh

PKG_DIR=$LTIB_DIR/merge/usr/local/networkservice


rm -rf $TEMP_DIR

mkdir -p $TEMP_DIR
if [ $? -ne 0 ]; then
	echo unable create temporary directory.
	exit -1	
fi

cd $MY_DIR

if [ ! -e $PKG_ARCHIVE ]; then
	echo File networkService_latest.tgz not found, Not updating NetworkService
	exit 0
fi

rm -rf $PKG_DIR/

mkdir -p $PKG_DIR
if [ $? -ne 0 ]; then
	echo unable create the directory $PKG_DIR
	exit -1	
fi


cd $TEMP_DIR
tar zxf $MY_DIR/$PKG_ARCHIVE
if [ $? -ne 0 ]; then
	echo unable extract $PKG_ARCHIVE
	exit -1	
fi

if [ ! -d lib ]; then
	echo File networkService_latest.tgz contents not in expected format
	exit -1
fi

cp -av lib/networkService*/* $PKG_DIR/

if [ ! -e  $PKG_DIR/service_main ]; then
	echo File $PKG_DIR/service_main not found
	exit -1
fi


