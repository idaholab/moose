#IdealGasFluidPropertiesPT
!syntax description /Modules/FluidProperties/IdealGasFluidPropertiesPT

A simple formulation that is suitable for ideal gases, where properties are derived from
the ideal gas law
\begin{equation}
  P = \rho R T.
\end{equation}

Internal energy of an ideal gas is
\begin{equation}
  e = c_v T
\end{equation}

while enthalpy is
\begin{equation}
  h = c_p T.
\end{equation}

All other properties (such as viscosity or thermal conductivity) are assumed to be constant
in this fluid, and must be provided in the input file (see below).

!syntax parameters /Modules/FluidProperties/IdealGasFluidPropertiesPT

!syntax inputs /Modules/FluidProperties/IdealGasFluidPropertiesPT

!syntax children /Modules/FluidProperties/IdealGasFluidPropertiesPT
