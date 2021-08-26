# ReporterPointSource

A `ReporterPointSource` reads in multiple point sources from a `Reporter`.  The point source values and coordinates are updated as the `Reporter` values are changed.  

An example of a `ReporterPointSource` using a [ConstantReporter](/ConstantReporter.md)
and a `VectorPostrocessor` of type [CSVReader](/CSVReader.md) is given by:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=reporter_point_source

where the reporter data is taken from two different reporters (e.g. the coordinates from reporterData1 and the values come from reporterData2)
The two ConstantReporters are given as:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=Reporters

The `CSVReader` VectorPostrocessor is given by:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=VectorPostprocessors

reading from the following csv file:

!listing test/tests/dirackernels/reporter_point_source/point_value_file.csv

The `Reporter` and `VectorPostrocessor` for the above example produce the same `ReporterPointSource` (e.g. same magnitude and location).   

The next example applies a `ReporterPointSource` in a transient simulation given by:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp_transient.i block=DiracKernels

using the following `VectorPostprocessor` to provide x,y,z coordinates and `value_name = u`

!listing test/tests/dirackernels/reporter_point_source/2d_vpp_transient.i block=VectorPostprocessors/point_sample_out

In the above input file, the `ReporterPointSource` is applying loads at two different locations.  Note that the `PointValueSampler` has `execute_on = timestep_begin` to force the `VectorPostprocessor` to execute prior to being used by `ReporterPointSource`.  

!alert note
It is important for the `ReporterPointSource` to never use a `VectorPostprocessor` with `contains_complete_history = true`, as this can modify the ordering of the coordinates and points.  In the above input file, two locations have loads applied to them by the `ReporterPointSource`.  The load values are given by the `PointValueSampler`.

!syntax parameters /DiracKernels/ReporterPointSource

!syntax inputs /DiracKernels/ReporterPointSource

!syntax children /DiracKernels/ReporterPointSource
