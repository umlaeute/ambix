#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# git-copyright: extract copyright-information from git
#
#
#  Copyright © 2016, IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as
#  published by the Free Software Foundation, either version 3 of the
#  License, or (at your option) any later version.
#  .
#  This program is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Affero General Public License for more details.
#  .
#  You should have received a copy of the GNU Affero General Public
#  License along with this program.  If not, see
#  <http://www.gnu.org/licenses/>.

from __future__ import print_function

import git
import sys

verbose = 0

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

author_aliases={
    "zmoelnig": "IOhannes m zmölnig",
    "IOhannes m zmoelnig": "zmoelnig",
    "iem user": "Institute of Electronic Music and Acoustics",
    "CUBEmixer": "Institute of Electronic Music and Acoustics",
    }


def commits2authordates(path):
    g = git.cmd.Git(path)
    #g.log('--name-only', '--pretty="%% %ad %an"')

    ## collect all commits
    #g.log('--pretty="%h %ad %an"', '--date=short').split("\n")

    commits=g.log('--pretty=%h').split("\n")

    filedict=dict()
    copydict=dict()
    for c in commits:
        if verbose:
            eprint("processing %s" % (c))
        author=g.show('--pretty="%an"', '-s', c).strip('"')
        date=g.show('--pretty="%ad"', '-s', '--date=short', c).strip('"')
        files=g.show('--pretty=', '--name-only', c).split("\n")
        while author in author_aliases:
            author=author_aliases[author]
        for f in files:
            f=f.strip('"')
            if not f in filedict:
                filedict[f]=dict()
            if not author in filedict[f]:
                filedict[f][author]=[]
            filedict[f][author]+=[date]
        if not author in copydict:
            copydict[author]=[]
        copydict[author]+=[date]
    return (filedict, copydict)

def copyright4files(filedict):
    for f in sorted(filedict):
        thisfile=filedict[f]
        authors=list(thisfile.keys())
        authors.sort()
        copyright=[]
        for a in authors:
            dates=thisfile[a]
            dates.sort()
            year0=dates[0].split("-")[0]
            year1=dates[-1].split("-")[0]
            if year0 == year1:
                if year0 == "2012":break
                copyright+=["%s, %s" % (year0, a)]
            else:
                copyright+=["%s-%s, %s" % (year0,year1, a)]
        if copyright:
            print("%s" % (f))
            copyright.sort()
            for c in copyright:
                print(" © %s" % (c))

def copyright4repo(copydict):
    if copydict:
        authors=list(copydict.keys())
        authors.sort()
        copyright=[]
        for a in authors:
            dates=copydict[a]
            dates.sort()
            year0=dates[0].split("-")[0]
            year1=dates[-1].split("-")[0]
            if year0 == year1:
                # if year0 == "2012":break
                copyright+=["%s, %s" % (year0, a)]
            else:
                copyright+=["%s-%s, %s" % (year0,year1, a)]
        if copyright:
            copyright.sort()
            for c in copyright:
                print(" © %s" % (c))

def main():
    import argparse
    global verbose

    parser = argparse.ArgumentParser(description='Extract copyright information from archive.')
    parser.add_argument('--verbose', '-v', action='store_true',
                        help='raise verbosity')
    parser.add_argument('--per-file', action='store_true',
                        help='show per-file copyright information (rather than per-project)')
    args = parser.parse_args()

    verbose = args.verbose

    c4f, c4r = commits2authordates(".")
    if args.per_file:
        copyright4files(c4f)
    else:
        copyright4repo(c4r)


if __name__ == '__main__':
    main()
