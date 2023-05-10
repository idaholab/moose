# ReporterPositions

!syntax description /Positions/ReporterPositions

In order to retrieve a [Reporter](syntax/Reporters/index.md) with the `ReporterPositions`,
it must be a vector of points (`std::vector<Point>`).

## Example File Syntax

In this example, the `ReporterPositions` is obtaining the positions from two other
`Positions`, using the fact that `Positions` objects are [Reporters](syntax/Reporters/index.md)
in the back-end.

!listing tests/positions/reporter_positions.i block=Positions

!syntax parameters /Positions/ReporterPositions

!syntax inputs /Positions/ReporterPositions

!syntax children /Positions/ReporterPositions
