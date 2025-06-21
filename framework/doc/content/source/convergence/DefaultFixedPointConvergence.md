# DefaultFixedPointConvergence

This [Convergence](Convergence/index.md) is the default convergence for fixed point solves,
using a combination of criteria to determine convergence.

!alert warning title=Shared executioner parameters
This object shares several parameters with the executioner. If one of these
parameters is set by the user in the executioner and not in the convergence
object, then the value is taken from the executioner, rather than the default
value. If a parameter is set by the user in both the executioner and the convergence
object, an error is thrown.

The parameter [!param](/Executioner/Steady/fixed_point_min_its) specifies the minimum number of iterations before convergence can occur. The parameter [!param](/Executioner/Steady/fixed_point_max_its) specifies the maximum number of iterations, upon which the solution will either diverge or if [!param](/Executioner/Steady/accept_on_max_fixed_point_iteration) is specified to be true, converge.

This Convergence allows checking for convergence on iteration $\ell$ in two ways:

- checking the nonlinear residual norm $\|\mathbf{r}\|_\ell$ (discrete $L_2$ norm) against tolerances, and
- checking a post-processor value $y_\ell$, specified by [!param](/Executioner/Steady/custom_pp), against tolerances.

## Residual Norm Checks

The residual norm checks are active unless [!param](/Executioner/Steady/disable_fixed_point_residual_norm_check) is set to true; convergence is declared if either of the following are true:

!equation
\|\mathbf{r}\|^\text{max}_\ell < \tau_\text{abs} \,,

!equation
\frac{\|\mathbf{r}\|^\text{max}_\ell}{\|\mathbf{r}\|^\text{init}} < \tau_\text{rel} \,,

where

- $\|\mathbf{r}\|^\text{max}_\ell = \text{max}\left(\|\mathbf{r}\|^\text{begin}_\ell, \|\mathbf{r}\|^\text{end}_\ell\right)$.
- $\|\mathbf{r}\|^\text{begin}_\ell$ is the `TIMESTEP_BEGIN` norm for iteration $\ell$.
- $\|\mathbf{r}\|^\text{end}_\ell$ is the `TIMESTEP_END` norm for iteration $\ell$.
- $\|\mathbf{r}\|^\text{init}$ is the `MULTIAPP_FIXED_POINT_BEGIN` norm.
- $\tau_\text{abs}$ is the absolute residual tolerance, provided by
  [!param](/Convergence/DefaultFixedPointConvergence/fixed_point_abs_tol).
- $\tau_\text{rel}$ is the relative residual tolerance, provided by
  [!param](/Convergence/DefaultFixedPointConvergence/fixed_point_rel_tol).

## Post-processor Checks

For the post-processor checks, activated by specifying [!param](/Executioner/Steady/custom_pp), there is an option [!param](/Executioner/Steady/direct_pp_value) which determines whether the post-processor value itself is checked or if the difference with the previous iteration value is used. If [!param](/Executioner/Steady/direct_pp_value) is set to `true`, convergence is declared if either of the following are true:

!equation
|y_\ell| < \tau^\text{pp}_\text{abs} \,,

!equation
\left|\frac{y_\ell}{y_0}\right| < \tau^\text{pp}_\text{rel} \,,

where

- $\tau^\text{pp}_\text{abs}$ is the absolute post-processor tolerance., provided by
  [!param](/Convergence/DefaultFixedPointConvergence/custom_abs_tol).
- $\tau^\text{pp}_\text{rel}$ is the relative post-processor tolerance., provided by
  [!param](/Convergence/DefaultFixedPointConvergence/custom_rel_tol).

If [!param](/Executioner/Steady/direct_pp_value) is set to `false`, convergence is declared if either of the following are true:

!equation
|y_\ell - y_{\ell-1}| < \tau^\text{pp}_\text{abs} \,,

!equation
\left|\frac{y_\ell - y_{\ell-1}}{y_\ell}\right| < \tau^\text{pp}_\text{rel} \,.

!syntax parameters /Convergence/DefaultFixedPointConvergence

!syntax inputs /Convergence/DefaultFixedPointConvergence

!syntax children /Convergence/DefaultFixedPointConvergence
