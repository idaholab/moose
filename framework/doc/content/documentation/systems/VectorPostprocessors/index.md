# VectorPostprocessors System

The VectorPostprocessors (VPP) System is designed to compute multiple related values each time it is executed.  It can be thought of as a Postprocessor that outputs multiple values. For example, if you'd like to sample a solution field across a line, a VPP is a good choice
(See [PointValueSampler](PointValueSampler.md)).  In addition to outputting the values along the line a VPP can actually output multiple vectors simultaneously.  Each vector is given a name, which is the column name.  Together, all of the vectors a VPP outputs are output in one CSV file (usually per-timestep).

## Design

The VPP system builds off from MOOSE's [UserObject](/UserObjects/index.md) system. Each VPP contains the full interface of a UserObject but
is also expected to declare one or more vectors that will be populated and output by each VPP. There are no restrictions on the lengths of
these vectors and the state of these vectors is managed by MOOSE and is automatically "restartable".

Vectors are declared with the `declareVector` method:

!listing framework/include/vectorpostprocessors/VectorPostprocessor.h line=declareVector

This method returns a writable reference to a VectorPostprocessorValue that must be captured and stored in the object.

!listing framework/include/utils/MooseTypes.h line=VectorPostprocessorValue

!alert! note
Developers are responsible for sizing these vectors as needed.
!alert-end!

### Output

VPP data can vary depending on the type of data being output. Again, the the "sample over line" example mentioned in the introduction,
a complete set of values will be generated each time the VPP is executed. The VPP system handles this scenario by creating seperate output
files for each invocation. The form of the output is as follows:

```
<filebase>_<vector name>_<serial number>.csv

# filebase - the normal output file base
# vector name - the name of the vector postprocessor (normally the input block name for that VPP)
# serial number - a fixed-width (normally four digit) number starting from 0000 and counting up.
```

In some cases however, a VPP may be accumulating information in time. For example, a user may wish to track values at several locations
over time. The output might consist of the coordinates of those positions and the sampled value. In such a scenario, the default seperate
file output may be cumbersome as each file would effectively have a single line so a script to aggregate the information from all of the
seperate output files may need to be used. Instead, MOOSE supports an option, which may be of use in these cases:

```
contains_complete_history = true
```

By setting this value, you are telling MOOSE that the values in all of the vectors of the given VPP are cummulative. MOOSE will take
advantage of this information in multiple ways. First, it will turn off writing the output to seperate files and will drop the serial
number from the output filename format altogether. Secondly, it will ignore any changed values in the vectors only outputting the newest
rows in each vector postprocessor.

### Parallel Assumptions

At the current time MOOSE makes no assumptions about the parallel integrity of VPP vectors. Developers are free to choose whether inforamtion
is gathered only on the root processors (required since output occurs on rank zero), or if information is replicated across all processors.
Replicating information across all processors may limit scalability of larger jobs and may also just be unecessary and wasteful. However, when
VPP vectors are used in conjuction with the "Transfers" system, in particular [MultiAppVectorPostprocessorTransfer](MultiAppVectorPostprocessorTransfer.md),
replicating information may be the most straightforward way to avoid missing data.

Design is ongoing on having MOOSE assist with replicating information for the user based on usage and necessity. Watch for future developments
in this area.

# VectorPostprocessor List

!syntax list /VectorPostprocessors objects=True actions=False subsystems=False

!syntax list /VectorPostprocessors objects=False actions=False subsystems=True

!syntax list /VectorPostprocessors objects=False actions=True subsystems=False
