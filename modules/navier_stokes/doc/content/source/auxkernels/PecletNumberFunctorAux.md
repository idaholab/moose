# PecletNumberFunctorAux

!syntax description /AuxKernels/PecletNumberFunctorAux

## Overview

The `PecletNumberFunctorAux` object computes the element/cell Peclet number
given by

\begin{equation}
Pe = \frac{h u}{\alpha}
\end{equation}

where $u$ is the norm of the velocity, e.g. the speed, $h$ the largest dimension
of the element (computed using `_current_elem->hmax()`), and $\alpha$ is the
thermal diffusivity (or mass diffusivity if this object is being used to gauge
mass transfer). The Peclet number is a ratio of the importance of advective
transport to conductive/diffusive transport. If greater than ~1, then typically
advection discretizations should favor upstream over downstream values in order
to appropriately model the physics and produce a stable solution algorithm. For
both finite volume and finite element methods, this means leaning towards upwind
methods.

!syntax parameters /AuxKernels/PecletNumberFunctorAux

!syntax inputs /AuxKernels/PecletNumberFunctorAux

!syntax children /AuxKernels/PecletNumberFunctorAux
