# CoupledConvectiveFlux

!syntax description /BCs/CoupledConvectiveFlux

!alert warning
`CoupledConvectiveFlux` is deprecated.

This boundary condition models the heat convection flux.
The residual is computed on the boundary $\delta \Omega$ as:

!equation
R = \int_{\delta \Omega}  C (T - T_{\infty}) \psi

with $C$ a constant convective heat transfer coefficient, $T$ the temperature variable, $T_{\infty}$ the temperature
in the bulk, and $\psi$ a test function.

!syntax parameters /BCs/CoupledConvectiveFlux

!syntax inputs /BCs/CoupledConvectiveFlux

!syntax children /BCs/CoupledConvectiveFlux
