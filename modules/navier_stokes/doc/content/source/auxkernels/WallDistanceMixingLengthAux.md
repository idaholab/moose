# WallDistanceMixingLengthAux

This auxkernel computes the turbulent mixing length by assuming that it is
proportional to the distance from the nearest wall. If a $\delta$ parameter is
set by the user, the mixing length takes on Escudier's modification to cap the
mixing length at a distance from the wall proportional to $\delta$. For more
details please refer to https://mooseframework.inl.gov/modules/navier_stokes/rans_theory.html.

!syntax parameters /AuxKernels/WallDistanceMixingLengthAux

!syntax inputs /AuxKernels/WallDistanceMixingLengthAux

!syntax children /AuxKernels/WallDistanceMixingLengthAux
