# ConservativeAdvection

## Description

This kernel implements an advection term given by $$\nabla \cdot \vec{v} u$$
where $v$ is the advecting velocity. `ConservativeAdvection` does not assume
that the velocity is divergence free and instead applies $\nabla$ to the test
function $\psi_i$ in the weak variational form after integrating by parts,
e.g. the weak form looks like $$-(\nabla \psi_i, \vec{v} u)\ + <\psi_i, \vec{v} u
\cdot \vec{n}>$$ where the first term is the volumetric term and the second term
is a surface term describing the advective flux out of the
volume. `ConservativeAdvection` corresponds to the former volumetric term.

The corresponding Jacobian is given by $$-(\nabla \psi_i, \vec{v} \phi_j)$$.

## Example Syntax

`ConservativeAdvection` can be used in a variety of problems, including
advection-diffusion-reaction. The syntax for `ConservativeAdvection` is
demonstrated in this `Kernel` block from an advection-diffusion-reaction test
case:

!listing
test/tests/dgkernels/advection_diffusion_mixed_bcs_test_resid_jac/dg_advection_diffusion_test.i
block=Kernels label=false

The velocity is supplied as a three component vector with order $v_x\, v_y\, v_z$.

!syntax parameters /Kernels/ConservativeAdvection

!syntax inputs /Kernels/ConservativeAdvection

!syntax children /Kernels/ConservativeAdvection
