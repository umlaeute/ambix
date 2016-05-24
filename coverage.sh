#!/bin/sh

INFOFILE="libambix.info"
OUTDIR=coverage/
MAKE=make

${MAKE} clean

# Reconfigure with gcov support
CFLAGS="-g -O0 --coverage" CXXFLAGS="-g -O0 --coverage" LDFLAGS="--coverage" \
	./configure --enable-debug --disable-silent-rules || exit 1

# Generate gcov output
${MAKE} || exit 1

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
