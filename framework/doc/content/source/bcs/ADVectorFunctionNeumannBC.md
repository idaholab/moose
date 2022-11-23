# ADVectorFunctionNeumannBC

!syntax description /BCs/ADVectorFunctionNeumannBC

## Description

`ADVectorFunctionNeumannBC` is similar to [`ADFunctionNeumannBC`](bcs/ADFunctionNeumannBC) for
vector variables and is used for
imposing flux boundary conditions on systems of
partial differential equations (PDEs). This
class is appropriate to use for PDEs of the form
\begin{equation}
\begin{aligned}
  -\nabla^2 \vec{u} &= \vec{f} && \quad \in \Omega \\
  \vec{u} &= \vec{g} && \quad \in \partial \Omega_D \\
  \frac{\partial \vec{u}}{\partial n} &= \vec{h} && \quad \in \partial \Omega_N
\end{aligned}
\end{equation}

where $\Omega \subset \mathbb{R}^n$ is the domain, and $\partial
\Omega = \partial \Omega_D \cup \partial \Omega_N$ is its boundary. In
this case, a `ADVectorFunctionNeumannBC` object is used to impose the condition (3)
on the subset of the boundary denoted by $\partial \Omega_N$. In this case,
$\vec{h}$ is supplied through the [`Function`](Functions/index.md) parameters [!param](/BCs/ADVectorFunctionNeumannBC/function_x), [!param](/BCs/ADVectorFunctionNeumannBC/function_y), and
[!param](/BCs/ADVectorFunctionNeumannBC/function_z). If any of those parameters are not supplied by the user, they
take a default value of $0$. The user must define one
or more sidesets corresponding to the boundary subset $\partial \Omega_N$.

Note that this BC computes its Jacobian using the automatic differentiation system.

!syntax parameters /BCs/ADVectorFunctionNeumannBC

!syntax inputs /BCs/ADVectorFunctionNeumannBC

!syntax children /BCs/ADVectorFunctionNeumannBC
