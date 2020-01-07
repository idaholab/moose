# ExplicitSSPRungeKutta

## Introduction

This time integrator includes explicit Strong Stability Preserving (SSP)
Runge-Kutta time integration methods, of orders 1, 2, and 3, deriving from
[/ExplicitTimeIntegrator.md], meaning that no nonlinear solver is invoked. The
key feature of SSP Runge-Kutta methods is that they preserve the strong
stability properties (in any norm or seminorm) of the explicit/forward Euler
method [!cite](gottlieb2005).

## Formulation

For the ODE
\begin{equation}
  \frac{d u}{d t} = f(u(t), t) \,,
\end{equation}
the SSP Runge-Kutta methods up to order 3 can be expressed in the following form
for a time step:
\begin{equation}
  u^{(0)} = u^n \,,
\end{equation}
\begin{equation}
  u^{(s)} = \sum\limits_{k=0}^{s-1} a_{s,k} u^{(k)} + b_s \Delta t f\left(u^{(s-1)}, t^n + c_s\Delta t\right) \,,
    \qquad i = 1,\ldots,N_s \,,
\end{equation}
\begin{equation}
  u^{n+1} = u^{(N_s)} \,,
\end{equation}
where $N_s$ is the number of stages and for methods up to order 3, is also the
order of accuracy. The coefficients $a_{i,j}$, $b_i$, and $c_i$ can be
conveniently expressed in the following tabular form:
\begin{equation}
  \begin{array}{c|c}
    \mathbf{c} & \mathbf{A}\\\hline
    & \mathbf{b}^T\\
  \end{array} \,.
\end{equation}
Respectively, the tables for the methods of orders 1, 2, and 3 are as follows:
\begin{equation}
  \begin{array}{c|c}
    0 & 1\\\hline
    & 1\\
  \end{array} \,,
\end{equation}
\begin{equation}
  \begin{array}{c|c c}
    0 & 1           & \\
    1 & \frac{1}{2} & \frac{1}{2}\\\hline
      & 1           & \frac{1}{2}\\
  \end{array} \,,
\end{equation}
\begin{equation}
  \begin{array}{c|c c}
    0           & 1           &             &\\
    1           & \frac{3}{4} & \frac{1}{4} &\\
    \frac{1}{2} & \frac{1}{3} & 0           & \frac{2}{3}\\\hline
                & 1           & \frac{1}{4} & \frac{2}{3}\\
  \end{array} \,.
\end{equation}
These methods have the following time step size requirement for stability:
\begin{equation}
  \Delta t < \gamma \Delta t_{FE} \,, \quad \gamma = \min\limits_{i,k} \frac{a_{i,k}}{b_i} \,.
\end{equation}
where $\Delta t_{FE}$ is the maximum time step size for stability of the forward
Euler method. For these methods of order 1, 2, 3, $\gamma = 1$.

In MOOSE, generally the system of ODEs to be solved result from discretization
using the finite element method, and thus a mass matrix exists:
\begin{equation}
  \mathbf{M}\frac{d \mathbf{u}}{d t} = \mathbf{f}\left(\mathbf{u}(t), t\right) \,.
\end{equation}
In this case, the stage $s$ solution is actually the following:
\begin{equation}
  \mathbf{u}^{(s)} = \sum\limits_{k=0}^{s-1} a_{s,k} \mathbf{u}^{(k)}
    + \mathbf{M}^{-1} b_s \Delta t \mathbf{f}\left(\mathbf{u}^{(s-1)}, t^n + c_s\Delta t\right) \,.
\end{equation}
As an implementation note, the usual mass matrix entry is
\begin{equation}
  m_{i,j} = (\phi_i, \phi_j) \,.
\end{equation}
However, in MOOSE, the mass matrix includes the time step size:
\begin{equation}
  m_{i,j} = \frac{(\phi_i, \phi_j)}{b_i \Delta t} \,,
\end{equation}
\begin{equation}\label{eq:ssprk_no_bc}
  \mathbf{u}^{(s)} = \sum\limits_{k=0}^{s-1} a_{s,k} \mathbf{u}^{(k)}
    + \mathbf{M}^{-1} \mathbf{f}\left(\mathbf{u}^{(s-1)}, t^n + c_s\Delta t\right) \,.
\end{equation}

## Dirichlet Boundary Conditions Treatment

Now consider the case where one or more degrees of freedom are subject to strong
(Dirichlet) boundary conditions:
\begin{equation}
  u_i(t) = g(\mathbf{x}_i, t) \,, \qquad i \in \mathcal{I}_\text{Dirichlet} \,.
\end{equation}
For a nonlinear solve with Newton's method, each iteration consists of the
solution of a linear system:
\begin{equation}\label{eq:ssprk_newton_update}
  \mathbf{J}^{(\ell-1)}\mathbf{\delta u}^{(\ell)} = -\mathbf{r}^{(\ell-1)} \,,
\end{equation}
and then updating the solution:
\begin{equation}
  \mathbf{u}^{(\ell)} = \mathbf{u}^{(\ell-1)} + \mathbf{\delta u}^{(\ell)} \,.
\end{equation}
In MOOSE, Dirichlet boundary conditions are implemented by modifying the residual
vector $\mathbf{r}$ to replace entries for the affected degrees of freedom:
\begin{equation}
  \tilde{r}_i = \left\{\begin{array}{l l}
    r_i \,,                      & i \notin \mathcal{I}_\text{Dirichlet}\\
    u_i - g(\mathbf{x}_i, t) \,, & i \in \mathcal{I}_\text{Dirichlet}
    \end{array}\right. \,.
\end{equation}
By modifying the Jacobian matrix as follows, one can guarantee that the
boundary conditions are enforced, i.e., $u_i^{(\ell)} = g(\mathbf{x}_i, t)$ for
$i \in \mathcal{I}_\text{Dirichlet}$:
\begin{equation}
  \tilde{J}_{i,j} = \left\{\begin{array}{l l}
    J_{i,j} \,, & i \notin \mathcal{I}_\text{Dirichlet}\\
    1 \,,       & i \in \mathcal{I}_\text{Dirichlet}, \quad j = i\\
    0 \,,       & i \in \mathcal{I}_\text{Dirichlet}, \quad j \ne i
    \end{array}\right. \,.
\end{equation}
To work with MOOSE's Dirichlet boundary condition implementation, [eq:ssprk_no_bc]
must be put in an update form, similar to [eq:ssprk_newton_update]:
\begin{equation}\label{eq:ssprk_stage_update}
  \mathbf{M}\mathbf{\delta u}^{(s)} = \mathbf{b} \,,
\end{equation}
\begin{equation}\label{eq:ssprk_b}
  \mathbf{b} = \mathbf{M}\left(\sum\limits_{k=0}^{s-1} a_{s,k} \mathbf{u}^{(k)}
    - \mathbf{u}^{(s-1)} \right)
    + \mathbf{f}\left(\mathbf{u}^{(s-1)}, t^n + c_s\Delta t\right) \,,
\end{equation}
\begin{equation}
  \mathbf{\delta u}^{(s)} \equiv \mathbf{u}^{(s)} - \mathbf{u}^{(s-1)} \,.
\end{equation}
To impose the Dirichlet boundary conditions, the mass matrix and right-hand
side vector are modified as for the Newton case:
\begin{equation}
  \tilde{m}_{i,j} = \left\{\begin{array}{l l}
    m_{i,j} \,, & i \notin \mathcal{I}_\text{Dirichlet}\\
    1 \,,       & i \in \mathcal{I}_\text{Dirichlet}, \quad j = i\\
    0 \,,       & i \in \mathcal{I}_\text{Dirichlet}, \quad j \ne i
    \end{array}\right. \,,
\end{equation}
\begin{equation}\label{eq:ssprk_b_mod}
  \tilde{b}_i = \left\{\begin{array}{l l}
    b_i \,,                              & i \notin \mathcal{I}_\text{Dirichlet}\\
    g_i^{(s)} - u_i^{(s-1)} \,, & i \in \mathcal{I}_\text{Dirichlet}
    \end{array}\right. \,,
\end{equation}
where $g_i^{(s)}$ is an appropriate value to impose for degree of freedom $i$
in stage $s$. For most cases, this is simply
\begin{equation}
  g_i^{(s)} = g(\mathbf{x}_i, t^{n+1}) \,.
\end{equation}
However, in general, certain conditions must be enforced on the imposed boundary
values for intermediate stages to preserve the formal order of accuracy of the
method. For methods up to order 2, it is safe to impose each stage as shown
above. For the 3rd-order method, the boundary values imposed in each stage
should be as follows, according to [!cite](zhao2019):
\begin{equation}
  u_i^{(1)} = g(\mathbf{x}_i, t^n) + \Delta t g'(\mathbf{x}_i, t^n) \,,
\end{equation}
\begin{equation}
  u_i^{(2)} = g(\mathbf{x}_i, t^n) + \frac{1}{2}\Delta t g'(\mathbf{x}_i, t^n)
    + \frac{1}{4} \Delta t^2 g''(\mathbf{x}_i, t^n) \,,
\end{equation}
\begin{equation}
  u_i^{(3)} = g(\mathbf{x}_i, t^{n+1}) \,.
\end{equation}

The convergence rates for a MMS problem with time-dependent Dirichlet boundary
conditions is shown in [ssprk_convergence_with_bc]. This illustrates the
degradation of the 3rd-order method to 1st-order accuracy in the presence of
time-dependent Dirichlet boundary conditions. Contrast this to
[ssprk_convergence_no_bc], which shows the results for an MMS problem without
time-dependent Dirichlet boundary conditions, demonstrating the expected orders
of accuracy.

!media media/time_integrators/ssprk_convergence_with_bc.png
       id=ssprk_convergence_with_bc
       caption=Convergence rates for SSPRK methods on an MMS problem with
         time-dependent Dirichlet boundary conditions
       style=width:100%;padding:20px;

!media media/time_integrators/ssprk_convergence_no_bc.png
       id=ssprk_convergence_no_bc
       caption=Convergence rates for SSPRK methods on an MMS problem without
         time-dependent Dirichlet boundary conditions
       style=width:100%;padding:20px;

## Implementation

[eq:ssprk_stage_update] is implemented as described in the following sections:

### `computeTimeDerivatives()`

Only the Jacobian `_du_dot_du` is implemented, which is needed by the mass matrix.
The time derivative itself is not needed because only part of it appears in the
residual vector.

### `solveStage()`

First the mass matrix is computed by calling `computeJacobianTag()` with the
time tag. Because the mass matrix is computed before the call
`computeResidual()`, the call to `computeTimeDerivatives()` must be made before
`computeJacobianTag()`, even though it will be called again in
`computeResidual()`. The Jacobian must be computed before the call to
`computeResidual()` because the mass matrix will be used in `computeResidual()`
via the call to `postResidual()`. In `computeResidual()`, the following steps
occur:

- The steady-state residual $\mathbf{f}\left(\mathbf{u}^{(s-1)}, t^n + c_s\Delta t\right)$
  is computed.
- `postResidual()` is called, which adds the mass matrix product shown in [eq:ssprk_b].
- Dirichlet boundary condition modifications are made to the residual as shown
  in [eq:ssprk_b_mod].

### `postResidual()`

Here $\mathbf{b}$ is assembled as shown in [eq:ssprk_b]. The mass matrix product
here is responsible for the need to call `computeJacobianTag()` before `computeResidual()`
in `solveStage()`.

!syntax parameters /Executioner/TimeIntegrator/ExplicitSSPRungeKutta

!syntax inputs /Executioner/TimeIntegrator/ExplicitSSPRungeKutta

!syntax children /Executioner/TimeIntegrator/ExplicitSSPRungeKutta

!bibtex bibliography
