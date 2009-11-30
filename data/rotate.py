#!/bin/bin/python
# -*- encoding: iso-8859-1 -*-
# OpenGL ES 2.0 memory performance estimator
# Copyright (C) 2009 Nokia
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

if not len(sys.argv) == 5:
    print "Usage: %s SOURCE DEST BYTES-PER-PIXEL STRIDE" % sys.argv[0]
    sys.exit(1)

fsrc = open(sys.argv[1], "rb")
fdst = open(sys.argv[2], "wb")
bpp = int(sys.argv[3])
stride = int(sys.argv[4])

lines = []
while 1:
  line = fsrc.read(stride)
  if not line: break
  lines.append(line)

print "%dx%dx => %dx%d" % (stride / bpp, len(lines), len(lines), stride / bpp)

for x in range(0, stride, bpp):
  for line in lines:
    fdst.write(line[x:x+bpp])

fsrc.close()
fdst.close()
