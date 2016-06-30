#!/bin/sh

APP=$1

_TESTTYPE=$2
TESTTYPE=${TESTTYPE:-$_TESTTYPE}

##################################

VALGRIND="libtool --mode=execute valgrind --error-exitcode=1 -q"

if which valgrind > /dev/null
then
 MEMGRIND="${VALGRIND} --leak-check=full"
 DRDGRIND="${VALGRIND} --tool=drd"
 HELGRIND="${VALGRIND} --tool=helgrind"
else
 MEMGRIND=
 DRDGRIND=
 HELGRIND=
fi

get_runtest() {
case "$1" in
 mem*|MEM*)
    echo ${MEMGRIND}
 ;;
 DRD|drd)
    echo ${DRDGRIND}
 ;;
 HEL*|hel*)
    echo ${HELGRIND}
 ;;
 *)
 ;;
esac
}


$(get_runtest $TESTTYPE) ${APP}
