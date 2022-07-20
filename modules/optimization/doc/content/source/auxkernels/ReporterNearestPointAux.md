# ReporterNearestPointAux

!syntax description /AuxKernels/ReporterNearestPointAux

## Overview

This auxkernel uses coordinate and time data to set auxiliary variable values. This data comes in the form of vector reporter values. The required parameter [!param](/AuxKernels/ReporterNearestPointAux/value) is the name of the vector containing the values used to set the variable to. [!param](/AuxKernels/ReporterNearestPointAux/coord_x)/[!param](/AuxKernels/ReporterNearestPointAux/coord_y)/[!param](/AuxKernels/ReporterNearestPointAux/coord_z) are vectors with the x-/y-/z-coordinate data. The auxkernel will set the variable's nodal/elemental values based on the nearest point in this data. [!param](/AuxKernels/ReporterNearestPointAux/time) is the name of the time data; the kernel will linearly interpolate between two closest times for a certain coordinate, without extrapolating. [!param](/AuxKernels/ReporterNearestPointAux/coord_x), [!param](/AuxKernels/ReporterNearestPointAux/coord_y), [!param](/AuxKernels/ReporterNearestPointAux/coord_z), and [!param](/AuxKernels/ReporterNearestPointAux/time) will assume to be 0 when not specified. When specified, [!param](/AuxKernels/ReporterNearestPointAux/coord_x), [!param](/AuxKernels/ReporterNearestPointAux/coord_y), [!param](/AuxKernels/ReporterNearestPointAux/coord_z), and [!param](/AuxKernels/ReporterNearestPointAux/time) must all be the same length as [!param](/AuxKernels/ReporterNearestPointAux/value).

## Example Input File Syntax

Here are several examples different combinations of x, y, z, and time data is specified:

!listing reporter_nearest_point.i block=AuxKernels Reporters

Run this test to see how the auxkernel interpolates the data between specified times.

!syntax parameters /AuxKernels/ReporterNearestPointAux

!syntax inputs /AuxKernels/ReporterNearestPointAux

!syntax children /AuxKernels/ReporterNearestPointAux
