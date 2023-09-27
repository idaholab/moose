# FVFunctorRadiativeBC

!syntax description /FVBCs/FVFunctorRadiativeBC

This boundary condition is the finite volume analog of
[FunctionRadiativeBC.md]. It computes the radiative heat flux from a boundary
where the emissivity is provided through a [functor](Functors/index.md).

\begin{equation}
 \sigma F_e (T_\infty^4-T_s^4)
\end{equation}
where $\sigma$ is the Stephan-Boltzmann constant, $F_e$ is the emissivity function,
$T_\infty$ is the temperature far from the surface, and $T_s$ is the temperature of
the surface.

!syntax parameters /FVBCs/FVFunctorRadiativeBC

!syntax inputs /FVBCs/FVFunctorRadiativeBC

!syntax children /FVBCs/FVFunctorRadiativeBC
