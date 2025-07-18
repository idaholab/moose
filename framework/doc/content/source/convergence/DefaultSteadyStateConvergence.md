# DefaultSteadyStateConvergence

This [Convergence](Convergence/index.md) is the default convergence for steady-state
detection in [/Transient.md]. The relative discrete L2 norm of the difference of all of the variables, either
from the solution vector or from the auxiliary system, is compared to a tolerance
to determine convergence:

!equation
\|\mathbf{u}^{n+1} - \mathbf{u}^n\|_2 < \tau \|\mathbf{u}^{n+1}\|_2 \,,

where

- $\mathbf{u}$ is the discrete variables vector. If [!param](/Convergence/DefaultSteadyStateConvergence/check_aux) is set to `true`, then the auxiliary system variables are used; else, the solution variables are used.
- $n$ is the time step index.
- $\tau$ is the tolerance, specified by [!param](/Convergence/DefaultSteadyStateConvergence/steady_state_tolerance).

If [!param](/Convergence/DefaultSteadyStateConvergence/normalize_solution_diff_norm_by_dt) is set to `true`, then the norm is normalized by the time step size:

!equation
\frac{\|\mathbf{u}^{n+1} - \mathbf{u}^n\|_2}{\Delta t} < \tau \|\mathbf{u}^{n+1}\|_2 \,.

!syntax parameters /Convergence/DefaultSteadyStateConvergence

!syntax inputs /Convergence/DefaultSteadyStateConvergence

!syntax children /Convergence/DefaultSteadyStateConvergence
