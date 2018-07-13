# PerfGraphData

!syntax description /Postprocessors/PerfGraphData

## Description

`PerfGraphData` allows you to pull timing data out of the [/PerfGraph.md] and use it as a `Postprocessor`.

The inputs are a `section_name`: which is the `Section` you want the timing data for and then the `time_type` which is one of `SELF CHILDREN TOTAL SELF_AVG CHILDREN_AVG TOTAL_AVG SELF_PERCENT CHILDREN_PERCENT TOTAL_PERCENT CALLS`.  These are as described in [/PerfGraph.md]: `SELF` is the time actually spent in that section, `CHILDREN` is time spent in sub-routines called by this section, `TOTAL` is the sum of the two.  `AVG` is the average time in that section while `PERCENT` is the percentage of the total time.

!syntax parameters /Postprocessors/PerfGraphData

!syntax inputs /Postprocessors/PerfGraphData

!syntax children /Postprocessors/PerfGraphData

!bibtex bibliography
