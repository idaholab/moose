# ReynoldsNumberFunctorAux

!syntax description /AuxKernels/ReynoldsNumberFunctorAux

## Overview

The `ReynoldsNumberFunctorAux` object computes the element/cell Reynolds number
given by

\begin{equation}
Re = \frac{\rho h u}{\mu}
\end{equation}

where $u$ is the norm of the velocity, e.g. the speed, $h$ the largest dimension
of the element (computed using `_current_elem->hmax()`), $\rho$ is the density,
and $\mu$ is the dynamic viscosity. The Reynolds number is a critical piece of a
fluid flow simulation as it is associated with transitions from laminar to
turbulent flows and also determines whether stabilization is needed. The
Reynolds number is a ratio of inertial forces to viscous forces. Large values of
the Reynolds number (e.g. >> 1) indicate advection or inertially dominated flows
(with the limit being an inviscid flow)
that require stabilization (typically upwinding) in order to reach a stable
solution. Low Reynolds numbers (e.g. << 1) are associated with viscous flows
(with the limit being Stokes flow) and are typically stable and do not require
stabilization.

!syntax parameters /AuxKernels/ReynoldsNumberFunctorAux

!syntax inputs /AuxKernels/ReynoldsNumberFunctorAux

!syntax children /AuxKernels/ReynoldsNumberFunctorAux
