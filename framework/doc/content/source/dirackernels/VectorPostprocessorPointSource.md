# VectorPostprocessorPointSource

A `VectorPostprocessorPointSource` reads in multiple point sources from a VectorPostprocessor.  The point source values and coordinates are updated as the vectorPostprocessor values are changed.  Example syntax for a `VectorPostprocessorPointSource` used in a transient simulation is given by:

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp_transient.i block=DiracKernels

using the following VectorPostprocessor to provide x,y,z coordinates and `value_name = u`

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp_transient.i block=VectorPostprocessors/point_sample_out

Note that the `PointValueSampler` has `execute_on = timestep_begin` to force the vpp to execute prior to being used by `VectorPostprocessorPointSource`.  It is also important for the `VectorPostprocessorPointSource` to never use a vpp with `contains_complete_history = true`, as this can modify the ordering of the coordinates and points.  In the above input file, two locations have loads applied to them by the `VectorPostprocessorPointSource`.  The load values are given by the `PointValueSampler`.

!syntax parameters /DiracKernels/VectorPostprocessorPointSource

!syntax inputs /DiracKernels/VectorPostprocessorPointSource

!syntax children /DiracKernels/VectorPostprocessorPointSource
