# PerfGraphData

!syntax description /Postprocessors/PerfGraphData

## Description

`PerfGraphData` retrieves performance data from the [/PerfGraph.md] and reports it as a `Postprocessor`.

The name of the section that you wish to retrieve performance data for is to be provided in the [!param](/Postprocessors/PerfGraphData/section_name) parameter. The performance data type (see [PerfGraph.md#performance-data-types] for details on the possible options) is to be provided in the [!param](/Postprocessors/PerfGraphData/data_type).

!alert warning
Most registered [PerfGraph.md] sections are not registered until the first moment they are ran. Depending on what execution flags are set (when `PerfGraphData` is executed), the system by default will error if retrieving information for a section that has not ran yet. To skip this error and return zero for sections that have not ran yet, you must set [!param](/Postprocessors/PerfGraphData/must_exist) to `false`. See [PerfGraph.md#early-retrieval] for more information.

!syntax parameters /Postprocessors/PerfGraphData

!syntax inputs /Postprocessors/PerfGraphData

!syntax children /Postprocessors/PerfGraphData

!bibtex bibliography
