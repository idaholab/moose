# SCMQuadPowerAux

!syntax description /AuxKernels/SCMQuadPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMQuadPowerIC](SCMQuadPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve.

## Caveat

!! Intentional comment to provide extra spacing

This AuxKernel requires a pin mesh. The axial heat rate `q_prime` is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error.

!syntax parameters /AuxKernels/SCMQuadPowerAux

!syntax inputs /AuxKernels/SCMQuadPowerAux

!syntax children /AuxKernels/SCMQuadPowerAux
