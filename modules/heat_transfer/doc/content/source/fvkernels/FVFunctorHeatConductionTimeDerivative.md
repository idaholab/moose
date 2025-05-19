# FVFunctorHeatConductionTimeDerivative

## Description

The `FVFunctorHeatConductionTimeDerivative` kernel implements the residual

\begin{equation}
\int_{\Omega_C} \rho c_p \frac{\partial u}{\partial t} dV
\end{equation}

where $\rho$ is the material density, $c_p$ is the specific heat capacity at
constant pressure and $u$ the temperature variable. It uses [functors](Functors/index.md)
for these properties. The [FVHeatConductionTimeDerivative.md] can be used with material properties instead.

!syntax parameters /FVKernels/FVFunctorHeatConductionTimeDerivative

!syntax inputs /FVKernels/FVFunctorHeatConductionTimeDerivative

!syntax children /FVKernels/FVFunctorHeatConductionTimeDerivative
