#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import sys
from optparse import OptionParser, OptionValueError
import vtk

p = OptionParser(usage="""usage: %prog [options] <out_vtr>
Creates an undecorated rectilinear grid (.vtr file)
""")
p.add_option("-x", "--xgrid", action="store", type="str", default="-1:1:2", dest="xgrid", help="X grid specified as xstart:xstop:number or x0,x1,x2,... (Default = %default)")
p.add_option("-y", "--ygrid", action="store", type="str", default="-1:1:2", dest="ygrid", help="Y grid specified as ystart:ystop:number or y0,y1,y2,... (Default = %default)")
p.add_option("-z", "--zgrid", action="store", type="str", default="0", dest="zgrid", help="Z grid specified as zstart:xstop:number or z0,z1,z2,... (Default = %default)")
p.add_option("-v", action="store_true", dest="verbose",  help="Verbose")
p.add_option("-a", action="store_true", dest="ascii",  help="ASCII, instead of binary, .vtr output")
(opts, args) = p.parse_args()

# get the output filename
if len(args) != 1:
   p.print_help()
   print "Incorrect number of agruments"
   sys.exit(1)
(out_vtr) = args[0]

if not out_vtr.endswith(".vtr"):
   p.print_help()
   print "\nYour out_vtr filename must end with .vtr"
   sys.exit(2)

if opts.verbose: print "Creating grid of points"

xArray = vtk.vtkDoubleArray()
min_max_num = opts.xgrid.split(":")
if len(min_max_num) == 3:
   (x0, x1, nx) = (float(min_max_num[0]), float(min_max_num[1]), int(min_max_num[2]))
   x_coords = [x0 + i*(x1 - x0)/float(nx-1) for i in range(nx)]
else:
   try:
      x_coords = sorted(map(float, opts.xgrid.split(",")))
   except:
      p.print_help()
      print "\nYour specification", opts.xgrid, "has incorrect format"
      sys.exit(2)
for x in x_coords:
   xArray.InsertNextValue(x)

yArray = vtk.vtkDoubleArray()
min_max_num = opts.ygrid.split(":")
if len(min_max_num) == 3:
   (y0, y1, ny) = (float(min_max_num[0]), float(min_max_num[1]), int(min_max_num[2]))
   y_coords = [y0 + i*(y1 - y0)/float(ny-1) for i in range(ny)]
else:
   try:
      y_coords = sorted(map(float, opts.ygrid.split(",")))
   except:
      p.print_help()
      print "\nYour specification", opts.ygrid, "has incorrect format"
      sys.exit(2)
for y in y_coords:
   yArray.InsertNextValue(y)

zArray = vtk.vtkDoubleArray()
min_max_num = opts.zgrid.split(":")
if len(min_max_num) == 3:
   (z0, z1, nz) = (float(min_max_num[0]), float(min_max_num[1]), int(min_max_num[2]))
   z_coords = [z0 + i*(z1 - z0)/float(nz-1) for i in range(nz)]
else:
   try:
      z_coords = sorted(map(float, opts.zgrid.split(",")))
   except:
      p.print_help()
      print "\nYour specification", opts.zgrid, "has incorrect format"
      sys.exit(2)
for z in z_coords:
   zArray.InsertNextValue(z)

grid = vtk.vtkRectilinearGrid()
grid.SetDimensions(len(x_coords), len(y_coords), len(z_coords))
grid.SetXCoordinates(xArray)
grid.SetYCoordinates(yArray)
grid.SetZCoordinates(zArray)
grid.Update()

if opts.verbose: print "Writing", out_vtr
writer = vtk.vtkXMLRectilinearGridWriter()
writer.SetFileName(out_vtr)
writer.SetDataModeToBinary()
writer.SetInputConnection(grid.GetProducerPort())
writer.Write()

sys.exit(0)
