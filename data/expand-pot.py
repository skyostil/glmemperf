#!/bin/bin/python
# -*- encoding: iso-8859-1 -*-
# OpenGL ES 2.0 memory performance estimator
# Copyright (C) 2010 Nokia
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# \author Sami Kyöstilä <sami.kyostila@nokia.com>
#
import sys

def nextPowerOfTwo(x):
    n = 1
    while n < x:
        n = n << 1
    return n

if not len(sys.argv) == 5:
    print "Usage: %s SOURCE DEST BYTES-PER-PIXEL STRIDE" % sys.argv[0]
    sys.exit(1)

fsrc = open(sys.argv[1], "rb")
fdst = open(sys.argv[2], "wb")
bpp = int(sys.argv[3])
stride = int(sys.argv[4])

newStride = nextPowerOfTwo(stride / bpp) * bpp

padding = "\x00" * (newStride - stride)
lines = 0
while 1:
  line = fsrc.read(stride)
  if not line: break
  lines += 1
  fdst.write(line + padding)

newLines = nextPowerOfTwo(lines)
padding = "\x00" * newStride
fdst.write(padding * (newLines - lines))

fsrc.close()
fdst.close()
