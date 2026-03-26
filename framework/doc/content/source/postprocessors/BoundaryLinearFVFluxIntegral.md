# BoundaryLinearFVFluxIntegral

Computes the boundary integral of boundary flux contributions from one or more
`LinearFVFluxKernel` objects (e.g. [LinearFVDiffusion.md], [LinearFVAdvection.md]).
Let $F_k(u)$ denote the boundary flux computed by kernel $k$. This postprocessor computes

\begin{equation}
\int_S \sum_i^{N} F_k(u)\, dS,
\end{equation}

where $N$ is the number of kernels listed in [!param](/Postprocessors/BoundaryLinearFVFluxIntegral/linearfvkernels)
and $S$ is the selected sideset.

!alert note
All kernels listed in [!param](/Postprocessors/BoundaryLinearFVFluxIntegral/linearfvkernels) must act
on the same linear FV variable name, and that
variable must have a [LinearFVBoundaryCondition](syntax/LinearFVBCs/index.md) on every boundary listed for this postprocessor.
This is a requirement at the moment because we would like to avoid summing fluxes from different variables/equations.

!alert note
Furthermore, we don't support flux evaluation for internal sidesets at the moment. This kernel is
supposed to serve as a boundary flux evaluator.

## Example Input Syntax

!listing test/tests/postprocessors/linearfv_flux_integral/linearfv_flux_integral.i block=Postprocessors

!syntax parameters /Postprocessors/BoundaryLinearFVFluxIntegral

!syntax inputs /Postprocessors/BoundaryLinearFVFluxIntegral

!syntax children /Postprocessors/BoundaryLinearFVFluxIntegral
