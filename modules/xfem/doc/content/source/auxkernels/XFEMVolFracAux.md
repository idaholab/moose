# XFEMVolFracAux

!syntax description /AuxKernels/XFEMVolFracAux

# Description

The MOOSE XFEM implementation uses the phantom node technique, in which elements traversed by a discontinuity are split into two partial elements, each containing physical and non-physical material. The `XFEMVolFracAux` is used to compute the volume fraction of physical portions of each partial element.
