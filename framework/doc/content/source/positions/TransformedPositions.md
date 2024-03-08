# TransformedPositions

!syntax description /Positions/TransformedPositions

!alert note
The `TransformedPositions` may require re-ordering the `Positions` objects as it must be initialized after the positions it transforms.

!alert note
Rotations along X Y and Z generally do not commute. If in doubt, using multiple `TransformedPositions`
to get the desired rotations may make it easier.

## Example File Syntax

In this example, several `TransformedPositions` are used to perform various transformations
on `Positions` objects.

!listing tests/positions/transformed_positions.i block=Positions

!syntax parameters /Positions/TransformedPositions

!syntax inputs /Positions/TransformedPositions

!syntax children /Positions/TransformedPositions
