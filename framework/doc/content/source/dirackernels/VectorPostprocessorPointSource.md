# VectorPostprocessorPointSource

A `VectorPostprocessorPointSource` reads in multiple point sources from a `VectorPostprocessor` or `Reporter`.  The point source values and coordinates are updated as the `VectorPostprocessor` or `Reporter` values are changed.  

An example of a `VectorPostprocessorPointSource` using a `Reporter` of type [ConstantReporter](/ConstantReporter.md)
and a `VectorPostrocessor` of type [CSVReader](/CSVReader.md) is given by

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp.i block=reporter_point_source

where the reporter data is taken from two different reporters (e.g. the coordinates from reporterData1 and the values come from reporterData2)
The two ConstantReporters are given as:

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp.i block=Reporters

The `CSVReader` VectorPostrocessor is given by:

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp.i block=VectorPostprocessors

reading from the following csv file:

!listing test/tests/dirackernels/vectorPostprocessor_point_source/point_value_file.csv

The `Reporter` and `VectorPostrocessor` for the above example produce the same `VectorPostprocessorPointSource` (e.g. same magnitude and location).  Different `VectorPostrocessor`s can be used to provide data to the `VectorPostprocessorPointSource` using the Reporter format where the `vector_postprocessor` name is left out and the Reporter consumer format is used (e.g. <vector_postprocessor>/<vector_name>).  

!alert note
A `vector_postprocessor` name should not be given when using the `Reporter` input format.

The next example applies a `VectorPostprocessorPointSource` in a transient simulation given by:

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp_transient.i block=DiracKernels

using the following `VectorPostprocessor` to provide x,y,z coordinates and `value_name = u`

!listing test/tests/dirackernels/vectorPostprocessor_point_source/2d_vpp_transient.i block=VectorPostprocessors/point_sample_out

Note that the `PointValueSampler` has `execute_on = timestep_begin` to force the `VectorPostprocessor` to execute prior to being used by `VectorPostprocessorPointSource`.  

!alert note
It is important for the `VectorPostprocessorPointSource` to never use a `VectorPostprocessor` with `contains_complete_history = true`, as this can modify the ordering of the coordinates and points.  In the above input file, two locations have loads applied to them by the `VectorPostprocessorPointSource`.  The load values are given by the `PointValueSampler`.

!syntax parameters /DiracKernels/VectorPostprocessorPointSource

!syntax inputs /DiracKernels/VectorPostprocessorPointSource

!syntax children /DiracKernels/VectorPostprocessorPointSource
