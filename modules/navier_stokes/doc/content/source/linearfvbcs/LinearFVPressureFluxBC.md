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

!syntax parameters /LinearFVBCs/LinearFVPressureFluxBC

!syntax inputs /LinearFVBCs/LinearFVPressureFluxBC

!syntax children /LinearFVBCs/LinearFVPressureFluxBC
