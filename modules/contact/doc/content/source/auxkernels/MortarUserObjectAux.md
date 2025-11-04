# MortarUserObjectAux

## Description

The `MortarUserObjectAux` outputs various contact quantities that
are helpful for analyzing or debugging contact results. This auxiliary kernel
is to be used in conjunction with mortar penalty contact user objects, namely
[PenaltyFrictionUserObject](/PenaltyFrictionUserObject.md) or
[PenaltyWeightedGapUserObject](/PenaltyWeightedGapUserObject.md).

Quantities that this object can output include: the normal and friction
pressure values, accumulated slip distances, tangential relative velocities,
and weighted gap values.

!syntax parameters /AuxKernels/MortarUserObjectAux

!syntax inputs /AuxKernels/MortarUserObjectAux

!syntax children /AuxKernels/MortarUserObjectAux
