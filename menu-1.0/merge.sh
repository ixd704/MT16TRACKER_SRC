#!/bin/bash

PACKAGE_BASE_NAME=menu
PACKAGE_VERSION=1.0
PACKAGE_NAME=$PACKAGE_BASE_NAME-$PACKAGE_VERSION
MY_DIR=`pwd`
TEMP_DIR=/tmp/t

source ../environment.sh
PKG_BUILD_DIR=$LTIB_DIR/rpm/BUILD/$PACKAGE_NAME/

cd $LTIB_DIR
if [ $? -ne 0 ]; then
	echo unable to change to LTIB DIR!
	exit -1	
fi

./ltib -m clean -p $PACKAGE_BASE_NAME
rm -rf $LTIB_DIR/rpm/BUILD/$PACKAGE_BASE_NAME*
rm -rf $PKG_BUILD_DIR
rm -f $LTIB_DIR/rpm/RPMS/arm/$PACKAGE_BASE_NAME*.rpm
rm -rf $LTIB_DIR/rpm/SOURCES/$PACKAGE_BASE_NAME*
rm -rf $LTIB_PACKAGE_DIR/$PACKAGE_BASE_NAME*
rm -rf $TEMP_DIR


mkdir -p $TEMP_DIR

cd $MY_DIR/..
cp -a $PACKAGE_NAME/ $TEMP_DIR/
if [ $? -ne 0 ]; then
	echo unable create temporary directory.
	exit -1	
fi

cp -vf protobuf/tracker.proto $TEMP_DIR/$PACKAGE_NAME/

cd $TEMP_DIR
tar jcf $LTIB_PACKAGE_DIR/$PACKAGE_NAME.tar.bz2 $PACKAGE_NAME
if [ $? -ne 0 ]; then
	echo unable create the package $PACKAGE_NAME.
	exit -1	
fi

md5sum $LTIB_PACKAGE_DIR/$PACKAGE_NAME.tar.bz2 > $LTIB_PACKAGE_DIR/$PACKAGE_NAME.tar.bz2.md5

cd $LTIB_DIR
echo Preparing $PACKAGE_NAME
./ltib -m prep -p $PACKAGE_BASE_NAME
if [ $? -ne 0 ]; then
	echo unable to prepare $PACKAGE_NAME
	exit -1	
fi


./ltib -m scbuild	-p $PACKAGE_BASE_NAME
if [ $? -ne 0 ]; then
	echo unable to build $PACKAGE_NAME
	exit -1	
fi

./ltib -m scinstall -p $PACKAGE_BASE_NAME
if [ $? -ne 0 ]; then
	echo unable to install $PACKAGE_NAME
	exit -1	
fi

./ltib -m scdeploy 	-p $PACKAGE_BASE_NAME
if [ $? -ne 0 ]; then
	echo unable to deploy $PACKAGE_NAME
	exit -1	
fi


