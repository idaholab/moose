# FunctorPositions

!syntax description /Positions/FunctorPositions

See the [Functor](syntax/Functors/index.md) for the list of available functors.
The functors are evaluated at the origin of the mesh at the current time.

## Example File Syntax

In this example, the `FunctorPositions` is obtaining the positions from three time-dependent functions.
The functions are evaluated at the origin of the mesh at the current time.

!listing tests/positions/functor_positions.i block=Positions

!syntax parameters /Positions/FunctorPositions

!syntax inputs /Positions/FunctorPositions

!syntax children /Positions/FunctorPositions
