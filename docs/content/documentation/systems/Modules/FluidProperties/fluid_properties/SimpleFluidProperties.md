#SimpleFluidProperties
!syntax description /Modules/FluidProperties/SimpleFluidProperties

This is a computationally simple fluid based on a constant bulk modulus density fluid,
with density given by
\begin{equation}
  \rho = \rho_{0}\exp(P/K_{f} - \alpha_{f} T),
  \label{eq:simplifiedfluid}
\end{equation}
where $K_{f}$ (the bulk modulus) and $\alpha_{f}$ (the thermal expansion coefficient) are
constants.

In this formulation, viscosity and thermal conductivity are constant (with values specified
in the input file), while internal energy and enthalpy are given by
\begin{equation}
  e = c_v T
\end{equation}
and
\begin{equation}
  h = e + \frac{p}{\rho}
\end{equation}
respectively.

!syntax parameters /Modules/FluidProperties/SimpleFluidProperties

!syntax inputs /Modules/FluidProperties/SimpleFluidProperties

!syntax children /Modules/FluidProperties/SimpleFluidProperties
