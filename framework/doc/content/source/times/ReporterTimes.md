# ReporterTimes

!syntax description /Times/ReporterTimes

In order to retrieve a [Reporter](syntax/Reporters/index.md) with the `ReporterTimes`,
it must be a vector of floating point numbers (`std::vector<Real>`).

The times will be extracted from all the reporter vectors and sorted.

## Example File Syntax

In this example, the `ReporterTimes` is combining the times from two other
`Times`, using the fact that `Times` objects are [Reporters](syntax/Reporters/index.md)
in the back-end.

!listing tests/times/reporter_times.i block=Times

!syntax parameters /Times/ReporterTimes

!syntax inputs /Times/ReporterTimes

!syntax children /Times/ReporterTimes
