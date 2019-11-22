#!/bin/bash
ROOT_DIR=`pwd`

function ExecuteCommand
{
	if [ $# -ne 2 ]; then
		echo Parameters missing
		echo -e "\n[FAILED]"
		exit 1
	else
		DIR=$1
		CMD=$2
		cd $DIR; $CMD
		if [ $? -ne 0 ]; then
			echo -e "\nExecuting [$DIR -> $CMD] Failed!!"
			echo -e "\n[FAILED]"
			exit 1
		fi
	fi
}



#Update the package sources
ExecuteCommand $ROOT_DIR 'tar jcf /opt/freescale/pkgs/libjsoncpp-1.0.tar.bz2 libjsoncpp-1.0/'

ExecuteCommand $ROOT_DIR/ltib 'bash ./merge.sh'
ExecuteCommand $ROOT_DIR/kernel 'bash ./merge_patches.sh'
ExecuteCommand $ROOT_DIR/menu-1.0 'bash ./merge.sh'
ExecuteCommand $ROOT_DIR/jamhubaudio-1.1 'bash ./merge.sh'
ExecuteCommand $ROOT_DIR/miscellaneous 'bash ./apply.sh'
ExecuteCommand $ROOT_DIR/networkservice 'bash ./merge.sh'

echo -e "\n[SUCCESS]"
