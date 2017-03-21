#!/bin/sh

## this assumes that libambix has been compiled with gcov support (--coverage)

INFOFILE="libambix.info"
OUTDIR=coverage/

MAKE=make

# Generate html report

lcov --base-directory . --directory . --zerocounters -q || exit 1
${MAKE} check || exit 1
lcov --base-directory libambix/src --directory libambix/src -c -o ${INFOFILE} || exit 1
lcov --base-directory libambix/tests --directory libambix/tests -c -o tests_${INFOFILE} || exit 1

# remove output for external libraries
lcov --remove ${INFOFILE} "/usr*" -o ${INFOFILE}  || exit 1
lcov --remove tests_${INFOFILE} "/usr*" -o tests_${INFOFILE}  || exit 1

rm -rf "${OUTDIR}"
genhtml -o "${OUTDIR}" -t "libambix test coverage" --num-spaces 4 ${INFOFILE} tests_${INFOFILE}
