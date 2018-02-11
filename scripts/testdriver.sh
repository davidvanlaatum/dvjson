#!/usr/bin/env bash

set -e

TEST=$1
if [ -z "$1" ]
then
    echo No test
fi

if [ ! -z "${VALGRIND}" ]
then
    export GTEST_OUTPUT="xml:${NAME}-valgrindunit.xml"
    ${VALGRIND} ${VALGRIND_OPTS} $TEST
    sed -i'' -e "s/classname=\"/classname=\"valgrind-/" ${NAME}-valgrindunit.xml
else
    export GTEST_OUTPUT="xml:${NAME}-unit.xml"
    [ -z "${COVERAGE_FILE}" ] || [ ! -f "${COVERAGE_FILE}" ] || mv ${COVERAGE_FILE} ${COVERAGE_FILE}.old
    $TEST
    [ -z "${NODE}" ] || sed -i'' -e "s/classname=\"/classname=\"${NODE}-/" ${NAME}-unit.xml
    if [ ! -z "${PYCOBERTURA}" ] && [ ! -z "${COVERAGE_FILE}" ]
    then
        ${PYCOBERTURA} show ${COVERAGE_FILE}
        [ ! -f "${COVERAGE_FILE}.old" ] || ${PYCOBERTURA} diff -s1 ${SOURCE_DIR} -s2 ${SOURCE_DIR} ${COVERAGE_FILE}.old ${COVERAGE_FILE}
    fi
fi

echo Complete
