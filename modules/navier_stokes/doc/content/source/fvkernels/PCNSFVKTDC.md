# PCNSFVKTDC

!syntax description /FVKernels/PCNSFVKTDC

## Overview

This object implements a deferred correction approach in the following way for a
transient simulation

\begin{equation}
\bm{F}_n = f \bm{F}_{n,ho} + \left(1 - f\right)\left(\bm{F}_{n-1,ho} +
\bm{F}_{n,lo} - \bm{F}_{n-1,lo}\right)
\end{equation}

where $\bm{F}$ is the flux, $ho$ denotes high order, $lo$ denotes low order, $n$ refers to the current
time-step, $n-1$ refers to the previous timestep, and $f$ corresponds to the
`ho_implicit_fraction` parameter. The default value for $f$
(`ho_implicit_fraction`) is 0, which corresponds to the canonical deferred
correction approach. This default value will result in the best nonlinear
convergence when using an implicit time integration scheme. However, if the
fluid variables have significant gradients, it may take a very long time (in terms of
actual `time`) to march to a steady-state solution (if that is the intent
of the simulation). Setting `ho_implicit_fraction = 1` will result in the same
solution behavior as directly using [PCNSFVKT.md], which this object inherits
from. This will result in the worst nonlinear convergence but will also have the
most transient accuracy, and will converge most rapidly to a steady-state
solution if it exists (in terms of `time`). In terms of `Time Steps` a lower
`ho_implicit_fraction` will generally converge to a steady-state solution
quicker if the `steady_state_tolerance` is not tight.

!syntax parameters /FVKernels/PCNSFVKTDC

!syntax inputs /FVKernels/PCNSFVKTDC

!syntax children /FVKernels/PCNSFVKTDC
