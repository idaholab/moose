# Exodus

!syntax description /Outputs/Exodus

## Overview

The Exodus output object is the preferred way to write out simulation results.  It creates ExodusII formatted files that are easily read by Peacock, Paraview, Visit and other postprocessing applications.

ExodusII has many benefits over other formats.  It is a binary format based on NetCDF so the files are compact.  It also can store multiple timesteps worth of information within one file, reducing output file clutter and storage for simulation results.  Only if the mesh changes (adaptivity, movement if outputting the displaced mesh) will a new file need to be written.

Most of the time to do Exodus output you can simply set `exodus = true` in your `Outputs` block.  For more control you can add sub-block of `Outputs` and set `type = Exodus`.  That gives you full access to the parameters described below.

## Advanced Parameters

### `output_dimension`

The `output_dimension` parameter allows you to override the default selection for the dimensionality of the output.  This is normally not needed (MOOSE can usually figure out what the dimensionality should be), but there are special cases where you might want to set this option.  In particular, if you are running a 2D simulation that is generating 3D displacement fields you will need to use `output_dimension = 3` to force the dimension so that Peacock and Paraview can properly render those displacements.

!syntax parameters /Outputs/Exodus

!syntax inputs /Outputs/Exodus

!syntax children /Outputs/Exodus
