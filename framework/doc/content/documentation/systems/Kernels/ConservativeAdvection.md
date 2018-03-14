# ConservativeAdvection

## Description

The `ConservativeAdvection` kernel implements an advection term given for the domain ($\Omega$) define as

\begin{equation}
\underbrace{\nabla \cdot \vec{v} u}_{\textrm{ConservativeAdvection}} + \sum_{i=1}^n \beta_i = 0 \in \Omega,
\end{equation}
where $v$ is the advecting velocity and the second term on the left hand side
represents the strong forms of other kernels. `ConservativeAdvection` does not assume
that the velocity is divergence free and instead applies $\nabla$ to the test
function $\psi_i$ in the weak variational form after integrating by parts,
as in

\begin{equation}
R_i(u_h) = \underbrace{-(\nabla \psi_i, \vec{v} u)}_{\textrm{ConservativeAdvection}} + \langle\psi_i, \vec{v} u
\cdot \vec{n}\rangle \quad \forall \psi_i,
\end{equation}
where $\psi_i$ are the test functions and $u_h \in \mathcal{S}^h$ is the finite
element solution of the weak formulation. The first term is the volumetric term and the second term
is a surface term describing the advective flux out of the
volume. `ConservativeAdvection` corresponds to the former volumetric term.

The corresponding Jacobian is given by

\begin{equation}
\frac{\partial R_i(u_h)}{\partial u_j} = -(\nabla \psi_i, \vec{v} \phi_j).
\end{equation}

## Example Syntax

`ConservativeAdvection` can be used in a variety of problems, including
advection-diffusion-reaction. The syntax for `ConservativeAdvection` is
demonstrated in this `Kernels` block from an advection-diffusion-reaction test
case:

!listing
test/tests/dgkernels/advection_diffusion_mixed_bcs_test_resid_jac/dg_advection_diffusion_test.i
block=Kernels label=false

The velocity is supplied as a three component vector with order $v_x$, $v_y$, and  $v_z$.

!syntax parameters /Kernels/ConservativeAdvection

!syntax inputs /Kernels/ConservativeAdvection

!syntax children /Kernels/ConservativeAdvection
