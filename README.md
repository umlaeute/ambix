libambix - The AMBIsonics eXchange library
==========================================

This is libambix `0.0.1`

libambix is a library of C routines for reading and writing
files following the "ambix" (AMBIsonics eXchange) conventions.

# Status

| *Check*                | *Health*                                                            |
| :--------------------- | :------------------------------------------------------------------ |
| Continuous Integration | ![Travis-CI](https://api.travis-ci.org/umlaeute/pd-iemnet.svg)      |
| Static Code Analysis   | ![Coverity Scan](https://scan.coverity.com/projects/1736/badge.svg) |
| Test Coverage          | [![codecov](https://codecov.io/gh/iem-projects/ambix/branch/master/graph/badge.svg)](https://codecov.io/gh/iem-projects/ambix) |


# INTRODUCTION
libambix is a library that reads and writes soundfiles following the "ambix"
specificiation.
Such files are:
- `CAF` (Core Audio Format) files
- *with* a special `UUID-chunk`

Audio data as output (and accepted as input) by libambix will follow the following specification:
- sample format is either `PCM16`, `PCM24`, `float32` (this one being best
  tested) and `float64`
- audio data is *interleaved*
- ambisonics channels are
	- normalization		: `SN3D`
	- channel ordering	: `ACN`

It is planned to provide conversion matrices to present the audio data in other
(common) formats namely
- Furse-Malham set (*FuMa*)
- other normalizations (*N3D*)
- other ordering (*SID*)

# Download

Get the source code (and releases) from
  https://git.iem.at/ambisonics/libambix

The source code is also mirrored to
 [GitHub](https://github.com/iem-projects/ambix/)
and (less often) to
 [SourceForge](https://sourceforge.net/p/iem/ambix/)

# API Documentation

An up-to-date API Documentation can be found at
  http://iem-projects.github.io/ambix/apiref/

# Directory layout
- `libambix/` - all components of the libambix library
- `libambix/src` - the source code for library itself.
- `libambix/ambix` - public header files for the library
- `libambix/tests` - programs which link against libambix and test its functionality.
- `utils/` - utility programs using libambix

# BUILDING from source

## DEPENDENCIES
Currently libambix uses libsndfile to read the actual file.
Due to some advanced functionality, you need at least libsndfile-1.0.26.
The current version of libsndfile can be obtained from
  https://github.com/erikd/libsndfile


## LINUX
Wherever possible, you should use the packages supplied by your Linux
distribution.

If you really do need to compile from source it should be as easy as:

    $ ./configure
    $ make
    $ make install

if you want to compile the development version of libambix, you might need
to run the following *before* any of the above:

    $ ./autogen.sh

## UNIX
Compile as for Linux.


## Win32/Win64
The default Windows compilers (Microsoft's Visual Studio) are nowhere near
compliant with the 1999 ISO C Standard and hence not able to compile libambix.

Please use the libambix binaries available on the ambix web site.


## MacOSX
Building on MacOSX should be the same as building it on any other Unix.


# CONTACT

libambix was written by IOhannes m zmölnig at the Institute of Electronic Music
and Acoustics (IEM), and the University of Music and Performing Arts (KUG), Graz,
Austria
The libambix home page is at :

	http://git.iem.at/ambisonics/libambix
