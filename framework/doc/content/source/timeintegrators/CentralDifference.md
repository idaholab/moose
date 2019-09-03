# CentralDifference

!syntax description /Executioner/TimeIntegrator/CentralDifference

## Description

Central difference integration is a common explicit integration scheme, typically used for structural dynamics problems. For a variable, $u$, at time $t$ with a time step, $\Delta t$, the central difference approximations for the first and second time derivatives, $\dot{u}$ and $\ddot{u}$, are given as,

\begin{equation}
\begin{aligned}
\mathbf{\dot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t-\Delta t)}{2 \Delta t} \\
\mathbf{\ddot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-2\mathbf{u}(t)+\mathbf{u}(t-\Delta t)}{\Delta t^2}
\end{aligned}
\end{equation}

The central difference time integrator derives from the [`ActuallyExplicitEuler`](ActuallyExplicitEuler.md) class and therefore circumvents the nonlinear solver. It can be used with `consistent`, `lumped` or `lump_preconditioned`, `solve_type` options.

## Assumptions

The implementation of the central difference timeintegrator assumes that corresponding kernels involving the first and second derivatives directly use the derivatives calculates by the timeintegrator. Currently, only the `InertialForce` kernel and the `NodalKernels` corresponding to nodal inertias are modified to work with the central difference timeintegrator. Therefore, the central difference time integrator can currently only be used for mechanics problems with solid elements. 

!syntax parameters /Executioner/TimeIntegrator/CentralDifference

!syntax inputs /Executioner/TimeIntegrator/CentralDifference

!syntax children /Executioner/TimeIntegrator/CentralDifference
