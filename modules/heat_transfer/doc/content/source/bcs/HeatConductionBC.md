# HeatConductionBC

!syntax description /BCs/HeatConductionBC

This boundary condition models the heat conduction flux when integrating the
heat conduction equation by parts when using the [HeatConduction.md] kernel.
The residual is computed on the boundary $\delta \Omega$ as:

!equation
R = \int_{\delta \Omega} k \nabla T \psi

with $k$ a `Real`-valued thermal conductivity, $T$ the solid temperature, and $\psi$ a test function.

!alert! warning
If the thermal conductivity depends on temperature, the contribution of this boundary condition to the Jacobian
will be approximate.
!alert-end!

!syntax parameters /BCs/HeatConductionBC

!syntax inputs /BCs/HeatConductionBC

!syntax children /BCs/HeatConductionBC
