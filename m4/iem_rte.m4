dnl Copyright (C) 2005-2012 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# CHECK_RTE()
#
# checks for RTE (currently: Pd)
# if so, they are added to the LDFLAGS, CFLAGS and whatelse
AC_DEFUN([IEM_CHECK_RTE],
[
IEM_OPERATING_SYSTEM

tmp_rte_orgcppflags="$CPPFLAGS"
tmp_rte_orgcflags="$CFLAGS"
tmp_rte_orgcxxflags="$CXXFLAGS"
tmp_rte_orgldflags="$LDFLAGS"
tmp_rte_orglibs="$LIBS"

rmp_rte_cflags="-DPD"
rmp_rte_libs=""
RTE="Pure Data"

AC_ARG_WITH([pd], 
	        AS_HELP_STRING([--with-pd=<path/to/pd>],[where to find pd-binary (./bin/pd.exe) and pd-sources]))

## some default paths
if test "x${with_pd}" = "x"; then
 case $host_os in
 *darwin*)
    if test -d "/Applications/Pd-extended.app/Contents/Resources"; then with_pd="/Applications/Pd-extended.app/Contents/Resources"; fi
    if test -d "/Applications/Pd.app/Contents/Resources"; then with_pd="/Applications/Pd.app/Contents/Resources"; fi
 ;;
 *mingw* | *cygwin*)
    if test -d "${PROGRAMFILES}/Pd-extended"; then with_pd="${PROGRAMFILES}/Pd-extended"; fi
    if test -d "${PROGRAMFILES}/pd"; then with_pd="${PROGRAMFILES}/pd"; fi
 ;;
 esac
fi

if test -d "$with_pd" ; then
 if test -d "${with_pd}/src" ; then
   AC_LIB_APPENDTOVAR([rmp_rte_cflags],"-I${with_pd}/src")
 elif test -d "${with_pd}/include/pd" ; then
   AC_LIB_APPENDTOVAR([rmp_rte_cflags],"-I${with_pd}/include/pd")
 elif test -d "${with_pd}/include" ; then
   AC_LIB_APPENDTOVAR([rmp_rte_cflags],"-I${with_pd}/include")
 else
   AC_LIB_APPENDTOVAR([rmp_rte_cflags],"-I${with_pd}")
 fi
 if test -d "${with_pd}/bin" ; then
   rmp_rte_libs="${rmp_rte_libs}${rmp_rte_libs:+ }-L${with_pd}/bin"
 else
   rmp_rte_libs="${rmp_rte_libs}${rmp_rte_libs:+ }-L${with_pd}"
 fi

 CPPFLAGS="$CPPFLAGS ${rmp_rte_cflags}"
 CFLAGS="$CFLAGS ${rmp_rte_cflags}"
 CXXFLAGS="$CXXFLAGS ${rmp_rte_cflags}"
 LIBS="$LIBS ${rmp_rte_libs}"
fi

AC_CHECK_LIB([:pd.dll], [nullfn], [have_pddll="yes"], [have_pddll="no"])
if test "x$have_pddll" = "xyes"; then
 rmp_rte_libs="${rmp_rte_libs}${rmp_rte_libs:+ }-Xlinker -l:pd.dll"
else
 AC_CHECK_LIB([pd], [nullfn], [rmp_rte_libs="${rmp_rte_libs}${rmp_rte_libs:+ }-lpd"])
fi

AC_CHECK_HEADERS([m_pd.h], [have_pd="yes"], [have_pd="no"])

dnl LATER check why this doesn't use the --with-pd includes
dnl for now it will basically disable anything that needs s_stuff.h if it cannot be found in /usr[/local]/include
AC_CHECK_HEADERS([s_stuff.h], [], [],
[#ifdef HAVE_M_PD_H
# define PD
# include "m_pd.h"
#endif
])

### this should only be set if Pd has been found
# the extension
AC_ARG_WITH([extension], 
		AS_HELP_STRING([--with-extension=<ext>],[enforce a certain file-extension (e.g. pd_linux)]))
if test "x$with_extension" != "x"; then
 tmp_rte_extension=$with_extension
else
  case x$host_os in
   x*darwin*)
     tmp_rte_extension=pd_darwin
     ;;
   x*mingw* | x*cygwin*)
     tmp_rte_extension=dll
     ;;
   x)
     dnl just assuming that it is still linux (e.g. x86_64)
     tmp_rte_extension="pd_linux"
     ;;
   *)
     tmp_rte_extension=pd_`echo $host_os | sed -e '/.*/s/-.*//' -e 's/\[.].*//'`
     ;;
  esac
fi
RTE_EXTENSION=$tmp_rte_extension
RTE_CFLAGS="$tmp_rte_cflags"
RTE_LIBS="$tmp_rte_libs"

AC_SUBST(RTE_EXTENSION)
AC_SUBST(RTE_CFLAGS)
AC_SUBST(RTE_LIBS)
AC_SUBST(RTE)
AC_ARG_WITH([rtedir], 
	[AS_HELP_STRING([--rtedir=DIR],	[externals (plugin) dir for RTE (default ${exec_prefix}/lib/pd/extra)])],
	[rtedir=$withval],
	[rtedir=['${exec_prefix}/lib/pd/extra']])
AC_SUBST([rtedir],	['${exec_prefix}/lib/pd/extra'])dnl

CPPFLAGS="$tmp_rte_orgcppflags"
CFLAGS="$tmp_rte_orgcflags"
CXXFLAGS="$tmp_rte_orgcxxflags"
LDFLAGS="$tmp_rte_orgldflags"
LIBS="$tmp_rte_orglibs"
]) # CHECK_RTE
