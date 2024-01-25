# CohesiveZoneMortarUserObjectAux

## Description

The `CohesiveZoneMortarUserObjectAux` outputs various cohesive zone quantities that
are helpful for analyzing or debugging cohesive zone or debonding simulation results.
This auxiliary kernel is to be used in conjunction with mortar cohesive zone user objects, namely
[BilinearMixedModeCohesiveZoneModel](/BilinearMixedModeCohesiveZoneModel.md).

Quantities that this object can output include: the cohesive zone model damage and the
opening mode mixity ratio.

!syntax parameters /AuxKernels/CohesiveZoneMortarUserObjectAux

!syntax inputs /AuxKernels/CohesiveZoneMortarUserObjectAux

!syntax children /AuxKernels/CohesiveZoneMortarUserObjectAux
