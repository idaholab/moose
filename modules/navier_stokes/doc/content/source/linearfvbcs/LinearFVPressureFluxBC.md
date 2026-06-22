# LinearFVPressureFluxBC

## Description

This pressure boundary condition is meant to be used in conjunction with the
 [LinearFVAdvectionDiffusionFunctorDirichletBC.md] applied to the velocity field in
the presence of body forces, in particular for no-slip walls.
This boundary condition ensures that the pressure flux on the boundary is
consistent with the Poisson equation solved for pressure:

\begin{equation}
-\nabla \cdot (\rho A^{-1} \nabla p )_{bf} = \nabla \cdot (-\rho A^{-1} H + \rho A^{-1} F_b)_{bf},
\end{equation}

where $A^{-1}$ and $H$ operators are built for the SIMPLE segregated solver, subscript $bf$ denotes boundary face,
and $F_b$ is the body force.

The `computeBoundaryValue` function in this class ensures that the pressure at the face is obtained
taking into account the Dirichlet boundary condition for velocity and the body forces.

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
