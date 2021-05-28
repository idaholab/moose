# MaterialMachAux

!syntax description /AuxKernels/MaterialMachAux

## Overview

`MaterialMachAux` computes the Mach number, similar to [NSMachAux.md], but
whereas `NSMachAux` computes the Mach number based on MOOSE variable objects,
`MaterialMachAux` uses material properties (based on default Navier-Stokes
naming (see
[NS.h](https://github.com/idaholab/moose/blob/next/modules/navier_stokes/include/base/NS.h))). The
Mach number is computed via the formula:

\begin{equation}
M = \frac{u}{c}
\end{equation}

where $u$ is the velocity norm (e.g. the speed) and $c$ is the local speed of
sound in the medium.

!syntax parameters /AuxKernels/MaterialMachAux

!syntax inputs /AuxKernels/MaterialMachAux

!syntax children /AuxKernels/MaterialMachAux
