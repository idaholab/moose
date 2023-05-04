# TransientAndAdjoint

!syntax description /Executioner/TransientAndAdjoint

## Overview

Similar to [SteadyAndAdjoint.md], this executioner can be used to solve a transient forward problem and it's adjoint. Like it's steady-state counterpart, this executioner has a two-step procedure, whereby performing the full forward transient solve (as in [Transient.md]) then stepping backward through the forward timesteps to solve the adjoint problem. To give context on what a transient adjoint looks like, let's start with the general time-dependent partial differential equation (PDE):

!equation
c(\vec{r}, t, u)\frac{\partial u(\vec{r}, t)}{\partial t} + R^{\mathrm{ss}}(u(\vec{r}, t)) = q(\vec{r}, t),

!equation
u(\vec{r}, t = 0) = u_0(\vec{r}),

!equation
\vec{r}\in \Omega, \quad t\in [0, t_N],

where $c$ is a nonlinear coefficient on the time derivative, $R^{\mathrm{ss}}$ is the parts of the PDE that do not contain a time-derivative, and $q$ is some time-space-dependent source.
The first step in formulating an adjoint for this system is to linearize it around $u(t)$.
Generally, this can be given in operator notation as:

!equation
\mathbf{M}(\mathbf{u}(t))\dot{\mathbf{u}}(t) + \mathbf{K}(\mathbf{u}(t))\mathbf{u}(t) = \mathbf{q}(t).

Using [ImplicitEuler.md] time integration, a given time step's ($n$) solve resembles:

!equation
\frac{1}{\Delta t_n}\mathbf{M}(\mathbf{u}_n)\mathbf{u}_n + \mathbf{K}(\mathbf{u}_n)\mathbf{u}_n = \mathbf{q}_n + \frac{1}{\Delta t_n}\mathbf{M}(\mathbf{u}_n)\mathbf{u}_{n-1}, \quad n=1,...,N.

This system can be represented by a large system for all time steps:

!equation
\begin{bmatrix}
\mathbf{1}       &                  &                  &                    &                  \\
\mathbf{A_{1,0}} & \mathbf{A_{1,1}} &                  &                    &                  \\
                 & \mathbf{A_{2,1}} & \mathbf{A_{2,2}} &                    &                  \\
                 &                  & \ddots           & \ddots             &                  \\
                 &                  &                  & \mathbf{A_{N-1,N}} & \mathbf{A_{N,N}} \\
\end{bmatrix}
\begin{bmatrix}
\mathbf{u}_0 \\
\mathbf{u}_1 \\
\mathbf{u}_2 \\
\vdots \\
\mathbf{u}_N \\
\end{bmatrix}
=
\begin{bmatrix}
\mathbf{u}_0 \\
\mathbf{q}_1 \\
\mathbf{q}_2 \\
\vdots \\
\mathbf{q}_N \\
\end{bmatrix}
,

where,

!equation
\mathbf{A_{n,n}} \equiv \frac{1}{\Delta t_n}\mathbf{M}(\mathbf{u}_n) + \mathbf{K}(\mathbf{u}_n) \quad \text{and} \quad \mathbf{A_{n,n-1}} \equiv \frac{1}{\Delta t_n}\mathbf{M}(\mathbf{u}_n)

The adjoint equation is then the transpose of this operator, with its own source ($q_\lambda(\vec{r},t)$):

!equation
\begin{bmatrix}
\mathbf{A_{1,1}}^{\top} & \mathbf{A_{2,1}}^{\top} &                         &                             &                           \\
                        & \mathbf{A_{2,2}}^{\top} & \mathbf{A_{3,2}}^{\top} &                             &                           \\
                        &                         & \ddots                  & \ddots                      &                           \\
                        &                         &                         & \mathbf{A_{N-1,N-1}}^{\top} & \mathbf{A_{N,N-1}}^{\top} \\
                        &                         &                         &                             & \mathbf{A_{N,N}}^{\top}   \\
\end{bmatrix}
\begin{bmatrix}
\mathbf{\lambda}_1 \\
\mathbf{\lambda}_2 \\
\vdots \\
\mathbf{\lambda}_{N-1} \\
\mathbf{\lambda}_N \\
\end{bmatrix}
=
\begin{bmatrix}
\mathbf{q_\lambda}_1 \\
\mathbf{q_\lambda}_2 \\
\vdots \\
\mathbf{q_\lambda}_{N-1} \\
\mathbf{q_\lambda}_N \\
\end{bmatrix}
.

To solve this system, the adjoint problem is solved backward in time:

!equation
\left( \frac{1}{\Delta t_n}\mathbf{M}(\mathbf{u}_n) + \mathbf{K}(\mathbf{u}_n) \right)^{\top}\mathbf{\lambda}_n = \mathbf{q_\lambda}_n + \frac{1}{\Delta t_{n+1}}\mathbf{M}^{\top}(\mathbf{u}_{n+1})\lambda_{n+1}, \quad n=N,...,1,

with $\lambda_{N+1} \equiv \mathbf{0}$.

## Executioner Algorithm

!algorithm caption=Transient and adjoint executioner algorithm id=alg:transient_and_adjoint
[!state text=Set forward initial condition: $\mathbf{u}_0$]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `INITIAL`]
[!state text=Cache forward solution and time: $\mathbf{u}_0$, $t_0$]
[!for!begin condition=$n\leftarrow 1,..,N$]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `TIMESTEP_BEGIN`]
[!state text=Solve forward time step: $\mathbf{u}_n \leftarrow \mathbf{A_{n,n}}^{-1}\left(\mathbf{q}_n + \mathbf{A_{n,n-1}}\mathbf{u}_{n-1}\right)$]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `TIMESTEP_END`]
[!state text=Cache forward solution and time: $\mathbf{u}_n$, $t_n$]
[!for!end]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `FINAL`]
[!state text=Set previous time residual: $\mathbf{R^{\mathrm{old}}}\leftarrow 0$]
[!for!begin condition=$n\leftarrow N,..,1$]
[!state text=Set forward solution: $\mathbf{u}_n$]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `ADJOINT_TIMESTEP_BEGIN`]
[!state text=Compute forward Jacobian: $\mathbf{A_{n,n}}$]
[!state text=Compute adjoint source: $\mathbf{q_\lambda}_n$]
[!state text=Solve adjoint system: $\mathbf{\lambda}_n \leftarrow \left(\mathbf{A_{n,n}}^{\top}\right)^{-1}\left(\mathbf{q_\lambda}_n + R^{\mathrm{old}}\right)$]
[!state text=Execute user-objects, auxiliary kernels, and multi-apps on `ADJOINT_TIMESTEP_END`]
[!state text=Evaluate time residual: $\mathbf{R^{\mathrm{old}}}\leftarrow \frac{1}{\Delta t_n}\mathbf{M}^{\top}(\mathbf{u}_n)\mathbf{\lambda}_n$]
[!for!end]

## Limitations of Adjoint Solve

1. The adjoint solver only supports a consistent adjoint with [ImplicitEuler.md] time integration.
1. Saving the forward solution at every time step can be extremely memory intensive, so there may be limitations on the number of timesteps of the forward problem based on system memory.
1. [Exodus.md] cannot output the resulting adjoint solution. [CSV.md] and [JSONOutput.md] work as expected.

## Example Input File Syntax

The input syntax for this executioner is identical to [SteadyAndAdjoint.md]. One key difference is the way the adjoint solution is outputted. Currently, [Exodus.md] does not support the backward time-stepping of the adjoint solve, but [CSV.md] can be outputted with two separate output objects. Furthermore, in order to see a table of postprocessors during the adjoint solve, the [!param](/Outputs/Console/execute_on) parameter must be modified in the [Console.md] output:

!listing transient_and_adjoint/self_adjoint.i block=Outputs

!syntax parameters /Executioner/TransientAndAdjoint

!syntax inputs /Executioner/TransientAndAdjoint

!syntax children /Executioner/TransientAndAdjoint
