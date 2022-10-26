# LMKernel

`LMKernel` enables stabilization of Lagrange multiplier (LM) methods in which
the Lagrange multiplier residual equation does not have any on-diagonal
dependence, e.g. dependence on itself. This is a saddle point problem. Use of
`LMKernel` is akin to a pressure-stabilized Petrov-Galerkin formulation for
incompressible Navier-Stokes in which the strong form of the momentum equation
is added to the mass equation. Because the momentum equation contains the
Lagrange multiplier (pressure in this case), addition of it to the mass equation
introduces on-diagonal dependence for the LM. For species transport, a user may
wish to introduce a Lagrange multiplier that ensures the species concentration
is always non-negative. This LM will appear in the (primal) species mass
transport equation in the following way. Let's assume that the mass transport
equation (without the LM) looks like:

\begin{equation}
\frac{\partial u}{\partial t} - \nabla \cdot \nabla u = f
\end{equation}

where $u$ is the primal variable and $f$ is some arbitrary source or sink. When
trying to enforce positivity of $u$ we introduce the LM into the primal equation
like so:

\begin{equation}
\label{eq:primal}
\frac{\partial u}{\partial t} - \nabla \cdot \nabla u - \lambda = f
\end{equation}

where $\lambda$ is the value of the Lagrange multiplier. The corresponding LM
equation will be a nonlinear complimentarity problem (NCP) function of the form

\begin{equation}
\text{min}\left(\lambda, u\right)
\end{equation}

This NCP function ensures the following conditions:

\begin{equation}
\begin{aligned}
\label{eq:conditions}
\lambda &\geq 0\\
u &\geq 0\\
\lambda u &= 0
\end{aligned}
\end{equation}

Examining [!eqref](eq:conditions) it's clear that positive values of $\lambda$ (required
by [!eqref](eq:conditions)) will introduce a source in the primal equation. $\lambda$
will "automatically" adjust itself such that $u$ never drops below zero.  When
$u < \lambda$ the LM is no longer present in its own residual equation and we
have a saddle point problem. However, by adding the strong form of the species
mass transport equation to the LM residual equation, on-diagonal dependence is
restored because of the presence of the LM in the primal equation. This removes
the saddle point and allows use of a much wider range of preconditioners.
