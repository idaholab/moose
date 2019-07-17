#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
from optparse import OptionParser

import math
import vtk

# parse command line
p = OptionParser(usage="""usage: %prog [options] <gridfile> <vtrfile_base>
Converts the griddata file (suitable for PiecewiseMultilinear functions) into vtr format.

Eg
%prog mygrid.txt mygrid

This program produces one vtr file for each TIME found in gridfile.
So in the above example,
  mygrid0.vtr would be produced if there were no TIME info in mygrid.txt
  mygrid0.vtr mygrid1.vtr would be produced if there were two TIMEs in mygrid.txt
  etc

""")
p.add_option("-v", action="store_true",        dest="verbose",  help="Verbose")


# extract commandline arguments
(opts, args) = p.parse_args()
if len(args) != 2:
    sys.stderr.write("Number of arguments incorrect\n")
    sys.exit(1)
(grid_file, vtr_base) = args


# extract the data from the grid file
if opts.verbose:
    sys.stdout.write("Opening " + grid_file + " and extracting all data\n")
f = open(grid_file, 'r')
x = []
y = []
z = []
t = []
fcn = []
for line in f:
    if not line.strip():
        continue # blank line
    if line.strip()[0].startswith("#"):
        continue # comment line
    if line.split()[0] == "AXIS":
        getting = line.split()[1]
        continue
    if line.split()[0] == "DATA":
        getting = "DATA"
        continue
    # line must contain floats
    if getting == "X":
        x += map(float, line.split())
    elif getting == "Y":
        y += map(float, line.split())
    elif getting == "Z":
        z += map(float, line.split())
    elif getting == "T":
        t += map(float, line.split())
    elif getting == "DATA":
        fcn += map(float, line.split())
f.close()



# following makes looping easier below
# and vtr needs an (x,y,z) grid, even if
# the grid file had no x, y, or z component
if len(x) == 0:
    x = [0]
if len(y) == 0:
    y = [0]
if len(z) == 0:
    z = [0]
if len(t) == 0:
    t = [0]


# need to check for repeated values as i haven't
# considered them below
if len(x) != len(set(x)) or len(y) != len(set(y)) or len(z) != len(set(z)):
    sys.stderr.write("griddeddata_to_vtk cannot handle repeated X, Y or Z values\n")
    sys.exit(4)



if opts.verbose:
    sys.stdout.write("Creating the VTR base grid\n")
# create the vtr grid
xArray = vtk.vtkDoubleArray()
for val in sorted(x):
    xArray.InsertNextValue(val)

yArray = vtk.vtkDoubleArray()
for val in sorted(y):
    yArray.InsertNextValue(val)

zArray = vtk.vtkDoubleArray()
for val in sorted(z):
    zArray.InsertNextValue(val)


# create the vtkRectilinearGrid
grid = vtk.vtkRectilinearGrid()
grid.SetDimensions(xArray.GetNumberOfTuples(), yArray.GetNumberOfTuples(), zArray.GetNumberOfTuples())
grid.SetXCoordinates(xArray)
grid.SetYCoordinates(yArray)
grid.SetZCoordinates(zArray)
grid.Update()


# the ordering of points inside the vtkRectilinearGrid might be different
# from that in the grid file, so a map between them is needed
if opts.verbose:
    sys.stdout.write("Creating map from (x,y,z) points to index in griddeddata function values\n")
nx = len(x)
ny = len(y)
nz = len(z)
grid_index = {} # given a point, this gives the index in fcn
for k in range(nz):
    for j in range(ny):
        for i in range(nx):
            grid_index[(x[i], y[j], z[k])] = i + j*nx + k*nx*ny




# prepare the function_vals PointData
f_vals = vtk.vtkDoubleArray()
f_vals.SetName("function_vals")
f_vals.SetNumberOfValues(grid.GetNumberOfPoints())

# prepare the grid_time FieldData
grid_time = vtk.vtkDoubleArray()
grid_time.SetName("grid_time")
grid_time.InsertNextValue(0)



for i in range(len(t)):
    fname = vtr_base + str(i).zfill(int(math.log10(len(t)+1))) + ".vtr"
    if opts.verbose:
        sys.stdout.write("Outputting to " + fname + "\n");
    writer = vtk.vtkXMLRectilinearGridWriter()
    writer.SetFileName(fname)

    # insert the grid_time field data
    grid_time.InsertValue(0, t[i])
    grid.GetFieldData().AddArray(grid_time)

    # construct the function_vals and insert it
    for ptid in range(grid.GetNumberOfPoints()):
        xyz = grid.GetPoint(ptid)
        f_vals.InsertValue(ptid, fcn[grid_index[xyz] + i*nx*ny*nz])
    grid.GetPointData().AddArray(f_vals)

    # update and write the file
    grid.Update()
    writer.SetInputConnection(grid.GetProducerPort())
    writer.Write()




sys.exit(0)
