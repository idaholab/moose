# CentralDifference

!syntax description /Executioner/TimeIntegrator/CentralDifference

## Description

Central difference integration is a common explicit integration scheme, typically used in structural dynamics.
The central difference time integrator derives from the [`ActuallyExplicitEuler`](ActuallyExplicitEuler.md) class and therefore circumvents the nonlinear solver. It can be used with `consistent`, `lumped` or `lump_preconditioned`, `solve_type` options. Information on these solve options can be found on the [`ActuallyExplicitEuler`](ActuallyExplicitEuler.md) page.

There are two key behaviors for central difference, indicated with the option `use_direct`.

### `use_direct = false`

For a variable, $u$, at time $t$ with a time step, $\Delta t$, the central difference approximations for the first and second time derivatives, $\dot{u}$ and $\ddot{u}$, are given as,

\begin{equation}
    \begin{aligned}
        \mathbf{\dot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-\mathbf{u}(t-\Delta t)}{2 \Delta t} \\
        \mathbf{\ddot{u}}(t) &=& \frac{\mathbf{u}(t+\Delta t)-2\mathbf{u}(t)+\mathbf{u}(t-\Delta t)}{\Delta t^2}
    \end{aligned}
\end{equation}

This leads to the following solution update,

\begin{equation}
    \mathbf{u}(t+\Delta t)=\mathbf{u(t)}+\left[\frac{1}{\Delta t^2}\mathbf{M}+\frac{1}{2\Delta t}\mathbf{C}\right]^{-1}\left[\mathbf{M}\left[\frac{1}{\Delta t^2}(\mathbf{u}(t-\Delta t)\right]-\mathbf{C}\left[\frac{1}{2\Delta t}\left(\mathbf{u}(t-\Delta t)-\mathbf{u}(t)\right)\right]+\mathbf{F}^{\text{ext}}-\mathbf{F}^{\text{int}}(\mathbf{u}(t))\right]
\end{equation},

where $\mathbf{M}$ represents the mass matrix, $\mathbf{C}$ represents the damping matrix, $\mathbf{F}^{\text{ext}}$ represents the external forces, and $\mathbf{F}^{\text{int}}(\mathbf{u}(t))$ represents the internal forces.

### `use_direct = true`

Here, the solution update will be based on a direct calculation of the acceleration. Note that this method only works if `solve_type = lumped` due to the need for a mass matrix inversion. This formulation follows the standard central difference definition for the solution update,

\begin{equation},
    \begin{aligned}
        \mathbf{\ddot{u}}(t) &= \mathbf{M}^{-1}\left[F^{\text{ext}}-F^{\text{int}}\left(\mathbf{u}(t)\right)\right] \\
        \mathbf{\dot{u}}(t+\frac{\Delta t}{2}) &= \mathbf{u}(t-\frac{\Delta t}{2}) + \Delta t_{\text{avg}} \mathbf{\ddot{u}}(t) \\
        \mathbf{u}(t + \Delta t) &= \mathbf{u}(t)+\Delta t \mathbf{\dot{u}}(t+\frac{\Delta t}{2})
    \end{aligned}
\end{equation}

where $\Delta t_\text{avg}$ is an average of the previous and current time steps.

If using nodal BC's, then use `preset=true` on those BC's for proper boundary condition enforcement.

!syntax parameters /Executioner/TimeIntegrator/CentralDifference

!syntax inputs /Executioner/TimeIntegrator/CentralDifference

!syntax children /Executioner/TimeIntegrator/CentralDifference
