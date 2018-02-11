#!/usr/bin/env bash
shift
while [ $# -gt 0 ] ; do
        case "$1" in
            -o) FILE=$2; shift; shift ;;
            *) shift ;;
        esac
done

touch $FILE
