#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

#
# Converts vtrfile info to griddata format
# See doco below
#

import os
import sys
import time
from optparse import OptionParser

import vtk

# parse command line
p = OptionParser(usage="""usage: %prog [options] <vtrfile1> <vtrfile2> ... <vtrfileN> <gridfile>
Converts the vtrfile info into griddata format suitable for PiecewiseMultilinear functions.
Each vtrfile must contain PointData with name "function_vals".

Usually this program is run with just one vtrfile, eg
%prog mygrid.vtr mygrid.txt

The reason for running with more than one vtrfile is to put time-dependent
function values into gridfile.  In this case, each vtrfile must:
 - be defined using the same grid
 - contain FieldData with name "grid_time" that defines the time value
 - be passed to this program in order of increasing time (this is just
      to reduce unexpected results due to accidents)
Eg
%prog data_tsmall.vtr data_tbig.vtr griddeddata.txt

""")
p.add_option("-v", action="store_true",        dest="verbose",  help="Verbose")


# extract commandline arguments
(opts, args) = p.parse_args()
if len(args) < 2:
    sys.stderr.write("Number of arguments incorrect\n")
    sys.exit(1)
vtr_files = args[0:-1]
grid_file = args[-1]


# read vtr data
r = {}
for vtr_file in vtr_files:
    if opts.verbose: sys.stdout.write("Reading " + vtr_file + "\n");
    if vtr_file.endswith(".vtr"):
        r[vtr_file] = vtk.vtkXMLRectilinearGridReader()
    else:
        sys.stderr.write("Not currently configured to read file of type " + vtr_file + "\n");
        sys.exit(2)
    r[vtr_file].SetFileName(vtr_file)
    r[vtr_file].Update()
    r[vtr_file] = r[vtr_file].GetOutput()


# get function_vals and check it exists
if opts.verbose: sys.stdout.write("Extracting function_vals\n")
fcn_vals = {}
for vtr_file in vtr_files:
    fcn_vals[vtr_file] = r[vtr_file].GetPointData().GetArray("function_vals")
    if not fcn_vals[vtr_file]:
        sys.stderr.write(vtr_file + " does not have PointData called function_vals\n")
        sys.exit(3)


# extract time info
grid_times = {}
if len(vtr_files) > 1:
    if opts.verbose: sys.stdout.write("Extracting grid_time\n")
    for vtr_file in vtr_files:
        grid_times[vtr_file] = r[vtr_file].GetFieldData().GetAbstractArray("grid_time")
        if not grid_times[vtr_file] or grid_times[vtr_file].GetNumberOfTuples() != 1:
            sys.stderr.write(vtr_file + " does not have FieldData called grid_time\n")
            sys.exit(4)
        grid_times[vtr_file] = grid_times[vtr_file].GetValue(0)
    # check monotonicity
    for i in range(1,len(vtr_files)):
        if grid_times[vtr_files[i]] <= grid_times[vtr_files[i-1]]:
            sys.stderr.write("You must re-order your vtrfile inputs so that grid_times are monotonically increasing.\nCurrently they are: " + " ".join([str(grid_times[vtr_file]) for vtr_file in vtr_files]) + "\n")
            sys.exit(5)


f = open(grid_file, 'w')


# write header info
if opts.verbose: sys.stdout.write("Writing header info into " + grid_file + "\n");
f.write("# This file was generated using the command\n")
f.write("# " + ' '.join(sys.argv) + "\n")

f.write("# Working directory\n")
f.write("# " + os.getcwd() + "\n")

f.write("# This file was generated at this time\n")
f.write("# " + time.asctime(time.localtime(time.time())) + "\n")

for vtr_file in vtr_files:
    f.write("#\n")
    fd = r[vtr_file].GetFieldData()
    if fd:
        f.write("# Field data from " + vtr_file + "\n")
        for fd_id in range(fd.GetNumberOfArrays()):
            the_fd = fd.GetAbstractArray(fd_id)
            f.write("# name=\n" + "#" + the_fd.GetName() + "\n")
            vals = [str(the_fd.GetValue(pt)) for pt in range(the_fd.GetNumberOfTuples())]  # note for future: not sure what happens with vector/tensors here
            f.write("# values=\n#" + "\n#".join(vals) + "\n")
        f.write("# End field data\n")
        f.write("#\n")



# write the grid and check it is the same for each vtr file
if opts.verbose: sys.stdout.write("Writing grid to " + grid_file + "\n");

grid_coords = [r[vtr_files[0]].GetXCoordinates().GetValue(i) for i in range(r[vtr_files[0]].GetXCoordinates().GetNumberOfTuples())]
if r[vtr_files[0]].GetXCoordinates().GetNumberOfTuples() > 1:
    f.write("AXIS X\n")
    f.write(" ".join(map(str, grid_coords)) + "\n");
for vtr_file in vtr_files[1:]:
    grid_coords_check = [r[vtr_file].GetXCoordinates().GetValue(i) for i in range(r[vtr_file].GetXCoordinates().GetNumberOfTuples())]
    if grid_coords != grid_coords_check:
        sys.stderr.write("Grid coordinates must be the same in each vtr_file.\nThey are: " + " ".join(map(str, grid_coords)) + "\nand: " + " ".join(map(str, grid_coords_check)) + "\n")
        sys.exit(7)

grid_coords = [r[vtr_files[0]].GetYCoordinates().GetValue(i) for i in range(r[vtr_files[0]].GetYCoordinates().GetNumberOfTuples())]
if r[vtr_files[0]].GetYCoordinates().GetNumberOfTuples() > 1:
    f.write("AXIS Y\n")
    f.write(" ".join(map(str, grid_coords)) + "\n");
for vtr_file in vtr_files[1:]:
    grid_coords_check = [r[vtr_file].GetYCoordinates().GetValue(i) for i in range(r[vtr_file].GetYCoordinates().GetNumberOfTuples())]
    if grid_coords != grid_coords_check:
        sys.stderr.write("Grid coordinates must be the same in each vtr_file.\nThey are: " + " ".join(map(str, grid_coords)) + "\nand: " + " ".join(map(str, grid_coords_check)) + "\n")
        sys.exit(7)

grid_coords = [r[vtr_files[0]].GetZCoordinates().GetValue(i) for i in range(r[vtr_files[0]].GetZCoordinates().GetNumberOfTuples())]
if r[vtr_files[0]].GetZCoordinates().GetNumberOfTuples() > 1:
    f.write("AXIS Z\n")
    f.write(" ".join(map(str, grid_coords)) + "\n");
for vtr_file in vtr_files[1:]:
    grid_coords_check = [r[vtr_file].GetZCoordinates().GetValue(i) for i in range(r[vtr_file].GetZCoordinates().GetNumberOfTuples())]
    if grid_coords != grid_coords_check:
        sys.stderr.write("Grid coordinates must be the same in each vtr_file.\nThey are: " + " ".join(map(str, grid_coords)) + "\nand: " + " ".join(map(str, grid_coords_check)) + "\n")
        sys.exit(7)



# do the time data, if appropriate
if len(vtr_files) > 1:
    f.write("AXIS T\n")
    f.write(" ".join(map(str, [grid_times[v] for v in vtr_files])) + "\n")



# write the function values
if opts.verbose: sys.stdout.write("Writing function_vals to " + grid_file + "\n");
f.write("DATA\n");
for v in vtr_files:
    if len(vtr_files) > 1:
        f.write("# time = " + str(grid_times[v]) + "\n")
    fcn_dict = {}
    for pt_id in range(r[v].GetNumberOfPoints()):
        fcn_dict[r[v].GetPoint(pt_id)] = fcn_vals[v].GetValue(pt_id)

    for k in range(r[v].GetZCoordinates().GetNumberOfTuples()):
        z = r[v].GetZCoordinates().GetValue(k)
        for j in range(r[v].GetYCoordinates().GetNumberOfTuples()):
            y = r[v].GetYCoordinates().GetValue(j)
            for i in range(r[v].GetXCoordinates().GetNumberOfTuples()):
                x = r[v].GetXCoordinates().GetValue(i)
                f.write(str(fcn_dict[tuple([x, y, z])]) + "\n");

f.close()

sys.exit(0)
