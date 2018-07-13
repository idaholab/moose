# PerfGraphTime

!syntax description /Postprocessors/PerfGraphTime

## Description

`PerfGraphTime` allows you to pull timing data out of the [/PerfGraph.md] and use it as a `Postprocessor`.

The inputs are a `section_name`: which is the `Section` you want the timing data for and then the `time_type` which is either `SELF`, `CHILDREN` or `TOTAL`.  These are as described in [/PerfGraph.md]: `SELF` is the time actually spent in that section, `CHILDREN` is time spent in sub-routines called by this section and `TOTAL` is the sum of the two.

!syntax parameters /Postprocessors/PerfGraphTime

!syntax inputs /Postprocessors/PerfGraphTime

!syntax children /Postprocessors/PerfGraphTime

!bibtex bibliography
