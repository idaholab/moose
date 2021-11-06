# VolumeAux

!syntax description /AuxKernels/VolumeAux

`VolumeAux` samples element or side volumes and stores them in an [AuxVariable.md].

It requires that the supplied [!param](/AuxKernels/VolumeAux/variable) be of type `CONSTANT` `MONOMIAL`.

## Example syntax

In this example, the `VolumeAux` object `volume_aux` samples the elemental volumes into the variable `volume`.

!listing test/tests/auxkernels/volume_aux/element.i

!syntax parameters /AuxKernels/VolumeAux

!syntax inputs /AuxKernels/VolumeAux

!syntax children /AuxKernels/VolumeAux
