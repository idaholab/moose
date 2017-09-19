# BodyForce

## Description

`BodyForce` implements a force term in momentum transport or structural
mechanics or a source term in species/mass transport. The strong form, given a
domain $\Omega$ is defined as

\begin{equation}
-f = 0 \in \Omega
\end{equation}

The weak form, in inner-product notation, is defined as

\begin{equation}
R_i(u_h) = (\psi_i, -f) \quad \forall \psi_i,
\end{equation}

where $f$ is the magnitude of the force or source and $\psi_i$ are the test functions, and
and $u_h$ are the trial functions for the unknown ($u$).

The Jacobian term for this kernel is zero: $\frac{\partial R_i(u_h)}{\partial u_j} = 0$, since
it is assumed that $f$ **is not** a function of the unknown $u$.

The force is constructed through a user supplied constant,
[function](systems/Functions/index.md), and/or
[postprocessor](systems/Postprocessors/index.md). The constant, supplied through
the parameter `value`, may also be controlled over the course of a transient
simulation with a [`Controls`](systems/Controls/index.md) block.

## Example Syntax

The case below demonstrates the use of `BodyForce` where the force term is
supplied through a postprocessor:

!listing test/tests/kernels/2d_diffusion/2d_diffusion_bodyforce_test.i
 block=Kernels label=false

 Since `value` and `function` were not supplied, they default to 1 and 1. The
 corresponding `Postprocessor` block is

!listing test/tests/kernels/2d_diffusion/2d_diffusion_bodyforce_test.i block=Postprocessors label=false

!!!note
    Test the postprocessor value is actually calculated from a function.

!syntax parameters /Kernels/BodyForce

!syntax inputs /Kernels/BodyForce

!syntax children /Kernels/BodyForce
