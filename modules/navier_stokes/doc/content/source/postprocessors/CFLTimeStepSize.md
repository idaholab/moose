# CFLTimeStepSize

!syntax description /Postprocessors/CFLTimeStepSize

## Overview

This object computes the timestep necessary to satisfy a
[CFL number](https://en.wikipedia.org/wiki/Courant%E2%80%93Friedrichs%E2%80%93Lewy_condition)
specified by the `CFL` parameter. The default value for `CFL` is $0.5$. The
timestep is computed as the global minimum of

\begin{equation}
dt = \frac{\text{CFL}\ h_{min}}{a_k + c_k}
\end{equation}

where $h_{min}$ is the minimum element side length, $a_k$ is the fluid speed for
the $k\text{-th}$ phase and $c_k$ is the speed of sound in the $k\text{-th}$
phase.

!syntax parameters /Postprocessors/CFLTimeStepSize

!syntax inputs /Postprocessors/CFLTimeStepSize

!syntax children /Postprocessors/CFLTimeStepSize
