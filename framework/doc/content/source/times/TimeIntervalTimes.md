# TimeIntervalTimes

This object specifies times with a fixed time interval between a start time and
end time. The start time and end time may be provided or if not, taken to be
the start time and end time from the simulation (in this case, the executioner type
must be [Transient.md]). If the end time does not fall exactly at the end of the
end of the last time interval, then the end time is only included if the parameter
[!param](/Times/TimeIntervalTimes/always_include_end_time) is set to `true`.

!syntax parameters /Times/TimeIntervalTimes

!syntax inputs /Times/TimeIntervalTimes

!syntax children /Times/TimeIntervalTimes
