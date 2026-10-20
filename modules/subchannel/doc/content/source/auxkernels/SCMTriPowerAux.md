# SCMTriPowerAux

!syntax description /AuxKernels/SCMTriPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMTriPowerIC](SCMTriPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve. Every time it is executed, as set by [!param](/AuxKernels/SCMTriPowerAux/execute_on), it does two things:

- If the value of [!param](/AuxKernels/SCMTriPowerAux/power) or the simulation time changed since the last execution, it recomputes the pin powers and the per-pin correction factors from the current value of [!param](/AuxKernels/SCMTriPowerAux/power) and the axial power profile function evaluated at the current time.
- It sets `q_prime` at each pin node from the pin power, the correction factor and the axial power profile function.

The [!param](/AuxKernels/SCMTriPowerAux/power) postprocessor executes before this object on every execution flag they share, so `q_prime` uses its current value. Execute both on the same flags, for example `TIMESTEP_BEGIN` so that the solver uses the power of the current time step; otherwise a time-dependent power is applied one execution late.

## Caveat

!! Intentional comment to provide extra spacing

This AuxKernel requires a pin mesh. The axial heat rate `q_prime` is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error.

!syntax parameters /AuxKernels/SCMTriPowerAux

!syntax inputs /AuxKernels/SCMTriPowerAux

!syntax children /AuxKernels/SCMTriPowerAux
