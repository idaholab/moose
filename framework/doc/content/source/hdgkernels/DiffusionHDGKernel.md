# DiffusionHDGKernel

This class implements the same physics as [MatDiffusion.md] (see note) but with a
hybridized discontinuous Galerkin (HDG) discretization. For more information on
HDG for a Poisson problem, please see [!citep](cockburn2008superconvergent). The
weak forms implemented are a slight variation on the cited work in order to make
them directly usable in downstream physics such as Navier-Stokes.

\begin{equation}
(\vec{q}, \vec{v})_{\Omega_h} + (u, \nabla \cdot \vec{v})_{\Omega_h} - \langle \hat{u}, \vec{n}\cdot\vec{v}\rangle_{\partial\Omega_h} = 0\\
(D\vec{q}, \nabla\omega)_{\Omega_h} - \langle D\vec{q}\cdot\vec{n}, \omega\rangle_{\partial\Omega_h} + \langle \tau(u - \hat{u}\vec{n}\cdot\vec{n}), \omega\rangle_{\partial\Omega_h} = (f, \omega)_{\Omega_h}\\
-\langle D\vec{q}\cdot\vec{n}, \mu\rangle_{\partial\Omega_h} + \langle \tau(u - \hat{u})\vec{n}\cdot\vec{n}, \mu\rangle_{\partial\Omega_h} = -\langle Dq_N, \mu\rangle_{\partial\Omega_N}
\end{equation}

where $\vec{q}$ is the gradient field, $\vec{v}$ are its associated test
functions, $u$ is the scalar field, $\omega$ are its associated test functions,
$\hat{u}$ is the trace of the scalar field (lives on the mesh skeleton), $\mu$
are its associated test functions, $\tau$ is a stabilization parameter, $f$ is a
forcing function, $D$ is the diffusivity coefficient, and $q_N$ represents a
prescribed gradient on a Neumann boundary, $\partial\Omega_N$. As indicated by
the test functions, the first equation is the equation for the variable
$\vec{q}$, the second is for the variable $u$, and the third is for the trace or
Lagrange multiplier variable $\hat{u}$.

$\vec{q}$ (the gradient) and $u$ (the values) are referred to as the primal
variables. $\hat{u} is often referred to as the dual variable. Its equation is
the only one that requires the use of a linear solver. For more information on
HDG solves, see the [HDGKernels page](HDGKernels/index.md).

$f$ the forcing function can be used for method of manufactured solutions
studies, or as a source term for the diffused quantity.

!alert note
Unlike MatDiffusion, the dependence of the diffusion coefficient on nonlinear variables is not captured in the construction of the Jacobian.

!syntax parameters /HDGKernels/DiffusionHDGKernel

!syntax inputs /HDGKernels/DiffusionHDGKernel

!syntax children /HDGKernels/DiffusionHDGKernel
