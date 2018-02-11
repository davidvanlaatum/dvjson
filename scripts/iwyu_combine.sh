#!/usr/bin/env bash

shift
TEMP=`getopt --quiet --options "o:l:L:I:" -- $@`
eval set -- "$TEMP"
while true ; do
        case "$1" in
            -o) LOGFILE=$2; shift ;;
            --) shift ; break ;;
            *) shift ;;
        esac
done

while [ "x$1" != "x" ]
do
    case "$1" in
        *.o) FILES="$FILES $1"; shift;;
        *) shift ;;
    esac
done

cat $FILES > $LOGFILE
