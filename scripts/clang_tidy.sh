#!/usr/bin/env bash
shift

LOGFILE=${@: -1}
LOGFILE=$(basename $LOGFILE).clang_tidy.log
while [ $# -gt 0 ]; do
        case "$1" in
            -o) LOGFILE=$2; shift; shift ;;
            -c) INPUT=$2; shift; shift;;
            *) shift ;;
        esac
done

@CLANG_TIDY_PATH@ -p "@CMAKE_BINARY_DIR@" $INPUT &> $LOGFILE.tmp
EXIT=$?
if [ "$EXIT" -gt 0 ]
then
    echo $INPUT
    cat $LOGFILE.tmp
    rm $LOGFILE.tmp
else
    if [ $(sed -e '/warnings generated/d;/to display errors from all non-system headers/d;/Suppressed [0-9][0-9]* warnings/d;/clang-diagnostic-ignored-optimization-argument/d' $LOGFILE.tmp | wc -l) -eq 0 ]
    then
        > $LOGFILE.tmp
    fi
    mv $LOGFILE{.tmp,}
fi
exit $EXIT
