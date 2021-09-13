# GapValueAux

!syntax description /AuxKernels/GapValueAux

This object is typically used in mechanical contact problems to get information about the variable
on the other side of the boundary.

See [GeometricSearchData.md] for more information on the treatment of contact problems.

## Example syntax

In this example, the `GapValueAux` is used to retrieve the value of variable `u` on
boundary `rightleft` across from boundary `leftright`.

!listing test/tests/auxkernels/gap_value/gap_value.i block=AuxKernels

!syntax parameters /AuxKernels/GapValueAux

!syntax inputs /AuxKernels/GapValueAux

!syntax children /AuxKernels/GapValueAux
