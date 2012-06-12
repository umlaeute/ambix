#!/bin/sh

usage_print() {
  echo "usage: $0 <directory>"
  echo "	will generate <directory>/Makefile.am by adding all files in"
  echo "	<directory> to the noinst_DATA target"
}

usage() {
 usage_print 1>&2
 exit 1
}


TARGETDIR=$1

if [ -d "${TARGETDIR}" ]; then
 :
else
 usage
fi

listfiles() {
  git ls-files | grep -v Makefile.am | sort -u
}

doit() {
echo -n "nodist_DATA="
listfiles | while read line; do
 echo " \\"
 echo -n "	${line}"
done
}

cd ${TARGETDIR}
doit > Makefile.am

