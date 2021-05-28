# Courant

!syntax description /AuxKernels/Courant

## Overview

The `Courant` object computes the Courant number given coupled variables
representing velocity components (`u`, `v`, and `w`) and using the current
timestep size and element size. The Courant number formula is:

\begin{equation}
C = \frac{u\Delta t}{h}
\end{equation}

where $u$ is the norm of the velocity, $\Delta t$ is the timestep size, and $h$
is the local element size (units of length). Knowledge of the Courant number can
be very useful when determining timestep sizes when using explicit time
integrators as it is related to the
[Courant-Friedrichs-Lewy condition](https://en.wikipedia.org/wiki/Courant%E2%80%93Friedrichs%E2%80%93Lewy_condition).

!syntax parameters /AuxKernels/Courant

!syntax inputs /AuxKernels/Courant

!syntax children /AuxKernels/Courant
