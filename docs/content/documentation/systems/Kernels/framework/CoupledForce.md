# CoupledForce

## Description

`CoupledForce` implements a source term within the domain $\Omega$ proportional to a coupled variable's:
\begin{equation}
-\sigma v = 0 \in \Omega,
\end{equation}
where $\sigma$ is a known scalar coefficient and $v$ is a coupled unknown value. In a species
transport context, the value $\sigma$ can be regarded as a reaction rate coefficient.

The weak form, in inner-product notation, is defined as

\begin{equation}
R_i(u_h) = (\psi_i, -\sigma v) \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $u_h$ is the finite element solution of $u$ (trail functions).

The corresponding Jacobian is
\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = (\psi_i, -\sigma \phi_j).
\end{equation}

## Example Syntax

The kernel block below shows a variable $u$ that is diffusing and being produced
at a rate proportional to the concentration of a variable $v$ which is also
diffusing.

!listing test/tests/bcs/coupled_dirichlet_bc/coupled_dirichlet_bc.i
block=Kernels label=false

!!!note
    In this example the input parameter "coef" that corresponds to
    $\sigma$ is omitted. In this case the "coef" value defaults to one.

!syntax parameters /Kernels/CoupledForce

!syntax inputs /Kernels/CoupledForce

!syntax children /Kernels/CoupledForce
