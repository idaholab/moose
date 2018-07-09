# MassLumpedTimeDerivative

## Description

Lumping of the time derivative can be useful for a couple of reasons. Lumping
helps ensure conservation of mass at a node. In a standard node-based Galerkin
approximation, fluxes from spatial terms can be thought of as "entering"
nodes. If there is no flux to a node, then the mass at that node should stay
fixed. However, if the standard Galerkin method is applied to a time derivative
term, $$(\psi_i, \frac{\partial u_h}{\partial t}$$ the corresponding coefficient
matrix is tri-diagonal and the mass at a node is affected by fluxes to neighboring
nodes. This can lead to violation of local mass conservation and generation of
spurious oscillations with unphysical under- and over-shoot phenomena. Lumping
fixes this problem by isolating a nodal solution from neighboring nodal
solutions in the time derivative term. Mathematically, lumping looks like
this. We start with our governing equation $$u_t = Au$$ where A is a
differential operator. We write our finite element solution as

$$ u(t, x) = \sum u_j(t) \phi_j(x) $$

Substituting into our governing equation, we have:

$$ \sum u_j'\phi_j = Sum u_jA\phi_j $$

Now we apply our test functions $\psi_i$ and integrate over the volume:

$$ \sum u_j' (\psi_i\phi_j) = \sum u_j (\psi_i, Au_j) $$

After applying all of our test functions, we have the matrix system

$$ \widetilde{M}\vec{u'} = \widetilde{K}\vec{u} $$

where here $\vec{u}$ denotes the vector of coefficients $u_j$. $\widetilde{M}$
is the mass matrix and $\widetilde{K}$ is the stiffness matrix. Note that
neither of these matrices are ever explicitly formed in MOOSE but they are still
directly relevant. As mentioned previously, a standard Galerkin procedure
results in a tri-diagonal $\widetilde{M}$. As it turns out, the sum of the
matrix row elements in $\widetilde{M}$ is one. Lumping then consists of summing
the matrix row elements (result 1) and placing the sums on the diagonals. The
result is the identity matrix and $\widetilde{M}\vec{u'}$ becomes simply
$\vec{u'}$:

$$ \vec{u'} = \widetilde{K}\vec{u} $$

As seen by the above equation, besides helping with local conservation of mass,
mass lumping makes explicit time stepping feasible because it removes the need
for solving a linear system.

## Example Syntax

The `MassLumpedTimeDerivative` syntax is simple, only taking `type` and
`variable` as shown in the kernel block below. This particular test file, from
which the kernel block is taken, demonstrates a case where the concentration of
$u$ would become negative in a non-lumped scheme for sufficiently small time
steps.

!listing test/tests/kernels/mass_lumping/mass_lumping.i block=Kernels 

!syntax parameters /Kernels/MassLumpedTimeDerivative

!syntax inputs /Kernels/MassLumpedTimeDerivative

!syntax children /Kernels/MassLumpedTimeDerivative
