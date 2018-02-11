#!/usr/bin/env bash
shift
while [ $# -gt 0 ] ; do
        case "$1" in
            -o) LOGFILE=$2; shift ;;
            *.o) FILES="$FILES $1"; shift;;
            *) shift ;;
        esac
done

if [ "x$FILES" != "x" ]
then
    cat $FILES > $LOGFILE
fi
