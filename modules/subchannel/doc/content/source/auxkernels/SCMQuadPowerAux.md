# SCMQuadPowerAux

!syntax description /AuxKernels/SCMQuadPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMQuadPowerIC](SCMQuadPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve.

## Caveat

!! Intentional comment to provide extra spacing

If the user has created a mesh for the pins, the axial heat rate `q_prime` will be assigned to the nodes of the pin mesh. If the user hasn't created a pin mesh the appropriate heat rate `q_prime` will be assigned to
the nodes of the subchannel mesh.

!syntax parameters /AuxKernels/SCMQuadPowerAux

!syntax inputs /AuxKernels/SCMQuadPowerAux

!syntax children /AuxKernels/SCMQuadPowerAux
