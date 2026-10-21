# SCMTriPowerAux

!syntax description /AuxKernels/SCMTriPowerAux

## Overview

!! Intentional comment to provide extra spacing

This AuxKernel does the same thing as the [SCMTriPowerIC](SCMTriPowerIC.md) IC kernel, but is more versatile in the sense that it can populate `q_prime` not only during initialization but at multiple times during the solve. This object does two things:

- At initial setup and at the start of each time step, it computes the trapezoidal integral of the axial power profile function over each pin on the subchannel axial grid. This normalization does not depend on [!param](/AuxKernels/SCMTriPowerAux/power).
- Every time it is executed, as set by [!param](/AuxKernels/SCMTriPowerAux/execute_on), it sets `q_prime` at each pin node to the pin power times the axial power profile function, divided by that integral. The pin power is computed from the current value of [!param](/AuxKernels/SCMTriPowerAux/power) and the radial power distribution.

The [!param](/AuxKernels/SCMTriPowerAux/power) postprocessor executes before this object on every execution flag they share, so `q_prime` uses its current value, including within fixed point iterations. Execute both on the same flags, for example `TIMESTEP_BEGIN` so that the solver uses the power of the current time step; otherwise a time-dependent power is applied one execution late. This object warns if the postprocessor does not execute on all of its [!param](/AuxKernels/SCMTriPowerAux/execute_on) flags, or if it sets `force_postaux`. These checks are skipped when [!param](/AuxKernels/SCMTriPowerAux/power) is a [Receiver.md] postprocessor, because its value is set by a transfer rather than by its own execution. In that case `q_prime` is current only if the application sending the power computes it before the transfer executes, for example on `MULTIAPP_FIXED_POINT_BEGIN` when the transfer executes on `TIMESTEP_BEGIN`; otherwise `q_prime` uses the power of the previous time step. Set [!param](/AuxKernels/SCMTriPowerAux/verbose) to print the per-pin correction factor, the heated length divided by the integral of the axial power profile function, every time it is computed.

The axial power profile function [!param](/AuxKernels/SCMTriPowerAux/axial_heat_rate) may depend on time: both the integral and `q_prime` are evaluated at the current time, so its amplitude and its shape can change between time steps while each pin keeps its assigned power. It must not depend on values that change during a time step, such as postprocessors, because the integral is computed at the start of the time step, before they execute, while `q_prime` uses their updated values. The pin powers then no longer match [!param](/AuxKernels/SCMTriPowerAux/power).

## Caveat

!! Intentional comment to provide extra spacing

This AuxKernel requires a pin mesh. The axial heat rate `q_prime` is assigned only to the nodes of the pin mesh. If the subchannel mesh has no pin mesh, this object reports an error.

!syntax parameters /AuxKernels/SCMTriPowerAux

!syntax inputs /AuxKernels/SCMTriPowerAux

!syntax children /AuxKernels/SCMTriPowerAux
