# CSV

!syntax description /Outputs/CSV

## Overview

The CSV output object creates files containing comma separated values. Unless disabled
(see [syntax/Outputs/index.md) all postprocessors and scalar variables will be written to a single
file that includes a time column.

If vector postprocessors exist within the simulation an additional set of files will be created,
there will be one file for every vector that exists within a VectorPostprocessor object for
each timestep.

!syntax parameters /Outputs/CSV

!syntax inputs /Outputs/CSV

!syntax children /Outputs/CSV
