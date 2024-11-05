# DefaultNonlinearConvergence

This [Convergence](Convergence/index.md) is the default convergence for
[/FEProblem.md], using a combination of criteria to determine convergence.

!alert warning title=Shared executioner parameters
This object shares several parameters with the executioner. If one of these
parameters is set by the user in the executioner and not in the convergence
object, then the value is taken from the executioner, rather than the default
value. If a parameter is set by the user in both the executioner and the convergence
object, an error is thrown.

Consider the system of algebraic equations:

!equation
\mathbf{r}(\mathbf{u}) = \mathbf{0} \,.

This class reports convergence of the solution to this system if
+any+ of the following conditions are true:

!equation
\|\mathbf{r}\|_2 < \tau_\text{abs} \,,

!equation
\frac{\|\mathbf{r}\|_2}{\|\mathbf{r}_0\|_2} < \tau_\text{rel} \,,

!equation
\frac{\|\mathbf{\delta u}\|_2}{\|\mathbf{u}\|_2} < \tau_{\delta u,\text{rel}} \,.

This class reports divergence if +any+ of the following conditions are true:

!equation
\|\mathbf{r}\|_2 = \text{NaN} \,,

!equation
n_\text{evals} \geq n_\text{evals,max} \,,

!equation
\|\mathbf{r}\|_2 > \tau_\text{div,abs} \,,

!equation
\frac{\|\mathbf{r}\|_2}{\|\mathbf{r}_0\|_2} > \tau_\text{div,rel} \,,

!equation
n_\text{ping} > n_\text{ping,max} \,,

where

- $\|\cdot\|_2$ is the discrete $L_2$ norm,
- $\mathbf{r}_0$ is the initial (guess) residual vector,
- $\mathbf{\delta u} = \mathbf{u}^{(\ell)} - \mathbf{u}^{(\ell-1)}$ is the solution step vector,
- "NaN" is a not-a-number value,
- $\tau_\text{abs}$ is the absolute residual tolerance, specified with the
  [!param](/Convergence/DefaultNonlinearConvergence/nl_abs_tol) parameter.
- $\tau_\text{rel}$ is the relative residual tolerance, provided by
  [!param](/Convergence/DefaultNonlinearConvergence/nl_rel_tol).
- $\tau_{\delta u,\text{abs}}$ is the relative step tolerance., provided by
  [!param](/Convergence/DefaultNonlinearConvergence/nl_rel_step_tol).
- $\tau_\text{div,abs}$ is the absolute residual divergence tolerance, provided by
  [!param](/Convergence/DefaultNonlinearConvergence/nl_abs_div_tol).
- $\tau_\text{div,rel}$ is the relative residual divergence tolerance, provided by
  [!param](/Convergence/DefaultNonlinearConvergence/nl_div_tol).
- $n_\text{evals}$ is the number of residual evaluations.
- $n_\text{evals,max}$ is the maximum number of residual evaluations, provided by
  [!param](/Convergence/DefaultNonlinearConvergence/nl_max_funcs).
- $n_\text{ping}$ is the number of ping-pong iterations (consecutive iterations in which the residual grows then reduces on every other iteration).
- $n_\text{ping,max}$ is the maximum number of ping-pong iterations, provided by
  [!param](/Convergence/DefaultNonlinearConvergence/n_max_nonlinear_pingpong).

!syntax parameters /Convergence/DefaultNonlinearConvergence

!syntax inputs /Convergence/DefaultNonlinearConvergence

!syntax children /Convergence/DefaultNonlinearConvergence
