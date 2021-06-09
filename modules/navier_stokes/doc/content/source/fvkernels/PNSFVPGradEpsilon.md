# PNSFVPGradEpsilon

!syntax description /FVKernels/PNSFVPGradEpsilon

## Overview

This object adds a residual equivalent to

\begin{equation}
\int_{\Omega_C} -p \nabla \epsilon dV
\end{equation}

This object must be included in any simulations where the $\epsilon \nabla p$
term has been integrated by parts as is done by the [PCNSFVKT.md] and
[PCNSFVHLLC.md] objects.

This object only accepts porosity as a function. If the porosity profile is
complicated, a [SolutionFunction](SolutionFunction.md) may be used to represent
it.

!alert note
We have yet to see good results when the porosity is represented by a
discontinuous function. On straight-channel porosity jump tests with no momentum
or energy sources/sinks, if the porosity steps down and up again and this change
is represented in a discontinuous fashion, then the outlet pressure and density
(the latter when solving a compressible problem) do not match the inlet pressure
and density. However, if the step changes are approximated by steep continuous
ramps, then the inlet and outlet pressure and density match as they should. This
behavior has been observed both with [Kurganov-Tadmor](PCNSFVKT.md) and
[HLLC](PCNSFVHLLC.md) discretizations of the advective fluxes.

!syntax parameters /FVKernels/PNSFVPGradEpsilon

!syntax inputs /FVKernels/PNSFVPGradEpsilon

!syntax children /FVKernels/PNSFVPGradEpsilon
