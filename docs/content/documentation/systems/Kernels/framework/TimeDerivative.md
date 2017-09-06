<!-- MOOSE Documentation Stub: Remove this when content is added. -->

# TimeDerivative

## Description

This kernel implements a simple time derivative term given by $$\frac{\partial
u}{\partial t}$$. The corresponding weak form is $$(\psi_i, \frac{\partial
u_h}{\partial t})$$ where $u_h$ is the approximate solution and $\psi_i$ is a
finite element test function.

The Jacobian is given by $$(\psi_i, a\phi_j)$$ where $a$ is referred to as
`du_dot_du` in MOOSE syntax. More information about time kernels can be found on
the Kernels description [page](systems/Kernels/index.md).

## Example Syntax

Time derivative terms are ubiquitous in any transient simulation. The kernel
block for a transient advection-diffusion-reaction problem that demonstrates the
`TimeDerivative` syntax is shown below:

!listing
 test/tests/kernels/adv_diff_reaction_transient/adv_diff_reaction_transient_test.i
 block=Kernels label=False

!syntax parameters /Kernels/TimeDerivative

!syntax inputs /Kernels/TimeDerivative

!syntax children /Kernels/TimeDerivative
