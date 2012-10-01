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

AC_ARG_WITH([rte], 
	        AS_HELP_STRING([--with-rte=<rte>],[use the given RTE (e.g.  'pd') or none]))
AC_ARG_WITH([pd], 
	        AS_HELP_STRING([--with-pd=<path/to/pd>],[where to find pd-binary (./bin/pd.exe) and pd-sources]))
### this should only be set if Pd has been found
# the extension
AC_ARG_WITH([extension], 
		AS_HELP_STRING([--with-extension=<ext>],[enforce a certain file-extension (e.g. pd_linux)]))

AC_ARG_WITH([rtedir], 
	[AS_HELP_STRING([--rtedir=DIR],	[externals (plugin) dir for RTE (default ${exec_prefix}/lib/pd/extra)])],
	[rtedir=$withval],
	[rtedir=['${exec_prefix}/lib/pd/extra']])

test "x${with_rte}" = "x" && with_rte="yes"

## this really should run some autodetection
test "x${with_rte}" = "xyes" && with_rte="pd"

if test "x${with_rte}" = "xpd"; then
tmp_rte_orgcppflags="$CPPFLAGS"
tmp_rte_orgcflags="$CFLAGS"
tmp_rte_orgcxxflags="$CXXFLAGS"
tmp_rte_orgldflags="$LDFLAGS"
tmp_rte_orglibs="$LIBS"

tmp_rte_cflags="-DPD"
tmp_rte_libs=""
RTE="Pure Data"

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
   tmp_rte_cflags="${tmp_rte_cflags}${tmp_rte_cflags:+ }-I${with_pd}/src"
 elif test -d "${with_pd}/include/pd" ; then
   tmp_rte_cflags="${tmp_rte_cflags}${tmp_rte_cflags:+ }-I${with_pd}/include/pd"
 elif test -d "${with_pd}/include" ; then
   tmp_rte_cflags="${tmp_rte_cflags}${tmp_rte_cflags:+ }-I${with_pd}/include"
 else
   tmp_rte_cflags="${tmp_rte_cflags}${tmp_rte_cflags:+ }-I${with_pd}"
 fi
 if test -d "${with_pd}/bin" ; then
   tmp_rte_libs="${tmp_rte_libs}${tmp_rte_libs:+ }-L${with_pd}/bin"
 else
   tmp_rte_libs="${tmp_rte_libs}${tmp_rte_libs:+ }-L${with_pd}"
 fi

 CPPFLAGS="$CPPFLAGS ${tmp_rte_cflags}"
 CFLAGS="$CFLAGS ${tmp_rte_cflags}"
 CXXFLAGS="$CXXFLAGS ${tmp_rte_cflags}"
 LIBS="$LIBS ${tmp_rte_libs}"
fi

AC_CHECK_LIB([:pd.dll], [nullfn], [have_pddll="yes"], [have_pddll="no"])
if test "x$have_pddll" = "xyes"; then
 tmp_rte_libs="${tmp_rte_libs}${tmp_rte_libs:+ }-Xlinker -l:pd.dll"
else
 AC_CHECK_LIB([pd], [nullfn], [tmp_rte_libs="${tmp_rte_libs}${tmp_rte_libs:+ }-lpd"])
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
AC_SUBST([rtedir],	['${exec_prefix}/lib/pd/extra'])dnl

CPPFLAGS="$tmp_rte_orgcppflags"
CFLAGS="$tmp_rte_orgcflags"
CXXFLAGS="$tmp_rte_orgcxxflags"
LDFLAGS="$tmp_rte_orgldflags"
LIBS="$tmp_rte_orglibs"
fi
]) # CHECK_RTE
