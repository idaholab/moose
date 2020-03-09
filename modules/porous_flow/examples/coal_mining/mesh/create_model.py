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
import subprocess
import math
from optparse import OptionParser, OptionValueError


# parse command line
p = OptionParser(usage="""usage: %prog [options] <plan_mesh_vtp> <output_basename>
Creates an exodus and vtk mesh called output_basename.e and output_basename.vtu
""")
p.add_option("-v", action="store_true", dest="verbose",  help="Verbose output")
p.add_option("--seam_thickness", action="store", type=float, default=3.0, dest="seam_thickness", help="Thickness of the coal seam.  Default=%default")
p.add_option("--bias", action="store", type=float, default=1.1, dest="bias", help="Bias of the vertical sizes of the elements.  b>=1.  Default=%default")
p.add_option("--base_elevation", action="store", type=float, default=-400.0, dest="base_elevation", help="Elevation of the base of the model.  Default=%default")
(opts, args) = p.parse_args()
if len(args) != 2:
   p.print_help()
   sys.exit(1)
(plan_mesh_vtp, output_base_name) = args

if opts.verbose: sys.stdout.write("Determining the vertical mesh\n")
floor_elevations = []
if opts.bias < 1.0:
   sys.stderr.write("Bias must be >=1\n")
   sys.exit(1)
elif opts.bias == 1.0:
   num_eles = int(round(-opts.base_elevation - opts.seam_thickness) / opts.seam_thickness)
   ele_height = (-opts.base_elevation - opts.seam_thickness) / num_eles
   floor_elevations = [opts.base_elevation, opts.base_elevation + opts.seam_thickness]
   floor_elevations += [opts.base_elevation + opts.seam_thickness + i * ele_height for i in range(num_eles)]
   floor_elevations += [0.0]
else:
   num_eles = int(round(math.log(1.0 - ((- opts.base_elevation - opts.seam_thickness) / opts.seam_thickness * (1.0 - opts.bias))) / math.log(opts.bias)))
   floor_elevations = [opts.base_elevation, opts.base_elevation + opts.seam_thickness]
   h = (-opts.base_elevation - opts.seam_thickness) * (1.0 - opts.bias) / (1.0 - math.pow(opts.bias, num_eles))
   for i in range(num_eles - 1):
      floor_elevations.append(floor_elevations[-1] + h)
      h *= opts.bias
   floor_elevations.append(0.0)



vtps = ""
for vp in floor_elevations:
   cmd = "vtk_trans.py"
   cmd += " -z " + str(vp) + "  -- "
   cmd += plan_mesh_vtp
   fn = " tmp" + str(vp) + ".vtp"
   cmd += fn
   vtps += fn
   if opts.verbose: sys.stdout.write(" Creating vtp at elevation " + str(vp) + "\n")
   if subprocess.call(cmd, shell = True) != 0:
      sys.stderr.write("Failed to create vtp at elevation " + str(vp) + "\n")
      sys.exit(1)

if opts.verbose: sys.stdout.write("Creating 3D mesh " + output_base_name + ".vtu\n")
cmd = "vtk_sweep_solid.py -r" + vtps + " -- " + output_base_name + ".vtu"
if subprocess.call(cmd, shell = True) != 0:
   sys.stderr.write("Failed to create vtu " + output_base_name + ".vtu\n")
   sys.exit(1)

if opts.verbose: sys.stdout.write("Removing temporary vtp files\n")
for vp in floor_elevations:
   os.remove("tmp" + str(vp) + ".vtp")

if opts.verbose: sys.stdout.write("Creating 3D mesh " + output_base_name + ".e\n")
cmd = "vtk_to_exodus.py"
cmd += " -- " + output_base_name + ".vtu " + output_base_name + ".e"
if subprocess.call(cmd, shell = True) != 0:
   sys.stderr.write("Failed to create exodus file " + output_base_name + ".e\n")
   sys.exit(1)

sys.exit(0)
