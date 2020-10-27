# VectorPostprocessors System

The VectorPostprocessors (VPP) System is designed to compute multiple related values each time it is executed.  It can be thought of as a Postprocessor that outputs multiple values. For example, if you'd like to sample a solution field across a line, a VPP is a good choice
(See [PointValueSampler](PointValueSampler.md)).  In addition to outputting the values along the line a VPP can actually output multiple vectors simultaneously.  Each vector is given a name, which is the column name.  Together, all of the vectors a VPP outputs are output in one CSV file (usually per-timestep).

!alert note Consider using a Reporter Object
The [Reporters/index.md] is a newer, more flexible system for computing aggregate values. It is recommended
that new objects for aggregate calculations use the Reporter system.

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

## Output

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
over time. The output might consist of the coordinates of those positions and the sampled value. In such a scenario, the default separate
file output may be cumbersome as each file would effectively have a single line so a script to aggregate the information from all of the
separate output files may need to be used. Instead, MOOSE supports an option, which may be of use in these cases:

```
contains_complete_history = true
```

By setting this value, you are telling MOOSE that the values in all of the vectors of the given VPP are cummulative. MOOSE will take
advantage of this information in multiple ways. First, it will turn off writing the output to seperate files and will drop the serial
number from the output filename format altogether. Secondly, it will ignore any changed values in the vectors only outputting the newest
rows in each vector postprocessor.

### Parallel Assumptions

VectorPostprocessors are required to create their complete vectors on processor zero (rank 0).  They should use the `_communicator` to reduce their values to processor zero.  Objects that use VPPs must specify how they need the data by calling the `getVectorPostprocessorValue()` or `getScatterVectorPostprocessorValue()` functions with the correct arguments.

If the object needing VPP values only needs those values on processor zero it should call:

```c++
getVectorPostprocessorValue('the_vpp_parameter_name', 'the_vector_name', false)
```

The `false` in that call tells MOOSE that this object does NOT need the vector to be "broadcast" (i.e. "replicated).

If this object does indeed need the VPP data to be broadcast (replicated on all processors) it should make this call:

```c++
getVectorPostprocessorValue('the_vpp_parameter_name', 'the_vector_name', true)
```

In the very special case that a VPP is producing vectors that are `num_procs` length an object can ask for the value of a VPP to be "scattered" - which means that each processor will receive only the value that corresponds to it's rank (i.e. `_values[rank]`).  This is accomplished by calling:

```c++
getScatterVectorPostprocessorValue('the_vpp_parameter_name', 'the_vector_name')
```

`getScatterVectorPostprocessorValue()` returns a `const ScatterVectorPostprocessorValue &`... which is a single scalar value that you don't index into.  Each process receives the "correct" value and can just directly use it.


If the data in a VPP is naturally replicated on all processors a VectorPostprocessor should set `_auto_broadcast = false` in its `validParams()` like so:

```c++
params.set<bool>("_auto_broadcast") = "false";
```

This tells MOOSE that the data is already replicated and there is no need to broadcast it if another object is asking for it to be broadcast.

## TimeData

The `time_data` parameter produces an additional CSV file containing just the real time and the corresponding time step for any VectorPostprocessor output information. This file may be useful in producing animations or your simulation results.

# VectorPostprocessor List

!syntax list /VectorPostprocessors objects=True actions=False subsystems=False

!syntax list /VectorPostprocessors objects=False actions=False subsystems=True

!syntax list /VectorPostprocessors objects=False actions=True subsystems=False
