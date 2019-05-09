# PerProcessorPerfGraphData

!syntax description /VectorPostprocessors/PerProcessorPerfGraphData

# Description

Similar to [PerfGraphData.md] this `VectorPostprocessor` allows you to obtain timing data from every processor individually.  The data to be captured is set by using `section_names` to specify the [PerfGraph.md] sections and the `data_type` to specify the column name from the `PerfGraph`.

As with any `VectorPostprocessor` the data can be output to a CSV file using `csv = true` in the `Outputs` block.  Those CSV files can then be easily viewed using `Peacock`.

This object can be combined with [VectorPostprocessorVisualizationAux.md] to visualize the timing on the mesh.  In addition [StatisticsVectorPostprocessor.md] is useful to compute min/max/sum/standard deviation of the timings.

!syntax parameters /VectorPostprocessors/PerProcessorPerfGraphData

!syntax inputs /VectorPostprocessors/PerProcessorPerfGraphData

!syntax children /VectorPostprocessors/PerProcessorPerfGraphData

!bibtex bibliography
