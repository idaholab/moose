## Description

The steady-state diffusion equation on a domain $\Omega$ is defined as
$$-\nabla \cdot \nabla u = 0 \in \Omega.$$

The weak form of this equation, in inner-product notation, is given by:
$$R_i(u_h) = (\nabla \phi_i, \nabla u_h) = 0 \quad \forall  \phi_i, $$
where $\phi_i$ are the test functions and $u_h$ is the finite element solution of $u$.

The Jacobian is defined as:
$$\frac{\partial R_i(u_h)}{\partial u_j} = (\nabla \phi_i, \nabla \psi_j).$$
