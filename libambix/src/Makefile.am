AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/libambix

lib_LTLIBRARIES = libambix.la

#------------------------------------------------------------------------------------
# Rules for library version information:
#  http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
#  1. Start with version information of `0:0:0' for each libtool library.
#  2. Update the version information only immediately before a public release of
#     your software. More frequent updates are unnecessary, and only guarantee
#     that the current interface number gets larger faster.
#  3. If the library source code has changed at all since the last update, then
#     increment revision (`c:r:a' becomes `c:r+1:a').
#  4. If any interfaces have been added, removed, or changed since the last
#     update, increment current, and set revision to 0.
#  5. If any interfaces have been added since the last public release, then
#     increment age.
#  6. If any interfaces have been removed since the last public release, then
#     set age to 0.
#
# The following explanation may help to understand the above rules a bit better:
# consider that there are three possible kinds of reactions from users of your
# library to changes in a shared library:
# 
# 1. Programs using the previous version may use the new version as drop-in
#    replacement, and programs using the new version can also work with the
#    previous one. In other words, no recompiling nor relinking is needed. In
#    this case, bump revision only, don't touch current nor age.
# 2. Programs using the previous version may use the new version as drop-in
#    replacement, but programs using the new version may use APIs not present in
#    the previous one. In other words, a program linking against the new version
#    may fail with “unresolved symbols” if linking against the old version at
#    runtime: set revision to 0, bump current and age.
# 3. Programs may need to be changed, recompiled, relinked in order to use the
#    new version. Bump current, set revision and age to 0.

libambix_la_CPPFLAGS  = $(AM_CPPFLAGS)
libambix_la_CFLAGS    = $(AM_CFLAGS)
libambix_la_LDFLAGS   = $(AM_LDFLAGS)

libambix_la_CPPFLAGS += -DAMBIX_INTERNAL
if DEBUG
libambix_la_CPPFLAGS += -DDEBUG=1
else
libambix_la_CFLAGS   += -fvisibility=hidden
endif
libambix_la_LDFLAGS  += -version-info 0:0:0 -no-undefined
libambix_la_LIBADD    = $(LIBM)

libambix_la_OBJCFLAGS = $(libambix_la_CFLAGS)

libambix_la_SOURCES = libambix.c \
	adaptor.c \
	adaptor_acn.c \
	adaptor_fuma.c \
	matrix.c matrix_invert.c \
	utils.c \
	uuid_chunk.c \
  marker_region_chunk.c \
	private.h

if HAVE_SNDFILE
libambix_la_SOURCES += sndfile.c
libambix_la_CFLAGS += @SNDFILE_CFLAGS@
libambix_la_LIBADD += @SNDFILE_LIBS@
else !HAVE_SNDFILE
if HAVE_FRAMEWORK_AUDIOTOOLBOX
libambix_la_SOURCES += coreaudio.m
libambix_la_LIBADD += @IEM_FRAMEWORK_AUDIOTOOLBOX@ @IEM_FRAMEWORK_FOUNDATION@
else !HAVE_FRAMEWORK_AUDIOTOOLBOX
libambix_la_SOURCES += null.c
endif !HAVE_FRAMEWORK_AUDIOTOOLBOX
endif !HAVE_SNDFILE




