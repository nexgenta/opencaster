#!/bin/bash

if [ $# -lt 5 ]
then
    /bin/echo "Usage:"
    /bin/echo "oc-update.sh object_carousel_directory association_tag module_version dsmcc_pid carousel_id [compress_on] [padding_on] [clean_off]"
    /bin/echo "	carousel_directory: the directory to marshal in an object carousel"
    /bin/echo "	association_tag aka common tag, referenced by PMTs and AITs, every carousel has its own"
    /bin/echo "	modules_version, all the modules will have the same version, you need to change this to notify to the box files are changed, goes from 0 to 15"
    /bin/echo "	pid, referenced by PMTs using this carousel"
    /bin/echo "	carousel_id, referenced by PMTs using this carousel, every carousel has its own"
    /bin/echo "	compress_on, compress the carousel, default off"
    /bin/echo "	padding_on, every section is padded, usuful with some buggy decoder, waste bandwith, default off"
    /bin/echo "	clean_off, don't delete temp file, default off"
    /bin/echo
    /bin/echo "Example:"
    /bin/echo "oc-update.sh ocdir1 0xB 5 2001 1 1 0 0"
    /bin/echo "	carousel_directory: ocdir1"
    /bin/echo "	association_tag: 0xB (11)"
    /bin/echo "	modules_version: 5"
    /bin/echo "	pid: 2001"
    /bin/echo "	carousel_id: 1"
    /bin/echo "	compress the carousel"
    /bin/echo "	don't pad"
    /bin/echo "	delete temp files"
    exit 65
fi

#Parameters passing
OCDIR=$1
ASSOCIATION_TAG=$2
MODULE_VERSION=$3
PID=$4
CAROUSEL_ID=$5
COMPRESS_ON="0"
PAD_ON="0"
NO_DELETE_TEMP="0"
if [ $# -gt 5 ]
then
COMPRESS_ON=$6    
fi
if [ $# -gt 6 ]
then
PAD_ON=$7
fi
if [ $# -gt 7 ]
then
NO_DELETE_TEMP=$8
fi

#Generate temp directories
TEMP_DIR_MOD=`/bin/mktemp -d`
TEMP_DIR_SEC=`/bin/mktemp -d`

#Generate the modules from the directory, the modules are stored into a tmp directory TEMP_DIR_MOD
/usr/local/bin/file2mod.py $OCDIR $TEMP_DIR_MOD $CAROUSEL_ID $CAROUSEL_ID $ASSOCIATION_TAG $MODULE_VERSION

#Compress modules if required
if [ "$COMPRESS_ON" = "1" ]
then
    for file in $TEMP_DIR_MOD/*.mod
    do
	/usr/bin/du --apparent-size --block-size 1 $file > $file.size
	/usr/local/bin/zpipe < $file > $file.z 
	/bin/mv $file.z $file
    done
fi

#Generate sections from modules, the sections are stored into a tmp directory TEMP_DIR_SEC
/usr/local/bin/mod2sec.py $TEMP_DIR_MOD $TEMP_DIR_SEC

# Check if it is necessary to pad every sections or not, unluckly we have found some decoders having buggy section filtering that needed this
if [ "$PAD_ON" = "1" ]
then	
    #Every section will be padded to the minimum number of packets needed to contain it, all the packets are enqueued into the output ts file
    for file in $TEMP_DIR_SEC/*.sec
    do
        /usr/local/bin/sec2ts $PID < $file >> $TEMP_DIR_SEC/temp_ts
    done
    mv $TEMP_DIR_SEC/temp_ts $OCDIR.ts
else
    # All the single section files are enqueued in a single file, padding will occur only at the end of the last section
    for file in $TEMP_DIR_SEC/*.sec
    do
	/bin/cat $file >> $TEMP_DIR_SEC/temp_sec
    done
    /usr/local/bin/sec2ts $PID < $TEMP_DIR_SEC/temp_sec > $OCDIR.ts
fi

# Delete temp files
if [ "$NO_DELETE_TEMP" = "0" ]
then
/bin/rm $TEMP_DIR_MOD/*
/bin/rm $TEMP_DIR_SEC/*
/bin/rmdir $TEMP_DIR_MOD
/bin/rmdir $TEMP_DIR_SEC
else
/bin/echo "Modules generated in $TEMP_DIR_MOD were not delete"
/bin/echo "Sections generated in $TEMP_DIR_SEC were not delete"
fi
