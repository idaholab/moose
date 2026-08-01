# LinearFVPressureFluxBC

## Description

This pressure boundary condition is meant to be used with a
[LinearFVAdvectionDiffusionFunctorDirichletBC.md] applied to the velocity field. It makes the
boundary pressure flux consistent with the pressure Poisson equation and the prescribed boundary
velocity. The $HbyA$ flux supplied by [RhieChowMassFlux.md] includes the off-diagonal momentum
contributions and every assembled non-pressure momentum source.

The boundary velocity functors [!param](/LinearFVBCs/LinearFVPressureFluxBC/u),
[!param](/LinearFVBCs/LinearFVPressureFluxBC/v), and
[!param](/LinearFVBCs/LinearFVPressureFluxBC/w), together with the density functor
[!param](/LinearFVBCs/LinearFVPressureFluxBC/rho), define the prescribed normal mass flux
that this boundary condition enforces:

\begin{equation}
(\rho \vec{u} \cdot \vec{n})_{bf} = -HbyA_{flux,bf} + (\rho A^{-1} \nabla p \cdot \vec{n})_{bf}.
\end{equation}

!syntax parameters /LinearFVBCs/LinearFVPressureFluxBC

!syntax inputs /LinearFVBCs/LinearFVPressureFluxBC

!syntax children /LinearFVBCs/LinearFVPressureFluxBC
