# FVAdvectedUpwind

## Overview

This object provides first-order upwind interpolation for an advected quantity on a
finite-volume face. The upwind state is selected using the sign of the face mass flux
($m_f$), yielding a monotone, TVD discretization that is robust near discontinuities at the cost
of increased numerical diffusion ([!cite](moukalled2016finite), [!cite](harten1997)).

Let $\phi_C$ and $\phi_N$ denote the cell-centered values on the element and neighbor sides of a
face. Define the upwind value

!equation
\phi_U =
\begin{cases}
\phi_C & \text{if } m_f \ge 0,\\
\phi_N & \text{if } m_f < 0.
\end{cases}

Then the interpolated face value is simply

!equation
\phi_f = \phi_U.

In matrix form this corresponds to weights $(w_C, w_N)\in\{(1,0),(0,1)\}$ chosen by the mass flux
sign, so no gradients are required. This is a common baseline scheme for advection and is often
used as the low-order component in higher-order schemes with limiting or deferred correction (see
[Limiters](syntax/Limiters/index.md)).

## Example Syntax

Declare the interpolation method in `[FVInterpolationMethods]`:

!listing test/tests/linearfvkernels/advection/advection-1d.i block=upwind

Use it in a linear FV advection kernel via
[!param](/LinearFVKernels/LinearFVAdvection/advected_interp_method_name):

!listing test/tests/linearfvkernels/advection/advection-1d.i block=advection

!syntax parameters /FVInterpolationMethods/FVAdvectedUpwind

!syntax inputs /FVInterpolationMethods/FVAdvectedUpwind

!syntax children /FVInterpolationMethods/FVAdvectedUpwind
