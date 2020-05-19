# CentralDifference

!syntax description /Executioner/TimeIntegrator/CentralDifference

## Description

Central difference integration is a common explicit integration scheme, typically used in structural dynamics. For a variable, $u$, at time $t$ with a time step, $\Delta t$, the central difference approximations for the first and second time derivatives, $\dot{u}$ and $\ddot{u}$, are given as,

\begin{equation}
\begin{aligned}
\mathbf{\dot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t-\Delta t)}{2 \Delta t} \\
\mathbf{\ddot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-2\mathbf{u}(t)+\mathbf{u}(t-\Delta t)}{\Delta t^2}
\end{aligned}
\end{equation}

The central difference time integrator derives from the [`ActuallyExplicitEuler`](ActuallyExplicitEuler.md) class and therefore circumvents the nonlinear solver. It can be used with `consistent`, `lumped` or `lump_preconditioned`, `solve_type` options. Information on these solve options can be found on the [`ActuallyExplicitEuler`](ActuallyExplicitEuler.md) page.

!syntax parameters /Executioner/TimeIntegrator/CentralDifference

!syntax inputs /Executioner/TimeIntegrator/CentralDifference

!syntax children /Executioner/TimeIntegrator/CentralDifference
