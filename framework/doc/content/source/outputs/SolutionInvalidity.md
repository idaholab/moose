# PerfGraphOutput

!syntax description /Outputs/SolutionInvalidityOutput

## Description

The [/SolutionInvalidity.md] object holds solution invalid warning information for MOOSE. With this object you can mark a solution as "invalid" and output where and how many times the warning occurs. An invalid solution means that the solution somehow does not satisfy requirements such as a value being out of bounds of a correlation.  Solutions are allowed to be invalid _during_ the nonlinear solve - but are not allowed to invalid once it.


!syntax parameters /Outputs/PerfGraphOutput

!syntax inputs /Outputs/PerfGraphOutput

!syntax children /Outputs/PerfGraphOutput

!bibtex bibliography
