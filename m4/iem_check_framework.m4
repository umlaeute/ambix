dnl Copyright (C) 2005-2012 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# IEM_CHECK_FRAMEWORK(FRAMEWORK, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
#
AC_DEFUN([IEM_CHECK_FRAMEWORK],
[
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                              [ABCDEFGHIJKLMNOPQRSTUVWXYZ____])])
  AC_SUBST(IEM_FRAMEWORK_[]NAME[])

  AC_MSG_CHECKING([for "$1"-framework])

  iem_check_ldflags_org="${LDFLAGS}"
  LDFLAGS="-framework [$1] ${LDFLAGS}"

  AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])], [iem_check_ldflags_success="yes"],[iem_check_ldflags_success="no"])

  if test "x$iem_check_ldflags_success" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_$1), [1], [framework $1])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_IEM_FRAMEWORK_$1), [1], [framework $1])
    IEM_FRAMEWORK_[]NAME[]="-framework [$1]"
    [$2]
  else
    AC_MSG_RESULT([no])
    LDFLAGS="$iem_check_ldflags_org"
    [$3]
  fi
AM_CONDITIONAL(HAVE_FRAMEWORK_[]NAME, [test "x$iem_check_ldflags_success" = "xyes"])
undefine([NAME])
])# IEM_CHECK_FRAMEWORK
