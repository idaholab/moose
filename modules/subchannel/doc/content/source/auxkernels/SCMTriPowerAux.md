# SCMTriPowerAux

!syntax description /AuxKernels/SCMTriPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMTriPowerIC](SCMTriPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve. The pin powers and pin power correction factors are recomputed once per time step, at the start of the time step, from the value of [!param](/AuxKernels/SCMTriPowerAux/power) and from the axial power profile function evaluated at the new time. [!param](/AuxKernels/SCMTriPowerAux/execute_on) only controls when `q_prime` is filled from these stored values; executing this object more often, for example on `LINEAR` or `MULTIAPP_FIXED_POINT_END`, does not update them. Because postprocessors and transfers execute after the start of the time step, a [!param](/AuxKernels/SCMTriPowerAux/power) postprocessor that changes in time is applied one time step late, and changes to it during fixed point iterations within a time step are not picked up.

## Caveat

!! Intentional comment to provide extra spacing

This AuxKernel requires a pin mesh. The axial heat rate `q_prime` is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error.

!syntax parameters /AuxKernels/SCMTriPowerAux

!syntax inputs /AuxKernels/SCMTriPowerAux

!syntax children /AuxKernels/SCMTriPowerAux
