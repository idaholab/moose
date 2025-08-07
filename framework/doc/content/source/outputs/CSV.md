# CSV

!syntax description /Outputs/CSV

## Overview

The CSV output object creates files containing comma separated values. Unless disabled
(see [Outputs](syntax/Outputs/index.md)) all postprocessors and scalar variables will be written to a single
file that includes a time column.

If [vector postprocessors](syntax/VectorPostprocessors/index.md) exist within the simulation, an additional set of files will be created ---
one each for every vector that exists within a `VectorPostprocessor` object for
each timestep.

This output supports an additional execution schedule, with the `execute_on` flag `MULTIAPP_FIXED_POINT_ITERATION_END`, corresponding
to the end of each successful MultiApp fixed point iteration (if there is iteration).
In this case it is recommended to set [!param](/Outputs/CSV/new_row_detection_columns) to `all`.

!syntax parameters /Outputs/CSV

!syntax inputs /Outputs/CSV

!syntax children /Outputs/CSV
