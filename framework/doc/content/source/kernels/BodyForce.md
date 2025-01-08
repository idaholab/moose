# BodyForce

## Description

`BodyForce` implements a force term, such as a heat generation/sink term for heat
conduction, a momentum source/sink for momentum transport or structural mechanics, or
a source/sink term in species/mass transport. The context of this kernel depends
on the differential equation of interest, but shares the strong form on a domain
$\Omega$ as

\begin{equation}
\underbrace{-f}_{\textrm{BodyForce}} + \text{other kernels} = 0 \in \Omega
\end{equation}
where $f$ is the source term (negative if a sink) and "other kernels"
represent the strong forms of other terms present in the equation. The `BodyForce`
weak form, in inner-product notation, is defined as

\begin{equation}
R_i(u_h) = (\psi_i, -f) \quad \forall \psi_i,
\end{equation}
where the $\psi_i$ are the test functions, and $u_h$ are the trial solutions in
the finite dimensional space $\mathcal{S}^h$ for the unknown ($u$).

The Jacobian term for this kernel is zero: $\frac{\partial R_i(u_h)}{\partial u_j} = 0$, since
it is assumed that $f$ +is not+ a function of the unknown $u$.

The force is constructed through a user supplied constant $c$,
[function](/Functions/index.md) value evaluated at the current time and
quadrature point $f$, and/or [postprocessor](/Postprocessors/index.md)
value $p$. The constant $c$ may also be
controlled over the course of a transient simulation with a
[Controls](/Controls/index.md) block.
Not supplying $c$, $f$, or $p$ through its corresponding
parameter is equivalent to setting its value to unity.

## Example Input File Syntax

The case below demonstrates the use of `BodyForce` where the force term is
supplied based upon a function form:

!listing test/tests/kernels/block_kernel/block_kernel_test.i block=Kernels

!syntax parameters /Kernels/BodyForce

!syntax inputs /Kernels/BodyForce

!syntax children /Kernels/BodyForce
