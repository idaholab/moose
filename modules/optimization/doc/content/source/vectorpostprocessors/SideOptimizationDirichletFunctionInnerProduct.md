# SideOptimizationDirichletFunctionInnerProduct

!syntax description /VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct

## Overview

This vector-postprocessor computes the gradient used when the value of a
[Dirichlet boundary condition](FunctionDirichletBC.md) is the quantity being inverted for. The
boundary value is supplied to the forward problem by an `OptimizationFunction` such as
[ParsedOptimizationFunction.md] or [NearestReporterCoordinatesFunction.md]. The parameters enter
the forward problem through a strongly imposed value rather than through a load term, so the
gradient is the normal diffusive flux of the adjoint variable on the controlled boundary. For
steady-state problems it is

\begin{equation}
V_i = -\oint_{\partial\Omega}\kappa(\vec{r})
\left(\nabla\lambda(\vec{r})\cdot\boldsymbol{n}\right)
\left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0}~d\vec{r},
\end{equation}

which uses a quadrature rule to perform the integration. $\lambda$ is the adjoint variable
specified by
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/variable), $\kappa$ is
the material property named by
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/diffusivity),
$\boldsymbol{n}$ is the outward unit normal of the side-set specified by
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/boundary),
$f(\vec{r},\vec{p})$ is the `OptimizationFunction` specified by
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/function), $\vec{p}$ is
the vector of parameters that is defined in the function, and $\vec{p}_0$ is the current values of
the parameters in the function.

This object executes on the adjoint problem, so
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/diffusivity) must name
a material property that the adjoint input defines on the controlled boundary, and it must be the
same conductivity that the forward diffusion operator uses. The expression above is the gradient of
the Dirichlet data only when the adjoint variable vanishes on that boundary, which the adjoint
solve enforces for every Dirichlet condition of the forward problem; see the
[inverse optimization theory](theory/InvOptTheory.md) page for the derivation of the adjoint
problem and its boundary conditions. The
variable coupled through
[!param](/VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct/variable) must be a
finite element variable.

This object executes at the default `TIMESTEP_END`, which is correct in a split layout where the
forward and adjoint problems run as separate applications. When the forward and adjoint systems
instead solve within a single input under [SteadyAndAdjoint.md], the object must set
`execute_on = ADJOINT_TIMESTEP_END` so it evaluates only after the adjoint solve completes.

## Accuracy

The boundary-flux quadrature above is consistent with, but not identical to, the exact gradient of
the discrete objective: the two differ by a discretization error that vanishes with mesh
refinement. The observed rate depends on the element type; on the tensor-product quadrilateral
meshes used in the module tests it converges at second order, while on triangular meshes it is
first order and substantially larger at equal element counts. A finite-difference check of the gradient (for example with the `TaoGradientTester`
test harness) therefore reports a small mesh-dependent mismatch rather than round-off, and its
tolerance must account for the resolution of the controlled boundary. The variationally exact
alternative is to assemble the gradient from the adjoint nodal reactions on the controlled
boundary, which this object does not do. An exact cross-check that avoids the flux quadrature
entirely is to impose the boundary data weakly with [FunctionPenaltyDirichletBC.md] and compute the
gradient with [SideOptimizationNeumannFunctionInnerProduct.md] scaled by the penalty factor.

## Example Input File Syntax

!listing test/tests/optimizationreporter/bc_load_dirichlet/adjoint.i block=VectorPostprocessors

This function is primarily used for computing the gradient in an optimization routine where the
value of a [Dirichlet boundary condition](FunctionDirichletBC.md) is being optimized. See
[bc_load_dirichlet/forward.i] and [bc_load_dirichlet/adjoint.i] as an example of the forward and
adjoint input pair.

!syntax parameters /VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct

!syntax inputs /VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct

!syntax children /VectorPostprocessors/SideOptimizationDirichletFunctionInnerProduct
