# MachNumberAux

!syntax description /AuxKernels/MachNumberAux

The Mach number is computed as:

!equation
\text{Mach number} = \dfrac{\text{velocity}}{\text{speed of sound}}

## Example usage:

!listing test/tests/auxkernels/mach_number/1phase.i id=1phase start=AuxKernels end=Components caption=Single phase example

The variables `v`, `e`, `vel` are created when a single phase component is present in the
input file. If not using the component syntax, the user is responsible for creating the variables
and computing them.

!syntax parameters /AuxKernels/MachNumberAux

!syntax inputs /AuxKernels/MachNumberAux

!syntax children /AuxKernels/MachNumberAux
