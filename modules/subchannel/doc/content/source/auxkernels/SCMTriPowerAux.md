# SCMTriPowerAux

!syntax description /AuxKernels/SCMTriPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMTriPowerIC](SCMTriPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve. This object does two things:

- At initial setup and at the start of each time step, it computes the trapezoidal integral of the axial power profile function over each pin on the subchannel axial grid. This normalization does not depend on [!param](/AuxKernels/SCMTriPowerAux/power).
- Every time it is executed, as set by [!param](/AuxKernels/SCMTriPowerAux/execute_on), it sets `q_prime` at each pin node to the pin power times the axial power profile function, divided by that integral. The pin power is computed from the current value of [!param](/AuxKernels/SCMTriPowerAux/power) and the radial power distribution.

The [!param](/AuxKernels/SCMTriPowerAux/power) postprocessor executes before this object on every execution flag they share, so `q_prime` uses its current value, including within fixed point iterations. Execute both on the same flags, for example `TIMESTEP_BEGIN` so that the solver uses the power of the current time step; otherwise a time-dependent power is applied one execution late.

## Caveat

!! Intentional comment to provide extra spacing

This AuxKernel requires a pin mesh. The axial heat rate `q_prime` is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error.

!syntax parameters /AuxKernels/SCMTriPowerAux

!syntax inputs /AuxKernels/SCMTriPowerAux

!syntax children /AuxKernels/SCMTriPowerAux
