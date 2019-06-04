# BodyForce

## Description

`BodyForce` implements a force term in momentum transport or structural
mechanics or a source term in species/mass transport. The strong form, given a
domain $\Omega$ is defined as

\begin{equation}
\underbrace{-f}_{\textrm{BodyForce}} + \sum_{i=1}^n \beta_i = 0 \in \Omega
\end{equation}
where $f$ is the source term (negative if a sink) and the second term on the
left hand side represents the strong forms of other kernels. The `BodyForce`
weak form, in inner-product notation, is defined as

\begin{equation}
R_i(u_h) = (\psi_i, -f) \quad \forall \psi_i,
\end{equation}
where the $\psi_i$ are the test functions, and $u_h$ are the trial solutions in
the finite dimensional space $\mathcal{S}^h$ for the unknown ($u$).

The Jacobian term for this kernel is zero: $\frac{\partial R_i(u_h)}{\partial u_j} = 0$, since
it is assumed that $f$ **is not** a function of the unknown $u$.

The force is constructed through a user supplied constant $c$,
[function](/Functions/index.md) value evaluated at the current time and
quadrature point $f$, and/or [postprocessor](/Postprocessors/index.md)
value $p$. The constant $c$, supplied through the parameter `value`, may also be
controlled over the course of a transient simulation with a
[`Controls`](/Controls/index.md) block.  $c$, $f$, $p$ are supplied
through the input parameters `value`, `function`, and `postprocessor`
respectively. Not supplying $c$, $f$, or $p$ through its corresponding
parameter is equivalent to setting its value to unity.

## Example Syntax

The case below demonstrates the use of `BodyForce` where the force term is
supplied based upon a function form:

!listing test/tests/kernels/block_kernel/block_kernel_test.i block=Kernels

!syntax parameters /Kernels/BodyForce

!syntax inputs /Kernels/BodyForce

!syntax children /Kernels/BodyForce
