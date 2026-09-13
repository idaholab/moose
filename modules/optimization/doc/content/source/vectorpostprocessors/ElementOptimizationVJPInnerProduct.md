# ElementOptimizationVJPInnerProduct

!syntax description /VectorPostprocessors/ElementOptimizationVJPInnerProduct

## Overview

This vector-postprocessor computes the gradient of the objective function with respect to a NEML2
model parameter from a precontracted vector-Jacobian product, rather than from a materialized
derivative tensor assembled here. For steady-state problems, the inner product is defined as:

!equation
V_i = -\int_{\Omega} g_k(\vec{r}) \left.\frac{df(\vec{r},\vec{p})}{dp_i}\right|_{\vec{p}=\vec{p}_0} d\vec{r},

which uses a quadrature rule to perform the integration. $g_k(\vec{r}) =
\boldsymbol{\varepsilon}(\boldsymbol{\lambda}) : \dfrac{\partial\boldsymbol{\sigma}}{\partial p_k}$,
the contraction of the adjoint strain with the derivative of stress with respect to NEML2 parameter
$p_k$, is the value at $\vec{r}$ of the material property named by
[!param](/VectorPostprocessors/ElementOptimizationVJPInnerProduct/vjp_name). That property is named
`vjp_<neml2_output>_<parameter>` (e.g. `vjp_neml2_stress_elasticity_E`) and is produced by the
[!param](/NEML2/parameter_vjp_variable), [!param](/NEML2/parameter_vjp_cotangent) and
[!param](/NEML2/parameter_vjp_parameters) parameters of the [NEML2 block](syntax/NEML2/index.md),
which contract the adjoint strain against the parameter derivative in a single reverse-mode pass
rather than assembling that contraction from a coupled `variable`/`forward_variable` pair. $f(\vec{r},
\vec{p})$ is the `OptimizationFunction` specified by
[!param](/VectorPostprocessors/ElementOptimizationVJPInnerProduct/function), $\vec{p}$ is the vector
of parameters defined in that function, and $\vec{p}_0$ is the current values of the parameters in
the function; see [ParsedOptimizationFunction.md] for an example that also converts a normalized
optimizer parameter to the physical NEML2 parameter value.

The [!param](/VectorPostprocessors/ElementOptimizationVJPInnerProduct/variable) parameter, required
by an ancestor class, plays no role in this class: the adjoint contraction is already folded into
[!param](/VectorPostprocessors/ElementOptimizationVJPInnerProduct/vjp_name), so any coupled variable
satisfies the parameter.

## Example Input File Syntax

!listing modules/combined/test/tests/optimization/invOpt_neml2_viscoplastic/forward_and_adjoint_vjp.i
         block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ElementOptimizationVJPInnerProduct

!syntax inputs /VectorPostprocessors/ElementOptimizationVJPInnerProduct

!syntax children /VectorPostprocessors/ElementOptimizationVJPInnerProduct
