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
import math
import vtk

cohesion = 1.0
friction_angle = 20 * math.pi / 180.0
sinphi = math.sin(friction_angle)
cosphi = math.sin(friction_angle)
cohcos = cohesion * cosphi
dp_c = 3.0
dp_phi = math.pi / 6.0
dp_sinphi = math.sin(dp_phi)
dp_cosphi = math.cos(dp_phi)
dp_t = 0.0
dp_tc = 2.0

def ismoother(f_diff):
   if (abs(f_diff) >= opts.smoothing_tol):
      return 0.0
   return 0.5 * (opts.smoothing_tol - abs(f_diff)) - opts.smoothing_tol / math.pi * math.cos(0.5 * math.pi * f_diff / opts.smoothing_tol)

def yield_function_2(yf1, yf2):
   return max(yf1, yf2) + ismoother(yf1 - yf2)

def yield_function(x, y, z):
   yfs = []
   if opts.twoD_example:
      yfs += [- x, - y, y - 1.0, - y - 0.5 + 0.5 * x]
   if opts.twoD_example_alternative:
      yfs += [y - 1.0, - y - 0.5 + 0.5 * x, - x, - y]
   if opts.dp:
      yfs += [y + x * dp_sinphi - dp_c * dp_cosphi, x - dp_t, -x - dp_tc]
   if opts.tensile:
      yfs += [x - opts.tensile_strength, y - opts.tensile_strength, z - opts.tensile_strength]
   if opts.mc:
      yfs += [0.5 * (x - z) + 0.5 * (x + z) * sinphi - cohcos,
              0.5 * (y - z) + 0.5 * (y + z) * sinphi - cohcos,
              0.5 * (x - y) + 0.5 * (x + y) * sinphi - cohcos,
              0.5 * (y - x) + 0.5 * (x + y) * sinphi - cohcos,
              0.5 * (z - y) + 0.5 * (y + z) * sinphi - cohcos,
              0.5 * (z - x) + 0.5 * (x + z) * sinphi - cohcos]
   yf = yfs[0]
   for i in range(1, len(yfs)):
      yf = yield_function_2(yf, yfs[i])
   return yf


# parse command line
p = OptionParser(usage="""usage: %prog [options] <vtk_file>
Inserts yield function values into <vtk_file>.
Only 3D input is accepted: this program assumes that the individual yield functions are functions of x, y, z.
""")
p.add_option("-v", action="store_true",        dest="verbose",  help="Verbose")
p.add_option("--name", action="store", type="string", default="yield_function", dest="name", help="The pointdata produced will have this name.  Default=%default")
p.add_option("--smoothing_tol", action="store", type="float", default=0.1, dest="smoothing_tol", help="The smoothing tolerance (a) parameter.  Default=%default")
p.add_option("-t", action="store_true", dest="tensile", help="Yield function will contain contributions from tensile (Rankine) failure")
p.add_option("--tensile_strength", action="store", type="float", default=0.7, dest="tensile_strength", help="Tensile strength")
p.add_option("-m", action="store_true", dest="mc", help="Yield function will contain contributions from Mohr-Coulomb failure")
p.add_option("-d", action="store_true", dest="dp", help="Yield function will contain contributions from Drucker-Prager failure")
p.add_option("-e", action="store_true", dest="twoD_example", help="Yield function will contain contributions from an example 2D yield function")
p.add_option("-a", action="store_true", dest="twoD_example_alternative", help="Yield function will contain contributions from an alternative example 2D yield function")
(opts, args) = p.parse_args()

# get the com filename
if len(args) != 1:
   p.print_help()
   sys.exit(1)
in_file = args[0]

if opts.verbose: print "Reading", in_file
if in_file.endswith(".vtp"):
   indata = vtk.vtkXMLPolyDataReader()
   writer = vtk.vtkXMLPolyDataWriter()
elif in_file.endswith(".vtu"):
   indata = vtk.vtkXMLUnstructuredGridReader()
   writer = vtk.vtkXMLUnstructuredGridWriter()
elif in_file.endswith(".vtr"):
   indata = vtk.vtkXMLRectilinearGridReader()
   writer = vtk.vtkXMLRectilinearGridWriter()
else:
   print "This program has not yet been configured to read files of type", in_file
   sys.exit(2)


indata.SetFileName(in_file)
indata.Update()
indata = indata.GetOutput()

if opts.verbose: print "Generating", opts.name

yf = vtk.vtkDoubleArray()
yf.SetName(opts.name)
yf.SetNumberOfValues(indata.GetNumberOfPoints())
for ptid in range(indata.GetNumberOfPoints()):
   (x, y, z) = indata.GetPoint(ptid)
   yf.SetValue(ptid, yield_function(x, y, z))
indata.GetPointData().AddArray(yf)

if opts.verbose: print "Writing", in_file
writer.SetFileName(in_file)
writer.SetDataModeToBinary()
writer.SetInputConnection(indata.GetProducerPort())
writer.Write()

sys.exit(0)
