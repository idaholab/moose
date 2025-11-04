# FVFunctorHeatConductionTimeDerivative

## Description

The `FVFunctorHeatConductionTimeDerivative` kernel implements the residual

\begin{equation}
\int_{\Omega_C} \rho c_p \frac{\partial T}{\partial t} dV
\end{equation}

where $\rho$ is the material density, $c_p$ is the specific heat capacity at
constant pressure and $T$ the temperature variable. It uses [functors](Functors/index.md)
for these properties. [FVHeatConductionTimeDerivative.md] can be used with [traditional material properties](/Materials/index.md) instead.

!syntax parameters /FVKernels/FVFunctorHeatConductionTimeDerivative

!syntax inputs /FVKernels/FVFunctorHeatConductionTimeDerivative

!syntax children /FVKernels/FVFunctorHeatConductionTimeDerivative
