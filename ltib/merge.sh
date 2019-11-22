#!/bin/sh

source ../environment.sh

echo Updating LTIB configuration..
cp -av  config/ dist/ $LTIB_DIR/


if [ $? -ne 0 ]; then
	echo unable to copy LTIB config files!
	exit -1	
fi

