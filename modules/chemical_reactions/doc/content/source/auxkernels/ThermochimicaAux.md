# ThermochimicaAux

!syntax description /AuxKernels/ThermochimicaAux

!alert note title=For Use with Thermochimica
This AuxKernel is designed for use with thermochemistry library Thermochimica.

## Description

[`ThermochimicaAux`](ThermochimicaAux.md) forms the base class for any AuxKernels required for moving data
calculated by Thermochimica from a UserObject derived from [`ThermochimicaNodalData`](ThermochimicaNodalData.md)
into the necessary variables for subsequent use and output.
[`ThermochimicaAux`](ThermochimicaAux.md) copies phase and species data to the relevant AuxVariables, which
should have been created automatically, provided the [`ChemicalCompositionAction`](ChemicalCompositionAction.md) Action
was used.

!syntax parameters /AuxKernels/ThermochimicaAux

!syntax inputs /AuxKernels/ThermochimicaAux

!syntax children /AuxKernels/ThermochimicaAux
