# ReporterPointSource

A `ReporterPointSource` reads in multiple point sources from a `Reporter`.  The point source values and coordinates are updated as the `Reporter` values are changed.

!alert warning
Duplicated points, i.e. points with the same xyz coordinates, are dropped by [DiracKernels](/DiracKernels/index.md) and applied as a single point.  The input parameter [!param](/DiracKernels/ReporterPointSource/combine_duplicates) combines the values and weights of duplicated points when set to `True`.  Reporters containing duplicate points will produce an error when set to `False`.  The parameter `drop_duplicate_points` used by other DiracKernels to handle duplicate points is suppressed for the `ReporterPointSource` because it is expected that every duplicate point in a `ReporterPointSource` will have different value and weight and are not just multiples of the sames value.

An example of a `ReporterPointSource` using a [ConstantReporter](/ConstantReporter.md)
and a `VectorPostprocessor` of type [CSVReaderVectorPostprocessor](/CSVReaderVectorPostprocessor.md) is given by:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=reporter_point_source

The ConstantReporter provides the following data:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=Reporters/reporterData

The `CSVReaderVectorPostprocessor` is given by:

!listing test/tests/dirackernels/reporter_point_source/2d_vpp.i block=VectorPostprocessors

reading from the following csv file:

!listing test/tests/dirackernels/reporter_point_source/point_value_file.csv

The `Reporter` and `VectorPostprocessor` for the above example produce the same `ReporterPointSource` (e.g. same magnitude and location).   

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
