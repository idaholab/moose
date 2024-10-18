# SideFVFluxBCIntegral

Computes the side integral of different finite volume flux boundary conditions.
Let $F(u)$ denote a flux on the boundary given by a flux boundary condition.
In practice, this flux can be the result of diffusive, convective
or radiative boundary conditions, to name a few. This postprocessor
provides the following expression:

\begin{equation}
\int_S \sum_i^{N_bc}F_i(u) dS,
\end{equation}

where $N_bc$ is the number of specified boundary conditions, while $S$ denotes the
boundary surface.


!alert warning
The current implementation only supports objects that inherit from `FVFluxBCs`.

## Example Input Syntax

!listing test/tests/postprocessors/fvfluxbc_integral/fvfluxbc_integral.i block=Postprocessors

!syntax parameters /Postprocessors/SideFVFluxBCIntegral

!syntax inputs /Postprocessors/SideFVFluxBCIntegral

!syntax children /Postprocessors/SideFVFluxBCIntegral
